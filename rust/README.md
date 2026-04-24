# Lotus Engine - Rust Wrapper

This crate provides safe, idiomatic Rust bindings for **Lotus Engine**, a zero-dependency, high-performance Vietnamese Input Method Core.

## Author

**Huỳnh Thiện Lộc**

## Prerequisites

The crate statically links to `lotus_engine_core`. You must build the core C++ library before compiling this crate.

From the root of the project:
```bash
./dev.sh
```

## Usage

Include this crate in your project, and ensure the build script can find the static library in the `../build` directory relative to this folder, or adapt the `build.rs` if integrating into a larger workspace.

### Example

```rust
use lotus_engine::{LotusEngine, Method};

fn main() {
    let mut engine = LotusEngine::new();
    engine.set_method(Method::Telex);

    let res = engine.process_key('a' as u32, false, false);
    println!("{:?}", res.chars); // ['a']

    let res = engine.process_key('s' as u32, false, false);
    println!("{:?}", res.chars); // ['á']
}
```

## Features Supported

* Configuration of TELEX and VNI methods.
* Configuration of Tone Styles.
* Engine keystroke processing with auto-restore logic.
* Setting of text expansion shortcuts.
