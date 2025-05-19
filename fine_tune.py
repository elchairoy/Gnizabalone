import torch
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
import matplotlib.pyplot as plt

# Constants
NUM_FEATURES = 7  # 7 original features + unused f8
NUM_WEIGHTED_FEATURES = 7  # f1 to f7
NUM_PHASES = 3  # Polynomial coefficients (1, modus, modus^2)
EPOCHS = 1  # More epochs for convergence
LEARNING_RATE = 0.01  # Lower for stability
LAMBDA_REG = 0.002  # Adjusted for scaled features
LAMBDA_MODUS_REG = 0.001  # Lighter for modus_weights
EPSILON = 1e-4  # Smooth abs approximation
BATCH_SIZE = 1  # Mini-batch training
SEED = 42  # For reproducibility

import pandas as pd
data = pd.read_csv("dataset.csv")
max_abs = data[["f1", "f2", "f3", "f4", "f5", "f6", "f7"]].abs().max()
print("Max-abs values:", (max_abs.values))


def clamp(x, low=0.0, high=1.0):
    return torch.clamp(x, low, high)

def compute_phase_weights(modus):
    # modus assumed normalized or roughly in [0,1], adjust if needed
    w_early = clamp(1 - 2 * modus)
    w_mid = clamp(1 - 2 * torch.abs(modus - 0.5))
    w_late = clamp(2 * modus - 1)
    return w_early.unsqueeze(1), w_mid.unsqueeze(1), w_late.unsqueeze(1)  # shape: [batch_size, 1]


# Load dataset
def load_dataset(filename):
    data = pd.read_csv(filename)
    features = data[[f"f{i+1}" for i in range(NUM_FEATURES)]].values
    targets = data["target"].values
    return features, targets

# Train weights
def train_weights(features, targets, modus_weights, eval_weights, epochs, lr, lambda_reg, lambda_modus_reg, batch_size):
    torch.manual_seed(SEED)  # Fix random seed
    optimizer = torch.optim.Adam([eval_weights, modus_weights], lr=lr)
    scheduler = torch.optim.lr_scheduler.StepLR(optimizer, step_size=20, gamma=0.5)
    train_losses = []
    val_losses = []

    max_abs = torch.abs(features).max(dim=0)[0] + 1e-8
    features_scaled = features / max_abs

    features_train, features_val, targets_train, targets_val = train_test_split(
        features_scaled, targets, test_size=0.1, random_state=SEED
    )

    features_train = torch.tensor(features_train, dtype=torch.float32)
    targets_train = torch.tensor(targets_train, dtype=torch.float32)
    features_val = torch.tensor(features_val, dtype=torch.float32)
    targets_val = torch.tensor(targets_val, dtype=torch.float32)

    for epoch in range(epochs):
        total_train_loss = 0
        num_batches = 0
        indices = torch.randperm(len(features_train))

        for i in range(0, len(features_train), batch_size):
            batch_indices = indices[i:i + batch_size]
            f = features_train[batch_indices]
            target = targets_train[batch_indices]

            # modus: [batch_size]
            abs_f = torch.abs(f)
            modus = (abs_f * modus_weights).sum(dim=1)  # scalar per batch example

            # normalize modus to [0, 1] if needed (optional)
            # modus = (modus - modus.min()) / (modus.max() - modus.min() + 1e-8)

            w_early, w_mid, w_late = compute_phase_weights(modus)  # each [batch_size, 1]

            # eval_weights shape: [NUM_FEATURES, NUM_PHASES] == [7, 3]
            weights_early = eval_weights[:, 0].unsqueeze(0)  # shape [1, 7]
            weights_mid = eval_weights[:, 1].unsqueeze(0)    # shape [1, 7]
            weights_late = eval_weights[:, 2].unsqueeze(0)   # shape [1, 7]

            # blended weights per batch: [batch_size, 7]
            blended_weights = w_early * weights_early + w_mid * weights_mid + w_late * weights_late

            # eval_score: sum of (blended_weights * features), sum over features dim
            eval_score = (blended_weights * f).sum(dim=1)  # shape: [batch_size]

            loss = ((eval_score - target)**2).mean()
            loss += lambda_reg * eval_weights.norm()**2
            loss += lambda_modus_reg * modus_weights.norm()**2

            total_train_loss += loss.item()
            num_batches += 1

            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

        scheduler.step()

        total_val_loss = 0
        with torch.no_grad():
            for i in range(0, len(features_val), batch_size):
                f = features_val[i:i + batch_size]
                target = targets_val[i:i + batch_size]
                # modus: [batch_size]
                abs_f = torch.abs(f)
                modus = (abs_f * modus_weights).sum(dim=1)  # scalar per batch example

                # normalize modus to [0, 1] if needed (optional)
                # modus = (modus - modus.min()) / (modus.max() - modus.min() + 1e-8)

                w_early, w_mid, w_late = compute_phase_weights(modus)  # each [batch_size, 1]

                # eval_weights shape: [NUM_FEATURES, NUM_PHASES] == [7, 3]
                weights_early = eval_weights[:, 0].unsqueeze(0)  # shape [1, 7]
                weights_mid = eval_weights[:, 1].unsqueeze(0)    # shape [1, 7]
                weights_late = eval_weights[:, 2].unsqueeze(0)   # shape [1, 7]

                # blended weights per batch: [batch_size, 7]
                blended_weights = w_early * weights_early + w_mid * weights_mid + w_late * weights_late

                # eval_score: sum of (blended_weights * features), sum over features dim
                eval_score = (blended_weights * f).sum(dim=1)  # shape: [batch_size]

                loss = ((eval_score - target)**2).mean()
                loss += lambda_reg * eval_weights.norm()**2
                loss += lambda_modus_reg * modus_weights.norm()**2
                total_val_loss += loss.item()

        train_losses.append(total_train_loss / num_batches)
        val_losses.append(total_val_loss / (len(features_val) // batch_size))
        print(f"Epoch {epoch}, Train Loss: {train_losses[-1]}, Val Loss: {val_losses[-1]}, "
              f"Eval Weights Norm: {eval_weights.norm().item()}, Modus Weights Norm: {modus_weights.norm().item()}")

    return train_losses, val_losses

# Main
filename = "dataset.csv"
features, targets = load_dataset(filename)
features = torch.tensor(features, dtype=torch.float32)
targets = torch.tensor(targets, dtype=torch.float32)

torch.manual_seed(SEED)
modus_weights = torch.randn(NUM_WEIGHTED_FEATURES, requires_grad=True)
eval_weights = torch.randn(NUM_FEATURES, NUM_PHASES, requires_grad=True)

train_losses, val_losses = train_weights(
    features, targets, modus_weights, eval_weights, EPOCHS, LEARNING_RATE, LAMBDA_REG, LAMBDA_MODUS_REG, BATCH_SIZE
)

plt.plot(train_losses, label="Train Loss")
plt.plot(val_losses, label="Validation Loss")
plt.xlabel("Epoch")
plt.ylabel("Loss")
plt.legend()
plt.savefig("training_progress.png")
plt.close()

eval_weights_np = eval_weights.detach().numpy()
np.savetxt("weights.txt", eval_weights_np)
modus_weights_np = modus_weights.detach().numpy()
np.savetxt("modus_weights.txt", modus_weights_np)
