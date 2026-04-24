# 🏗️ Vietnamese Engine Architecture

> **Lotus Core** is designed as a modular, high-performance Vietnamese Input Engine. This document outlines its architectural design principles and internal component structure.

---

## 📖 Overview

The engine is designed as a **processing-only core**. It lacks any I/O direct interaction, receiving key events from a frontend and returning strings to be displayed or committed.

## 🎯 Core Design Principles

1. **Pipeline-Driven**: Processing is divided into 7 configured stages.
2. **Linguistic-First**: High-fidelity Vietnamese syllable validation instead of generic regex.
3. **Stateless Result**: The outcome of every key press provides a full result state (text, number of backspaces, commit status).

## 🧩 Components

### 1. Engine Core (Orchestrator)

Coordinates the execution of the pipeline. It manages the `Composition Buffer` (list of raw physical keys).

- **File**: `engine.cpp` / `engine.h`

### 2. Phonology Module

The "Brain" of the engine. It parses strings into structured Vietnamese components.

- **Syllable Parser**: Identifies Initial, Glide, Vowel nucleus, and Final consonant.
- **Validator**: Enforces phonological rules using **Inclusion lists** (valid patterns).
- **Files**: `parser.cpp`, `validator.cpp`, `types.h`

### 3. Transformation Module

Applies typing method rules (Telex, VNI, etc.).

- **Stages**: Stroke (d->đ), Tone (aa->â), Mark (s->sắc).

### 4. State & Recovery

Handles undoing and restoring original input when a word becomes phonologically invalid.

## ⚙️ Pipeline Architecture

> **Note:** The engine strictly implements the **GoNhanh 8-Stage Pipeline**, prioritizing validation before any transformation.

### Processing Pipeline

1. **STAGE 1: Stroke** - Handles `dd` → `đ`.
2. **STAGE 2: Vowel Marks** - Handles `aa` → `â`, `ee` → `ê`, `oo` → `ô`, `aw` → `ă`, `ow` → `ơ`, `uw` → `ư`.
3. **STAGE 3: Mark** - Handles tone marks (Telex: `s, f, r, x, j`).
4. **STAGE 4: Remove** - Handles `z` (Telex) or `0` (VNI) to clear marks or tones.
5. **STAGE 5: W-Vowel** - Handles free-standing `w` → `ư` (Telex only).
6. **STAGE 6: Smart Typing** - Handles double-space to period and auto-capitalization.
7. **STAGE 7: Normal Letter** - Passthrough for normal characters.
8. **STAGE 8: Shortcut** - Expands abbreviations at word boundaries or immediately.

### Key Principles

- **Validation First**: Every transformation is gated by a phonotactic check via the `Validator`.
- **Stateless Buffer**: The engine maintains a raw key buffer, allowing a full re-process on every key for robust recovery.
- **Double-Key Revert**: Pressing the same modifier key twice reverts the transformation (e.g., `aa` → `â` → `aa`).

## 📚 Documentation & Maintenance

- **Doxygen Standards**: All core engine files and tests are documented using Doxygen-style comments for automated documentation generation and developer clarity.
- **Realistic Benchmarking**: Performance is validated using a multi-scenario benchmark suite that simulates formal writing, flexible typing, mixed-language sessions, and stress tests with complex syllables.


### Result Structure (FFI Compatible)

```cpp
struct EngineResult {
    uint32_t chars[32];   // UTF-32 output characters
    uint8_t action;       // 0=None (Passthrough), 1=Send (Replace), 2=Restore (Undo)
    uint8_t backspace;    // Number of characters to delete before inserting chars
    uint8_t count;        // Valid character count in 'chars'
};
```

## 🚀 Production-Grade Features

To align with modern input method expectations, the engine includes:

### 1. English Auto-Restore (Smart Fix)

When a user types a word that looks like English but was transformed into Vietnamese (e.g., `test` -> `tẽt`), the engine detects the invalidity and restores the raw keys upon pressing Space or Enter. This is powered by an internal whitelist of common English word patterns.

### 2. Backspace-after-Space Recovery

The engine maintains a `Word History` (Ring Buffer) of the last 10 committed words. If the user presses Backspace immediately after a Space, the engine "re-opens" the previous word into the active buffer, allowing seamless editing of previous syllables.

### 3. Case-Aware Shortcuts

Shortcuts are not just string replacements; they are case-aware. Triggers like `VN` or `Vn` will correctly expand to `VIỆT NAM` or `Việt Nam` using Unicode-aware transformation logic.

## 🔌 FFI Interface

The engine is designed to be C-compatible to allow integration into Windows (IME), Linux (fcitx5), and macOS environments.
