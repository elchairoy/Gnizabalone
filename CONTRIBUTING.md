# Contributing to Gnizabalone

Thank you for your interest in developing **Gnizabalone**, the Abalone AI bot. This document explains the core design and how you can extend or improve the project.

---

## Core Components

### Evaluation Function

* The evaluation function is based on a **neural network**.
* The features are mainly based on center proximity, material balance, cohesion and number of 3 marbles in a row. 
* The evaluation function is perhaps the **most important element** of the bot: improving it significantly increases strength.

### Search Algorithm

* The engine uses **alpha–beta pruning** with:

  * **Null-move pruning**
  * **Quiescence search**
  * **Transposition table**

### Move Ordering

Moves are prioritized using:

* Killer moves
* Distance to center
* Number of marble pushes
* Number of lines of 3 created
* History heuristic

### Quiescence Search

* Only considers captures and moves that:

  * Push a certain number of marbles, and/or
  * Move towards the center.
* A score is assigned, and only moves above a threshold are searched.
* To prevent search explosion, only the **top 2 moves** are extended.

**Motivation:**
Abalone is often described as a quiet game, but in practice (especially in the opening and midgame) it is **highly chaotic** — almost every move can massively improve a player’s position. This makes quiescence search crucial for stability.

---

## Training the Evaluation

* The evaluation network is fine-tuned using **TD-learning** on datasets of self-play games.
* The **quality of the games** matters more than quantity, since the network is relatively small.
* The current evaluation function is far from optimal, it could be improved significantly with the right data (I suggest depth 5+ with quiescences).
* A key future improvement would be to bias the evaluation to **prefer winning more quickly**, e.g. by adding a small penalty per move.

---

## Possible Improvements

* **Evaluation**

  * Further fine-tuning for accuracy and faster wins.
* **Search**

  * Implementing **Late Move Reductions (LMR)**.
  * Improving aspiration windows.
  * Both are challenging because of the chaotic nature of Abalone — evals fluctuate at each depth.
* **Opening book**

  * Could provide a significant practical strength boost.
* **Parameters and Heuristics**

  * Fine-tuning the **move ordering formula**.
  * Adjusting the **quiescence depth** (currently max 10).
  * Tweaking the **threshold** and the **number of top moves** searched in quiescence.
  * Experimenting with the **null move reduction** and the **minimum depth** to apply null move search.

---

## Getting Help

If you need any help, you can contact the creator directly: **Elchairoy Meir ([elchairoy@gmail.com](mailto:elchairoy@gmail.com))**.
