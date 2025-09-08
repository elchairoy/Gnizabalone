import torch
import pandas as pd
import numpy as np
from pathlib import Path
from torch.utils.data import DataLoader, TensorDataset
import torch.nn as nn
import torch.optim as optim

FEATURE_COUNT = 12
MODEL_CHECKPOINT = "abalone_eval_selfplay.pth"
EVALS_FILE = "evals.txt"

# Training hyperparameters
I = 10          # TD horizon
LR = 0.01       # Learning rate
LEN = 2_000_000 # Max dataset length


class FeatureEvaluator(nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(FEATURE_COUNT, 16, bias=False)
        self.fc2 = nn.Linear(16, 4, bias=False)
        self.fc3 = nn.Linear(4, 1, bias=False)

    def forward(self, x):
        x = torch.tanh(self.fc1(x))
        x = torch.relu(self.fc2(x))
        x = torch.tanh(self.fc3(x))
        return x.squeeze(1)


def load_model_weights_from_file(model, filename):
    with open(filename, 'r') as f:
        weights = np.loadtxt(f)
    idx = 0
    for param in model.parameters():
        num_elements = param.numel()
        param.data = torch.tensor(weights[idx:idx + num_elements], dtype=torch.float32).view_as(param)
        idx += num_elements


def save_model_weights_to_file(model, filename):
    weights = []
    for param in model.parameters():
        weights.extend(param.detach().cpu().numpy().flatten())
    np.savetxt(filename, weights)


def load_td_training_data(td_csv_file=EVALS_FILE, feature_count=FEATURE_COUNT, gamma=1, alpha=1):
    if not Path(td_csv_file).exists():
        raise FileNotFoundError(f"File {td_csv_file} does not exist")

    X_list, y_list, w_list = [], [], []
    current_game_X, current_game_y = [], []

    with open(td_csv_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line:  # new game
                if current_game_X:
                    L = len(current_game_X)
                    for i in range(L):
                        if i >= L - I - 1:
                            target = current_game_y[-1][1] * alpha ** (L - 1)
                        else:
                            target = current_game_y[i + I][0] * alpha ** (i + I)
                        X_list.append(current_game_X[i])
                        y_list.append(target)
                        w_list.append(gamma ** i)
                current_game_X, current_game_y = [], []
            else:
                try:
                    parts = line.split(',')
                    features = [float(p) for p in parts[:feature_count]]
                    evaluation = float(parts[-2])
                    target_value = float(parts[-1])
                    current_game_X.append(features)
                    current_game_y.append([evaluation, target_value])
                except (ValueError, IndexError):
                    continue

    # Last game
    if current_game_X:
        L = len(current_game_X)
        for i in range(L):
            if i >= L - I - 1:
                target = current_game_y[-1][1] * alpha ** (L - 1)
            else:
                target = current_game_y[i + I][0] * alpha ** (i + I)
            X_list.append(current_game_X[i])
            y_list.append(target)
            w_list.append(gamma ** i)

    X_tensor = torch.tensor(np.array(X_list), dtype=torch.float32)
    y_tensor = torch.tensor(np.array(y_list), dtype=torch.float32)
    w_tensor = torch.tensor(np.array(w_list), dtype=torch.float32)

    # Limit dataset size
    if len(X_tensor) > LEN:
        X_tensor = X_tensor[:LEN]
        y_tensor = y_tensor[:LEN]
        w_tensor = w_tensor[:LEN]

    print(f"TD dataset: {len(X_tensor)} positions")
    return X_tensor, y_tensor, w_tensor


def train_td_model(model_class=FeatureEvaluator,
                   evals_file=EVALS_FILE,
                   epochs=20,
                   batch_size=1024,
                   lr=0.01,
                   val_fraction=0.05):
    X, y, weights = load_td_training_data(td_csv_file=evals_file, feature_count=FEATURE_COUNT)

    # Train/validation split
    n_val = max(1, int(len(X) * val_fraction))
    val_indices = torch.randperm(len(X))[:n_val]
    train_mask = torch.ones(len(X), dtype=torch.bool)
    train_mask[val_indices] = False

    X_train, y_train, w_train = X[train_mask], y[train_mask], weights[train_mask]
    X_val, y_val, w_val = X[val_indices], y[val_indices], weights[val_indices]

    train_dataset = TensorDataset(X_train, y_train, w_train)
    model = model_class()
    optimizer = optim.Adam(model.parameters(), lr=lr, weight_decay=1e-4)
    criterion = nn.MSELoss(reduction="none")

    scheduler = torch.optim.lr_scheduler.CyclicLR(
        optimizer,
        base_lr=lr / 10,
        max_lr=lr * 10,
        step_size_up=len(train_dataset) // batch_size,
        mode="triangular2",
        cycle_momentum=False
    )

    load_model_weights_from_file(model, "weights_A.txt")

    for epoch in range(epochs):
        model.train()
        total_loss = 0
        for X_batch, y_batch, w_batch in DataLoader(train_dataset, batch_size=batch_size, shuffle=True):
            optimizer.zero_grad()
            preds = model(X_batch)
            sample_losses = criterion(preds, y_batch).view(-1)
            loss = (sample_losses * w_batch).mean()
            loss.backward()
            optimizer.step()
            scheduler.step()
            total_loss += loss.item() * X_batch.size(0)
        total_loss /= len(train_dataset)

        # Validation
        model.eval()
        with torch.no_grad():
            val_preds = model(X_val)
            val_sample_losses = criterion(val_preds, y_val).view(-1)
            val_loss = (val_sample_losses * w_val).mean().item()

        print(f"TD Epoch {epoch+1}/{epochs} → Train Loss: {total_loss:.6f} | Val Loss: {val_loss:.6f} | LR: {scheduler.get_last_lr()[0]:.6f}")

    torch.save(model.state_dict(), MODEL_CHECKPOINT)
    save_model_weights_to_file(model, "weights_A.txt")
    print("TD model training finished and saved.")
    return model


def generate_evals_file(model_class=FeatureEvaluator,
                        model_checkpoint=MODEL_CHECKPOINT,
                        data_file="all_games_depth_2.csv",
                        output_file=EVALS_FILE):
    model = model_class()
    if not Path(model_checkpoint).exists():
        raise FileNotFoundError(f"Model checkpoint {model_checkpoint} not found")
    load_model_weights_from_file(model, "weights_A.txt")
    model.eval()

    use_cols = [f"f{i}" for i in range(FEATURE_COUNT)] + ["target"]
    df = pd.read_csv(data_file, usecols=use_cols, on_bad_lines='skip')
    df = df.apply(pd.to_numeric, errors='coerce').replace([np.inf, -np.inf], np.nan).dropna().reset_index(drop=True)

    feature_cols = [f"f{i}" for i in range(FEATURE_COUNT)]
    X = df[feature_cols].values.astype(np.float32)
    X_tensor = torch.tensor(X, dtype=torch.float32)
    with torch.no_grad():
        df['eval'] = model(X_tensor).cpu().numpy()

    with open(output_file, "w") as f:
        for i, row in df.iterrows():
            features = [row[f] for f in feature_cols]

            if all(v == -1 for v in features):
                f.write("\n")
                continue

            values = features + [row['eval']] + [row['target']]
            f.write(",".join(map(str, values)) + "\n")

    print(f"Saved {len(df)} rows with evaluations to {output_file}")


def selfplay_training_loop():
    for i in range(2):  # two-phase LR schedule
        lr = 0.01 if i == 0 else 0.001
        train_td_model(model_class=FeatureEvaluator,
                       evals_file=EVALS_FILE,
                       epochs=30,
                       batch_size=1024,
                       lr=lr)


if __name__ == "__main__":
    generate_evals_file()
    selfplay_training_loop()
