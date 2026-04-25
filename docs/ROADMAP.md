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
  - Transition internal buffers to stack-based `StaticString` structures (inspired by GoNhanh).
  - Goal: Maintain < 0.01ms latency by eliminating heap churn.
- [ ] **Linguistic Diagnostic System**
  - Implement detailed diagnostic codes in `EngineResult` to explain validation failures.
- [x] **Modern Web UI**
  - Professional 2-column sidebar layout for the web demo.

## Phase 3: Semantic State and Logic Refinement (Upcoming)

- [ ] **Semantic CharState Model**
  - Explore representing characters as `{base, modifier, tone}` instead of raw Unicode to simplify backspacing and diacritic management.
  - Goal: Make the code more intuitive by working with linguistic units directly.
- [ ] **Smart Tone Repositioning**
  - Implement a dedicated helper to automatically move the tone to the correct nucleus as the syllable grows, ensuring Bamboo-level precision with simple string logic.
- [ ] **Developer-First Auto-Restore Logic**
  - Tailor English detection heuristics for developer-specific environments.

## Phase 4: Ecosystem and Long-Term Stability (Future)

- [ ] **Opaque Handle C-API**
  - Standardize on an ID-based handle system for engine instances to improve FFI stability.
- [ ] **Property-Based Verification**
  - Implement automated verification using millions of generated keystroke combinations to ensure 100% linguistic correctness.
