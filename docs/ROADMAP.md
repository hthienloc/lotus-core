# Project Roadmap: Lotus Core

Lotus Core is a high-performance, developer-centric core engine designed for maximum linguistic accuracy and efficiency. This roadmap outlines the strategic evolution of the engine across clearly defined technical phases.

---

## Phase 1: Core Phonology and Unicode Stability (Completed)

- [x] **Vietnamese Phonology Base**
  - Robust Syllable Parser with Initial, Glide, Nucleus, and Final consonant separation.
  - Component-based validation with strict orthic rules and exhaustive rhyme coverage.
- [x] **Unicode and Integration Layer**
  - Guaranteed NFC output for cross-platform rendering stability.
  - Comprehensive handling of combining marks.
  - Robust C-API designed for seamless FFI integration.

## Phase 2: High-Performance and Clean Architecture (Active)

- [x] **Advanced Transformation Engine**
  - Full TELEX and VNI method support with shortcut expansion system.
- [x] **Architectural Modularization**
  - Decomposed Engine into ContextTracker, InputDispatcher, and CompositionBuffer.
  - Table-driven logic for all linguistic transformations.
- [ ] **Ultra-Low Latency and Zero-Allocation**
  - Transition internal buffers to stack-based structures to achieve a zero-allocation hot loop in `process_key`.
  - Goal: Maintain < 0.05ms average latency with minimal memory pressure.
- [ ] **Linguistic Diagnostic System**
  - Implement detailed diagnostic codes in `EngineResult` to explain validation failures (e.g., `INVALID_INITIAL_CODA_COMBINATION`).
- [x] **Modern Web UI**
  - Professional 2-column sidebar layout for the web demo.

## Phase 3: Intelligence and State Management (Upcoming)

- [ ] **Developer-First Auto-Restore Logic**
  - Tailor English detection heuristics for developer-specific environments (code syntax vs. prose).
- [ ] **Complex Vowel Reordering**
  - Gracefully handle out-of-order typing without losing composition state.
- [ ] **Surrounding Text API**
  - Deepen integration for robust state reconstruction and cursor-aware editing.

## Phase 4: Ecosystem and Long-Term Research (Future)

- [ ] **Ecosystem Maturity**
  - Develop idiomatic high-level bindings for Rust and Python.
  - Release an official CLI tool for testing and integration.
- [ ] **Property-Based Verification**
  - Implement automated verification using millions of generated keystroke combinations to ensure 100% linguistic correctness.
- [ ] **Matrix-Based Validation**
  - Migrate validator to a CV/VC matrix lookup for O(1) latency.
