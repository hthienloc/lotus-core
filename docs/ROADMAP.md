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
- [ ] **Floating Tone Implementation**
  - Adopt Bamboo's dependency-based tone placement logic to automatically reposition diacritics when syllable structure changes.

## Phase 3: Intelligence and State Management (Upcoming)

- [ ] **Semantic CharState Model**
  - Explore the `CharState` pattern (NexusKey) to represent characters as `{base, modifier, tone}` instead of raw Unicode for simpler backspacing.
- [ ] **Streamlined 4-Stage Pipeline**
  - Consolidate the transformation stages into a `Stroke -> Vowel -> Mark -> Cleanup` model for maximum efficiency.
- [ ] **Developer-First Auto-Restore Logic**
  - Tailor English detection heuristics for developer-specific environments.
- [ ] **Surrounding Text API**
  - Deepen integration for robust state reconstruction using Context Anchors.

## Phase 4: Ecosystem and Long-Term Research (Future)

- [ ] **Opaque Handle C-API**
  - Standardize on an ID-based handle system for engine instances to improve FFI stability across Rust, Python, and WASM.
