# Lotus Core 🪷

[![CI](https://github.com/hthienloc/lotus-core/actions/workflows/ci.yml/badge.svg)](https://github.com/hthienloc/lotus-core/actions/workflows/ci.yml)
[![Latency](https://img.shields.io/badge/avg%20latency-0.031%20ms-brightgreen)](https://github.com/hthienloc/lotus-core)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

> **Lotus Core** is a high-performance, zero-dependency, modular Vietnamese input processing core. Built with modern C++20, it focuses on phonological accuracy and strict linguistic correctness, embeddable in any project via a stable C-API.

---

## 👁️ Project Vision

**"Vietnamese input for the most demanding developer environments."**

Lotus Core envisions a future where Vietnamese developers don't have to compromise between raw performance, strict linguistic correctness, and modern development environments. We are building the ultimate embeddable processing core that handles everything from casual typing to edge-case phonotactics without breaking a sweat.

**Official Logo Concept:** (Coming Soon) A stylized Lotus flower dynamically formed from modern keyboard keycaps, symbolizing the intersection of Vietnamese cultural identity and advanced software engineering.

---

## ✨ Core Features

| Feature | Description |
| :--- | :--- |
| 🔤 **TELEX & VNI** | Full support for both standard Vietnamese input methods |
| 🧠 **Phonotactic Validator** | Validates syllable structure (Initial · Glide · Nucleus · Final) against Vietnamese orthographic rules |
| 🔁 **Resilient Recovery** | Auto-reverts invalid compositions and preserves raw keystrokes (e.g. `status`, `what`) |
| 🎵 **Tone Placement** | Smart heuristics for complex vowel clusters (`iêu`, `uôi`, `ươi`, `ươu`, `ưa`) |
| 🎨 **Tone Style** | Old style (`hòa`) and New style (`hoà`) selectable at runtime |
| 🔠 **NFC Output** | All output is Unicode NFC precomposed — eliminates phantom backspace bugs in Electron/Web apps |
| ⚡ **< 0.1ms/key** | Measured average 0.031ms, peak 0.44ms under heavy stress tests |
| 🔌 **C-API** | Stable `lotus_core_t*` handle, log callbacks, no C++ ABI leakage |

---

## 🔤 Smart Auto-Restore Comparison

When typing English or mixed-language text, naive engines often mistakenly apply Vietnamese diacritics. Lotus Core intelligently auto-restores to the original keystrokes, ensuring a seamless typing experience.

| Word Typed | Naive Telex | Lotus Core | Reason |
| :--- | :--- | :--- | :--- |
| `status` | statú ❌ | **status** ✅ | Protected final 's' cluster |
| `for` | fỏ ❌ | **for** ✅ | Invalid Vietnamese initial 'f' |
| `what` | ưhat ❌ | **what** ✅ | Invalid Vietnamese initial 'w' |
| `qquas` | qquás ❌ | **qquas** ✅ | Non-Vietnamese 'qq' cluster |
| `cs` | cớ ❌ | **cs** ✅ | Phonotactically invalid |
| `expect` | ẽpect ❌ | **expect** ✅ | English heuristic detection |

---

## 🏗️ Architecture

Lotus Core implements a robust processing pipeline that prioritizes validation before transformation:

1. **Engine Layer**: Orchestrates the input pipeline and manages the Composition Buffer.
2. **Parser Layer**: Decomposes strings into Vietnamese phonological components (Initial, Glide, Nucleus, Final).
3. **Validator Layer**: Enforces strict orthographic rules to ensure absolute linguistic correctness.

> 📖 For an in-depth look, check out the [ARCHITECTURE.md](docs/ARCHITECTURE.md) document. 
> You can also explore our [Documentation Index](docs/README.md) and [Gemini Mandates](GEMINI.md).

---

## 💻 Usage & Integration

### Interactive TUI Demo

```bash
./build/lotus_tui           # Telex mode (default)
./build/lotus_tui --vni     # VNI mode
```
*(Press `ESC` to exit and dump the debug log)*

### C-API Integration

Link against `liblotus_core_core.so` (or `.a`) and include `<lotus_core/capi.h>`:

```c
#include <lotus_core/capi.h>
#include <stdio.h>

void my_logger(lotus_log_level_t level, const char* msg) {
    if (level >= LOTUS_LOG_LEVEL_ERROR)
        fprintf(stderr, "[lotus] %s\n", msg);
}

int main(void) {
    lotus_core_set_log_callback(my_logger);

    lotus_core_t* engine = lotus_core_create();
    lotus_modifiers_t mods = { false, false }; // shift, caps_lock

    // Process a keystroke: 'a' + 'a' -> 'â'
    lotus_core_process_key(engine, 'a', mods);
    lotus_result_t r = lotus_core_process_key(engine, 'a', mods);

    // r.backspace = number of chars to delete before inserting
    // r.count = number of new UTF-32 chars in r.chars buffer
    // r.action = 0 (pass-through) or 1 (transformation)
    printf("Backspace: %d, Insert: %d chars\n", r.backspace, r.count);

    lotus_core_destroy(engine);
    return 0;
}
```

---

## 🛠️ Development

**Requirements:** `cmake ≥ 3.15`, `g++` or `clang++` with C++20 support.

```bash
# Build the project and run all tests
./dev.sh

# Build and launch the interactive TUI demo
./dev.sh tui

# Build and run the performance benchmark
./dev.sh bench
```

---

## 📄 License

GPL v3 © [hthienloc and contributors](https://github.com/hthienloc/lotus-core)
