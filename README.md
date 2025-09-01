# Gnizabalone – Abalone AI Bot

**Gnizabalone** is a simple Abalone board game bot written in C. It provides a basic AI for playing Abalone via the terminal using a minimal UCI-like protocol. For ease of use, a lightweight web interface is included, which communicates with the bot through a Node.js server.

---

## Features

* Abalone AI written in C
* Minimal UCI-like terminal protocol
* Optional web interface with basic GUI
* Easy to build and run

---

## Contents

```
src/          # C source code
include/      # Header files
botserver.js  # Node.js server to interface with the bot
index.html    # Basic web GUI
Makefile      # Build instructions
```

---

## Requirements

* **GCC** compiler
* **Node.js** (for the web interface)
* **Express and Cors library** for Node.js server

To install Express, run:

```bash
npm install express cors
```

---

## Building the Bot

Clone the repository:

```bash
git clone https://github.com/yourusername/Gnizabalone.git
cd Gnizabalone
```

Build the C bot locally:

```bash
make
```

This will produce the executable in a local folder (bin/).

---

## Running the Bot

### Terminal Mode

Run the bot directly in the terminal:

```bash
./bin/Gnizabalone.exe
```

The bot will interact via a simple UCI-like protocol.

---

### Web Interface

1. Start the Node.js server:

```bash
node botserver.js
```

2. Open `index.html` in a browser.
3. The page will communicate with the bot on port `3000`, allowing you to play using the GUI.

---

## About

Gnizabalone is currently **the strongest Abalone bot I’m aware of**. It was created by **Elchairoy Meir**.

Special thanks to **Vincent Forchot**, 10× Abalone world champion (who lost to the bot in a match), for his guidance and advice, and to Ishai for the inspiration.

---

## Notes

* The web interface is minimal and meant for convenience.

