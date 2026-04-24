# 🗺️ Project Roadmap: Lotus Core

> Lotus Core is positioned as a high-performance, developer-centric core engine that bridges the gap between Bamboo's linguistic accuracy and GoNhanh's modern efficiency. This roadmap outlines the strategic evolution of the engine, structured into clearly defined Technical Phases.

---

## ✅ Phase 1: Core Phonology & Unicode Stability (Completed)

- [x] **Vietnamese Phonology Base**
  - Robust Syllable Parser with Initial, Glide, Nucleus, and Final consonant separation.
  - Component-based validation with strict orthic rules and exhaustive 173+ rhyme coverage.
- [x] **Unicode & Integration Layer**
  - Guaranteed NFC output for cross-platform rendering stability.
  - Comprehensive handling of combining marks.
  - Robust C-API (`liblotus_core_core.so`) designed for seamless FFI integration (inspired by the stability of bamboo-core).

## 🏃 Phase 2: High-Performance Processing Pipeline (Active)

*Objective: Evolve the transformation architecture to achieve GoNhanh's ultra-low latency while maintaining the nuanced correctness of Bamboo.*

- [x] **Advanced Transformation Engine**
  - Full TELEX and VNI method support with shortcut expansion system.
  - Initial 7-stage transformer implementation.
- [ ] **Streamlined 4-Stage Pipeline Integration**
  - Refactor the current 7/8-stage pipeline into a highly optimized 4-stage processing pipeline (Stroke, Vowel, Mark, Cleanup) analogous to GoNhanh's architecture, drastically reducing cycle count per keystroke.
- [ ] **Ultra-Low Latency Design**
  - Optimize memory footprint and state transitions to guarantee < 0.05ms average processing latency.
  - Implement zero-allocation data paths for the hot loop in `process_key`.
- [x] **Complex Tone Placement Heuristics**
  - Smart heuristics for complex vowel clusters (`iêu`, `uôi`, `ươi`, `ươu`, `ưa`).
  - Runtime selectable Tone Style (Old style `hòa` vs New style `hoà`), matching bamboo-core's precision.

## 🧠 Phase 3: Developer-Centric Intelligence & State Management (Upcoming)

*Objective: Enhance the engine's awareness of context, making it robust for complex environments like IDEs, CLIs, and modern web apps.*

- [ ] **Developer-First Auto-Restore Logic**
  - Implement an advanced English auto-restore mechanism tailored for developer tools (e.g., CLI inputs, Claude Code, VIM).
  - Intelligently differentiate between fast Vietnamese typing and code syntax, reverting to raw keystrokes instantly upon detecting non-Vietnamese phonotactics without explicit whitelist bloat.
- [ ] **Complex Vowel Reordering Logic**
  - Adopt advanced vowel reordering algorithms (inspired by bamboo-core) to gracefully handle out-of-order typing without dropping the composition state.
- [ ] **Context-Aware Composition**
  - Deep surrounding-text API integration for robust state reconstruction (`rebuild_from_text`).
  - Cursor-aware editing to allow mid-syllable diacritic modification.
  - Resilient backspace chaining across word boundaries.

## 🔭 Phase 4: Long-Term Research & Extensibility (Future)

- [ ] **Matrix-Based Validation Graph**
  - Migrate the linear phonotactic validator to a CV/VC matrix lookup for O(1) validation latency.
- [ ] **Structured Composition Graph**
  - Replace the flat raw buffer with a directed acyclic graph (DAG) to represent complex Undo/Redo states natively within the engine.
- [ ] **Plugin & Extension API**
  - Expand the C-API to allow dynamic loading of custom shortcut dictionaries and localized linguistic rules.
