# C Regex Engine (Pike VM / State-Set Simulation)

A fully-featured, industrial-grade Regular Expression engine written entirely from scratch in C. 

Unlike standard recursive backtracking engines that suffer from exponential time complexity on complex patterns, this project implements a **State-Set Simulation (Pike VM)** architecture. This allows the engine to process strings with a strict $O(N)$ time complexity, making it completely immune to "Catastrophic Backtracking".

---

## Core Features

*   **Custom Compiler Architecture:** Features a hand-written Lexer (Tokenization) and Parser (AST/Expression resolution) that strictly respects operator precedence.
*   **NFA Graph Generation:** Converts regex tokens into a Non-deterministic Finite Automaton (NFA).
*   **Pike VM (State-Set Matching):** Traverses the NFA using parallel states, eliminating the need for backtracking.
*   **Capture Groups & Extraction:** Fully supports data extraction via capture groups `(...)` with industry-standard **Greedy Matching**.
*   **Custom CLI:** An interactive, Matrix-Green styled terminal interface with multilingual support.

## Supported Regex Syntax

The engine supports a wide range of standard regular expression operations:

*   **Quantifiers:** `*` (0 or more), `+` (1 or more), `?` (0 or 1), `{n,m}` (range repetition)
*   **Alternation & Grouping:** `|` (OR operator), `(...)` (Capture Groups)
*   **Character Classes:** `[abc]`, `[a-zA-Z0-9]`, `[^0-9]` (Negative classes)
*   **Anchors:** `^` (Start of string), `$` (End of string)
*   **Escape Sequences:** `\d` (digits), `\w` (word characters), `\s` (whitespace)
*   **Wildcard:** `.` (Any character except newline)
*   **Literal Escapes:** `\*`, `\+`, `\(`, etc.

## Installation & Compilation

This project is written in standard C99. You will need `gcc` and `make` (or `mingw32-make` on Windows) installed on your system.

**1. Clone the repository:**
```bash
git clone [https://github.com/cagatay005/regex-engine.git](https://github.com/cagatay005/regex-engine.git)
cd regex-engine
```
**2. Compile the project:**
_Using MinGW on Windows_
```bash
mingw32-make
```
_(On Linux/Mac, simply run make depending on your Makefile setup)._
**3. Run the engine:**
```bash
./regex_engine
```
## Usage & CLI Commands
_Once the engine is running, you will be greeted by the interactive terminal. You can enter a regex pattern, followed by the text you want to test._
**Available CLI Commands:
* **code:** Displays the help menu and command list.
* **clear:** Clears the terminal screen.
* **en:** Switches the interface language to English.
* **tr:** Switches the interface language to Turkish.
* **q:** Quits the application gracefully.