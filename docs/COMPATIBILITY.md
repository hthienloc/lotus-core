# 🤝 Compatibility Registry

> This document tracks the compatibility of Lotus Core with various platforms, editors, and environments. We are continuously working to expand support.

---

## ✅ Confirmed Support

| Environment | Status | Notes |
| :--- | :--- | :--- |
| **Terminal (TUI)** | ✅ Native | Tested via `lotus_tui` |
| **Vim / Neovim** | ✅ Excellent | No known phantom backspace issues |
| **VS Code** | 🟡 Partial | FFI integration possible. Electron environment requires careful NFC handling. |
| **Claude Code** | 🟡 Partial | Tested via CLI wrappers. |

---

## 🛠️ Platform Notes

### Electron / Web-based Apps (VS Code, Discord, Slack)
Electron apps are notorious for handling Unicode composition poorly, often resulting in the "phantom backspace" bug (where the app deletes too many or too few characters during a transformation). 

Lotus Core mitigates this by enforcing **Strict NFC Output**. The engine always outputs precomposed characters (e.g., `â` instead of `a` + `^`). 

### Terminal Environments (Vim, Tmux, Alacritty)
Terminal emulators generally handle our output perfectly, as long as the FFI wrapper correctly translates the `EngineResult` actions (Backspace + Insert) into standard ANSI escape codes or standard input events.

---

## 💡 Implementation Tips for Wrapper Developers

If you are building an IME, VS Code extension, or system-level hook using Lotus Core, keep these technical tips in mind:

1. **Handle the Result Action**: The `EngineResult` struct provides exactly how many characters to delete (`r.backspace`) and what to insert (`r.chars`). You **must** perform the backspaces before inserting the new characters.
   
2. **NFC Normalization**: Lotus Core outputs NFC characters natively. However, if your frontend performs its own normalization (e.g., NFD on macOS), you must intercept and normalize inputs back to NFC before passing them to `lotus_core_process_key`. The engine's internal phonotactic logic relies on Unicode boundaries.

3. **Pass-through Keys**: If `r.action == 0`, the engine did not transform the input. You should pass the raw key directly to the OS/Editor.

4. **Modifier Keys**: Pass Shift and Caps Lock states accurately via `lotus_modifiers_t`. The engine handles proper-casing dynamically based on these.
