# 🤖 Agent Instructions: Lotus Core

> This file provides context and instructions for AI agents (like Jules, Gemini, or GitHub Copilot) working on the `lotus-core` project.

---

## 📖 Project Context
`lotus-core` is a high-performance, modular Vietnamese Input Method (IM) engine written in C++20. It follows a 7/8-stage pipeline architecture (GoNhanh) and prioritizes linguistic correctness through phonotactic validation.

## ⚙️ Technical Standards
- **Language**: C++20.
- **Unicode**: Internally uses `char32_t` (UTF-32) for all logic. UTF-8 is only for boundaries.
- **Naming Conventions**:
  - **Classes/Structs**: `PascalCase` (e.g., `SyllableParser`).
  - **Methods/Variables**: `snake_case` (e.g., `transform_buffer()`).
  - **Constants/Enums**: `SCREAMING_SNAKE_CASE` (e.g., `KEY_BACKSPACE`).
- **Semantic Naming Principles**:
  - **Phonology**: Use precise terms (`Initial`, `Glide`, `Nucleus`, `Coda`, `Tone`). Avoid 'Start', 'End', 'Vowel', 'Mark'.
  - **Intent**: Use prefixes `transform_` (mutation), `validate_` (checks), `handle_` (routing), `build_` (construction), and `is_likely_` (heuristics).
- **Standards**: Strict Doxygen documentation for all public/private members. Explain the *linguistic rationale* for changes.
- **Formatting**: Adhere to `.clang-format`. Use `clang-format -i` before committing.
- **Safety**: NEVER delete or strip comments. Maintain existing architectural patterns.

## 🧩 Core Components
- `src/engine.cpp`: The main orchestrator. Keep it high-level.
- `src/validator.cpp`: Vietnamese linguistic rules. Add helper methods for new rules.
- `src/parser.cpp`: Syllable decomposition (Initial, Glide, Nucleus, Final).
- `include/lotus_core/unicode.h`: Character mapping and NFC normalization tables.

## 🔄 Development Workflow
AI agents MUST use the following tools and scripts:

1.  **Build & Test**: Run `./dev.sh` to compile and execute all unit tests. This is the primary verification step.
2.  **Manual Verification**: Use `./dev.sh tui` to launch an interactive demo for manual testing if needed.
3.  **Benchmarking (Optional)**: Performance is secondary. Only run `./dev.sh bench` if specifically requested. The main goal is correctness.
4.  **Debugging**: Use `LOTUSDEBUG=1` environment variable when running the TUI to see internal alignment logs.

## ⚠️ Critical Mandates
- **Zero-Regression**: Every bug fix MUST have a reproduction test case in `tests/`.
- **Surgical Edits**: Prefer minimal, targeted changes over file overwrites.
- **Canonicalization**: Always maintain deterministic state using `Syllable::to_keys`.
- **Modularity**: Do not bloat `Engine::process_key`. Extract logic into private helper methods.

## 🤝 Collaboration Protocol (Gemini & Jules)
This project uses a tiered AI workflow:
- **Gemini (Orchestrator)**: Acts as the Engineering Manager and Gatekeeper. Gemini defines task scopes, reviews code for linguistic correctness, ensures comment preservation, and performs final merges.
- **Jules (Primary Developer)**: Jules is the primary coding agent. It is responsible for implementing logic changes, refactoring, and following the technical standards defined above.

**Rules for Jules**:
1.  **Surgical Changes**: Focus ONLY on the requested logic. Avoid project-wide reformatting unless explicitly asked.
2.  **Incremental Updates**: Jules may push changes in stages. Wait for the session to be marked 'Completed' before finalizing.
3.  **Communication**: Use clear commit messages and explain any discrepancies between requirements and implementation (e.g., outdated tests).

## 📋 Task Delegation Patterns
If you are asked to "fix a bug" or "add a feature":
1.  **Research**: Find the relevant linguistic rule in `validator.cpp` or transformation in `engine.cpp`.
2.  **Reproduce**: Add a failing test case in `tests/test_engine.cpp` or similar.
3.  **Implement**: Fix the logic while adhering to the pipeline stages.
4.  **Validate**: Run `./dev.sh` and `./dev.sh bench`.
