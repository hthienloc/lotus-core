# Lotus Engine

[![CI](https://github.com/fcitx5-lotus/lotus-engine/actions/workflows/ci.yml/badge.svg)](https://github.com/fcitx5-lotus/lotus-engine/actions/workflows/ci.yml)
[![Latency](https://img.shields.io/badge/avg%20latency-0.016%20ms-brightgreen)](https://github.com/fcitx5-lotus/lotus-engine)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)

> **Lotus Engine** is a high-performance Vietnamese input processing core — phonologically accurate, zero-dependency, and embeddable via a stable C-API.

---

## Features

| Feature | Description |
| :--- | :--- |
| 🔤 **TELEX & VNI** | Full support for both standard Vietnamese input methods |
| 🧠 **Phonotactic Validator** | Validates syllable structure (Initial · Glide · Nucleus · Final) against Vietnamese orthographic rules |
| 🔁 **Resilient Recovery** | Auto-reverts invalid compositions and preserves raw keystrokes (e.g. `tests`, `nurses`) |
| 🎵 **Tone Placement** | Smart heuristics for complex vowel clusters (`iêu`, `uôi`, `ươi`, `ươu`, `ưa`) |
| 🎨 **Tone Style** | Old style (`hòa`) and New style (`hoà`) selectable at runtime |
| 🔠 **NFC Output** | All output is Unicode NFC precomposed — eliminates phantom backspace bugs in Electron/Web apps |
| ⚡ **< 0.15ms/key** | Measured average 0.014ms, peak 0.14ms over 10,000 live keystrokes |
| 🔌 **C-API** | Stable `lotus_engine_t*` handle, log callbacks, no C++ ABI leakage |

---

## Build

**Requirements:** `cmake ≥ 3.15`, `g++ / clang++` with C++20 support.

```bash
git clone https://github.com/fcitx5-lotus/lotus-engine.git
cd lotus-engine
cmake -B build
cmake --build build -j$(nproc)
```

### Using the Dev Script

```bash
./dev.sh          # Build + run tests
./dev.sh tui      # Build + launch interactive TUI demo
./dev.sh bench    # Build + run performance benchmark
./dev.sh clean    # Clean rebuild
```

---

## Usage

### Interactive TUI Demo

```bash
./build/lotus_tui           # Telex mode (default)
./build/lotus_tui --vni     # VNI mode
```

Type Vietnamese naturally and press `ESC` to exit with a debug log.

---

### C-API Integration

Link against `liblotus_engine_core.so` (or `.a`) and include `lotus_engine/capi.h`:

```c
#include <lotus_engine/capi.h>
#include <stdio.h>

// Optional: receive engine log events
void my_logger(lotus_log_level_t level, const char* msg) {
    if (level >= LOTUS_LOG_LEVEL_WARN)
        fprintf(stderr, "[lotus] %s\n", msg);
}

int main(void) {
    lotus_engine_set_log_callback(my_logger);

    lotus_engine_t* engine = lotus_engine_create();
    lotus_modifiers_t mods = { false, false };

    // Process a keystroke: "aa" -> "â"
    lotus_engine_process_key(engine, 'a', mods);
    lotus_result_t r = lotus_engine_process_key(engine, 'a', mods);

    // r.backspace = number of chars to delete before inserting
    // r.chars[0..r.count] = new chars to insert
    printf("Backspace: %d, Insert: %d chars\n", r.backspace, r.count);

    lotus_engine_destroy(engine);
    return 0;
}
```

**CMake (after `cmake --install`):**

```cmake
find_package(lotus_engine REQUIRED)
target_link_libraries(my_app PRIVATE lotus_engine_core)
```

---

## Architecture

```text
lotus-engine/
├── include/lotus_engine/   # Public headers (C++ + C-API)
│   ├── capi.h              # Stable C-API
│   ├── engine.h            # C++20 Engine class
│   ├── types.h             # Syllable, Tone, InputMethod types
│   ├── validator.h         # Phonotactic validator
│   ├── parser.h            # Syllable decomposer
│   ├── unicode.h           # UTF-8 ↔ UTF-32 utilities
│   └── log.h               # Internal logging macros
├── src/                    # Implementation
├── tests/                  # Unit tests + TUI demo + benchmark
│   ├── bench_engine.cpp    # Latency benchmark (10k keypresses)
│   └── tui_demo.cpp        # Interactive terminal demo
└── .github/workflows/ci.yml
```

7-stage TELEX/VNI pipeline: **Stroke → Vowel Modifiers → Free-W → Tone → Compose → Validate → Recover**

See [`ARCHITECTURE.md`](ARCHITECTURE.md) for full details.

---

## Performance

Measured on release build (`-O2`), 10,000 consecutive keypresses:

| Metric | Value |
| :--- | :--- |
| Average latency | **0.014 ms** |
| Peak latency | **0.14 ms** |
| Target SLA | < 1.0 ms ✅ |

Run yourself: `./dev.sh bench`

---

## Development

```bash
# Run full test suite
./dev.sh

# Format code (requires clang-format)
clang-format -i src/*.cpp tests/*.cpp include/lotus_engine/*.h

# Static analysis (optional)
clang-tidy src/*.cpp -- -std=c++20 -Iinclude
```

---

## License

GPL v3 © [fcitx5-lotus contributors](https://github.com/fcitx5-lotus)
