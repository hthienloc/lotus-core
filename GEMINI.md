# GEMINI Foundational Mandates — Lotus Engine

This file defines the foundational constraints and operational standards for the `lotus-engine` project. These directives take absolute precedence over general defaults.

## Project Context

`lotus-engine` is a high-performance, modular Vietnamese Input Method (IM) engine. It is designed for low-latency processing while maintaining strict linguistic correctness.

## Core Directives

### 1. Architectural Integrity (READ THIS FIRST)

- **Modularity is Non-Negotiable**: Keep the engine decomposed. The `Engine::process_key` function must remain a high-level orchestrator. New features (e.g., new smart typing rules) must be added as private helper methods, not directly into the main loop.
- **Table-Driven Logic**: Vietnamese character transformations (casing, tones, NFC) MUST use the data-driven lookup tables in `include/lotus_engine/unicode.h`. Avoid re-introducing long `switch` statements or manual if-else chains for character mapping.
- **Rule-Based Validation**: Linguistic rules in `Validator::is_valid` must be encapsulated in semantic helper methods (e.g., `check_front_vowel_affinity`).
- **Determinism through Canonicalization**: The engine MUST maintain a deterministic state. Any valid Vietnamese syllable in the active buffer or history should be stored/reclaimed in its canonical form (via `Syllable::to_keys`). This ensures that backspacing and late-transformation behaviors are consistent regardless of the original raw input sequence (e.g., `ddoanj` vs `ddoajn`).

### 2. Engineering Standards

- **Unicode awareness**: All internal logic and composition buffers MUST use `char32_t`. Conversion to UTF-8 should only happen at the boundary (Engine output or file I/O).
- **Zero-Regression Policy**: Every bug fix or feature MUST have a reproduction test case in `tests/`. Before finishing a task, run the full suite: `./dev.sh`.
- **Test Integrity (STRICT)**: NEVER modify existing test case expectations or delete existing tests without explicit user permission.
- **New Tests (STRICT)**: NEVER add new test cases or new test files without explicitly asking the user for permission first.
- **Surgical Changes (STRICT)**: Always prefer targeted, surgical edits using the `replace` tool over overwriting entire files.
- **Comment Preservation (TOP PRIORITY)**: NEVER arbitrarily delete, strip, or simplify comments (Doxygen or internal notes). Maintaining the codebase's documentation context is a non-negotiable priority. Any edit that results in lost comments must be reverted and fixed.
- **Doxygen Documentation**: Maintain strict Doxygen standards for all class methods and members. Comments should explain the *why* and the *linguistic rationale* for Vietnamese-specific logic.
- **Performance**: Always run `./dev.sh bench` after making changes to the core transformation logic (`engine.cpp` or `parser.cpp`).

## Workflow

1. **Research**: Reproduce bugs in `tui_demo.cpp` or a new test file first.
2. **Implement**: Use surgical edits. Keep helper methods private in `Engine` unless they are truly generic.
3. **Self-Audit (MANDATORY)**: Immediately after an edit, review the diff to ensure no important comments were accidentally stripped and that the code follows project formatting/naming conventions.
4. **Verify**: Run `./dev.sh` (tests). Benchmarking (`./dev.sh bench`) is optional and should not be prioritized over logical correctness.
- **Debug Log**: When using `tui_demo`, use `LOTUSDEBUG=1` to see the aligned ASCII table log. Maintain the fixed-width formatting using `unicode::display_width`.

## Key Files

- `include/lotus_engine/unicode.h`: Character mapping & NFC normalization.
- `src/engine.cpp`: Orchestrator of input methods.
- `src/validator.cpp`: Vietnamese linguistic rules.
- `src/parser.cpp`: Syllable decomposition logic.
- `src/linguistics.cpp`: Heuristics for English/Vietnamese detection.
