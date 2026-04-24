# Lotus Engine Python Wrapper

A high-performance C-API wrapper for integrating Lotus Engine directly into Python environments using purely `ctypes` (no external dependencies).

## Installation

Ensure that `liblotus_engine_core.so` (or `.dll` on Windows / `.dylib` on macOS) is available in the library path, or compiled within the main project. You can compile the project using standard CMake tools or the provided script:

```bash
cd .. && ./dev.sh
```

## Quick Start

The Python wrapper closely mimics the core Engine C-API.

```python
from python.lotus_engine import LotusEngine, Method, ToneStyle, FreeW

def main():
    # 1. Initialize Engine
    engine = LotusEngine()
    
    # 2. Setup Configuration
    engine.set_method(Method.TELEX)
    engine.set_tone_style(ToneStyle.NEW)
    engine.set_free_w(FreeW.ALWAYS)
    engine.set_auto_restore(True)
    
    # 3. Add Custom Shortcuts
    engine.add_shortcut("vn", "Việt Nam")

    # 4. Process Key Presses
    # We pass the typed character and modifiers (like Shift)
    char = "a"
    action, backspace, output_str = engine.process_key(char, shift=False)
    
    print(f"Action: {action}, Backspaces: {backspace}, New String: {output_str}")
    
    # Reset engine state between distinct words or when resetting UI
    engine.reset()

if __name__ == "__main__":
    main()
```

## Running the Demo

A simple interactive script `demo.py` is available:

```bash
python3 demo.py
```

## API Features
- **Key Processing**: Handles character actions and deletion values (for creating custom input frontends).
- **Shortcuts**: Register and test text expansions.
- **Auto-Restore**: Support for the engine's built-in English restoration logic.
- **Tone Handling**: Switch easily between TELEX / VNI and tone styles (e.g., Hòa vs. Hoà).

## Identity
Author: **Huỳnh Thiện Lộc**