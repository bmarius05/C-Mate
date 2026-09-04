# ♟️ C-Mate 

![C++](https://img.shields.io/badge/Language-C%2B%2B-blue.svg)
![Protocol](https://img.shields.io/badge/Protocol-UCI-success.svg)
![Status](https://img.shields.io/badge/Status-Active-brightgreen.svg)

**C-Mate** is a custom-built, high-performance chess engine written entirely from scratch in C++. Developed by bmarius05, this engine relies on a 64-bit Bitboard architecture to navigate complex game trees and deliver strong moves in real-time. 

It is fully compliant with the **Universal Chess Interface (UCI)** protocol, meaning it seamlessly plugs into standard graphical interfaces like Arena Chess GUI, CuteChess, or En Croissant.

## ✨ Core Features

* **Universal Chess Interface (UCI):** Ready to play out-of-the-box against humans or other engines.
* **64-bit Bitboard Engine:** Board representation uses 64-bit integers for ultra-fast move generation and state validation.
* **Minimax Search:** Navigates millions of positions efficiently under strict time controls.
* **Positional Evaluation:** A static evaluation function combining material weighting and piece placement.
* **Full Chess Rule Implementation:** Flawlessly handles Castling (kingside/queenside), En Passant captures, and Pawn Promotions.
* **Robust FEN Parser:** Dynamically loads custom board states for puzzle solving and mid-game analysis.

## 🧠 Technical Deep Dive

Building **C-Mate** involved solving complex low-level memory and algorithmic challenges:

### 1. Mathematical Ray-Casting for Sliding Pieces
Instead of relying on slow iterative loops, sliding piece attacks (Bishops, Rooks, Queens) are generated using bitwise operations. By extracting the Least Significant Bit (LSB) or Most Significant Bit (MSB) based on the ray's trajectory relative to the origin square, the engine calculates blockages and captures in O(1) operations per direction.

### 2. State Reversibility (`makeMove` / `unmakeMove`)
To avoid the immense memory overhead of deep-copying the board state during the Minimax search, the engine uses a custom `MoveInfo` tracking structure. This struct snapshots historical data (castling rights, en passant squares, captured pieces) right before a move is made. When `unmakeMove` is called, the engine steps backward in time, resolving state mutations instantly in O(1) time.

### 3. Absolute Bitboard Synchronization
A common pitfall in engine development is the "Ghost Piece" phenomenon, where the visual board array desynchronizes from the underlying bitboards (especially the `EMPTY` bitboard). **C-Mate** features a strict `validateBoardState()` debugger that mathematically guarantees the `EMPTY` mask is always the exact bitwise NOT of the union of the White and Black piece masks, guaranteeing zero memory desyncs during tree traversal.

## 🚀 Installation & Usage

1. Dowload the [latest release](https://github.com/bmarius05/C-Mate/releases) executable
2. Upload the executable into your GUI of choice (I use [Arena Chess](http://www.playwitharena.de/))
3. Play the Engine


## Releases

| Version | Date | Notes |
|---|---|---|
| [v0.0.1](https://github.com/bmarius05/C-Mate/releases/tag/PreRelease) | 2026-09-04 | Working base version |
