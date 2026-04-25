# Technical Comparison Report: Vietnamese IM Engines

This report analyzes core architectural insights from three leading Vietnamese input method engines to guide the evolution of **Lotus Core**.

---

## 1. Bamboo Core (LotusInputMethod)
**Strength:** Linguistic Accuracy and ABI Stability.

### Key Insights:
- **Transformation Graph:** Instead of simple string replacement, Bamboo treats every keystroke as a node in a graph. Diacritics and tones are linked as dependencies to base characters.
- **Floating Tone Hook:** Implements a `refreshLastToneTarget` mechanism. When a syllable structure changes (e.g., `hòa` + `n` -> `hoàn`), the tone automatically repositions itself based on phonetic rules.
- **Opaque Handle C-API:** Uses ID-based handles (uintptr) for engine instances. This ensures absolute memory safety and stability for high-level bindings (Rust, Python, WASM).

**Actionable for Lotus Core:**
- [ ] Implement a dependency-based tone placement system.
- [ ] Transition C-API to an Opaque Handle pattern for better FFI safety.

---

## 2. NexusKey
**Strength:** Modern C++ Engineering and System Integration.

### Key Insights:
- **CharState Pattern:** Stores input as a `vector<CharState>` where each state explicitly holds `{base, modifier, tone, case}`. Unicode rendering only happens at the final stage.
- **Lock-Free Configuration:** Uses C++20 `std::atomic_ref` and shared memory for non-blocking configuration sync between the engine and the UI process.
- **Context Anchors:** Allows the engine to accept external "context hints" from platform-specific APIs (like Windows TSF), improving auto-capitalization accuracy.

**Actionable for Lotus Core:**
- [ ] Explore the `CharState` model to simplify backspacing and tone relocation logic.
- [ ] Use atomic state flags for thread-safe configuration management.

---

## 3. GoNhanh
**Strength:** Extreme Performance and Minimalism.

### Key Insights:
- **4-Stage Pipeline:** Uses a streamlined model: `Stroke -> Vowel -> Mark -> Cleanup`. This reduces the computational cycles per keystroke significantly.
- **Zero-Allocation Hot Loop:** Utilizes fixed-size stack-allocated arrays for all intermediate processing, avoiding heap churn entirely.
- **Aggressive Inlining:** Heavily relies on `inline` linguistic primitives and data-driven static tables.

**Actionable for Lotus Core:**
- [ ] Finalize the transition to `StaticString` (Zero-Allocation).
- [ ] Refactor the 7-stage pipeline into a consolidated 4-stage model for < 0.01ms latency.

---

## Summary of Strategy
Lotus Core will aim to be the **"Balanced Engine"**:
1. **Bamboo's Precision**: Through Floating Tone dependency logic.
2. **NexusKey's Modernity**: Through C++20 primitives and modular state.
3. **GoNhanh's Speed**: Through Zero-Allocation and a 4-stage pipeline.
