# LOTUS CORE

[![Latency](https://img.shields.io/badge/latency-0.031ms-brightgreen)](#benchmarks)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-GPLv3-blue)](LICENSE)

> A high-performance, zero-allocation Vietnamese input method engine written in C++20.

- [What is Lotus Core?](#what-is-this)
- [Installation](#installation)
- [Usage](#usage)
- [Benchmarks](#benchmarks)
- [Support](#support)

## What is this?

Lotus Core is a low-latency, modular processing engine designed to transform key inputs into Vietnamese output. It is built specifically for developer-centric environments where raw performance and linguistic correctness are non-negotiable.

Unlike naive engines, Lotus Core intelligently differentiates between Vietnamese and English patterns, ensuring words like `status` or `expect` are preserved without manual switching.

## Installation

### Building from Source

Requirements: `cmake ≥ 3.15`, `g++` or `clang++` supporting C++20.

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

With Lotus Core, you can embed a professional Vietnamese engine into any project via its stable C-API.

```c
#include <lotus_core/capi.h>

int main() {
    lotus_core_t* engine = lotus_core_create();
    lotus_modifiers_t mods = { false, false };

    // Input: 'v' + 'i' + 'e' + 't' + 'j' -> 'việt'
    lotus_core_process_key(engine, 'v', mods);
    lotus_core_process_key(engine, 'i', mods);
    lotus_core_process_key(engine, 'e', mods);
    lotus_core_process_key(engine, 't', mods);
    lotus_result_t r = lotus_core_process_key(engine, 'j', mods);

    // Output result
    printf("Transformed: %d chars\n", r.count);
    
    lotus_core_destroy(engine);
    return 0;
}
```

## Benchmarks

Lotus Core is optimized for zero heap allocation in the core processing loop.

| Operation | Average Latency | Peak |
| :--- | :--- | :--- |
| Keystroke Processing | **0.031 ms** | 0.44 ms |
| Syllable Parsing | **0.012 ms** | 0.08 ms |

## Support

- [x] **Telex** (with auto-restoration)
- [x] **VNI**
- [x] **Stable C-API**
- [x] **Zero-Allocation Architecture**

## Creator

- Huỳnh Thiện Lộc ([Github](https://github.com/hthienloc))

---
*Phát triển với sự tận tâm về ngôn ngữ và hiệu năng.*
