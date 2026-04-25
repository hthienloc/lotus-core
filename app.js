// Author: Huỳnh Thiện Lộc

document.addEventListener("DOMContentLoaded", () => {
    const editor = document.getElementById('editor');
    const logOutput = document.getElementById('log-output');
    const methodSelect = document.getElementById('method-select');
    const toneStyleSelect = document.getElementById('tone-style-select');
    const freeWSelect = document.getElementById('free-w-select');
    const stdUoCheckbox = document.getElementById('std-uo-checkbox');
    const autoRestoreCheckbox = document.getElementById('auto-restore-checkbox');
    const doubleSpaceCheckbox = document.getElementById('double-space-checkbox');
    const autoCapitalizeCheckbox = document.getElementById('auto-capitalize-checkbox');
    const nonStandardInitialsCheckbox = document.getElementById('non-standard-initials-checkbox');
    const macroModeSelect = document.getElementById('macro-mode-select');
    const backspaceStyleSelect = document.getElementById('backspace-style-select');
    const showDebugCheckbox = document.getElementById('show-debug-checkbox');
    const resetBtn = document.getElementById('reset-btn');

    let engine = null;
    let resultPtr = null;

    // Helper to log messages to UI
    function uiLog(level, msg) {
        if (level === 'debug' && !showDebugCheckbox.checked) {
            return;
        }

        const div = document.createElement('div');
        div.className = `log-entry ${level}`;
        div.textContent = `[${level.toUpperCase()}] ${msg}`;
        logOutput.appendChild(div);
        logOutput.scrollTop = logOutput.scrollHeight;
    }

    // Define Module for Emscripten
    const basePath = window.location.pathname.includes('/lotus-core/') ? '/lotus-core/' : '/';
    
    window.Module = {
        locateFile: function(path) {
            if (path.endsWith('.wasm')) {
                return basePath + path;
            }
            return path;
        }
    };

    // Initialize WASM Module
    window.Module.onRuntimeInitialized = function() {
        uiLog('info', 'WASM Module Loaded successfully.');

        // Initialize Lotus Core
        engine = Module._lotus_core_create();

        // Allocate memory for lotus_result_t
        // struct lotus_result_t { uint8_t action; uint8_t backspace; uint8_t count; uint8_t pad; uint32_t chars[32]; }
        // Offset 0: action (1)
        // Offset 1: backspace (1)
        // Offset 2: count (1)
        // Offset 3: padding (1)
        // Offset 4: chars array (32 * 4 = 128)
        // Total = 132 bytes
        resultPtr = Module._malloc(132);

        // Setup Log Callback
        const logCallback = Module.addFunction(function(level, msgPtr) {
            const msg = Module.UTF8ToString(msgPtr);
            let levelStr = 'debug';
            if (level === 1) levelStr = 'info';
            else if (level === 2) levelStr = 'error';
            uiLog(levelStr, msg);
        }, 'vii'); // void func(int, int)

        Module._lotus_core_set_log_callback(logCallback);

        uiLog('info', 'Lotus Core initialized.');
        
        updateConfig();
    };

    // Load the WASM script dynamically
    const script = document.createElement('script');
    script.src = basePath + 'lotus_core.js';
    document.body.appendChild(script);

    function updateConfig() {
        if (!engine) return;
        Module._lotus_core_set_method(engine, parseInt(methodSelect.value));
        Module._lotus_core_set_tone_style(engine, parseInt(toneStyleSelect.value));
        Module._lotus_core_set_free_w(engine, parseInt(freeWSelect.value));
        Module._lotus_core_set_std_uo(engine, stdUoCheckbox.checked ? 1 : 0);
        Module._lotus_core_set_auto_restore(engine, autoRestoreCheckbox.checked ? 1 : 0);
        Module._lotus_core_set_double_space_to_period(engine, doubleSpaceCheckbox.checked ? 1 : 0);
        Module._lotus_core_set_auto_capitalize(engine, autoCapitalizeCheckbox.checked ? 1 : 0);
        Module._lotus_core_set_allow_non_standard_initials(engine, nonStandardInitialsCheckbox.checked ? 1 : 0);
        Module._lotus_core_set_macro_mode(engine, parseInt(macroModeSelect.value));
        Module._lotus_core_set_backspace_style(engine, parseInt(backspaceStyleSelect.value));
        uiLog('info', 'Engine configuration updated.');
        editor.focus();
    }

    methodSelect.addEventListener('change', updateConfig);
    toneStyleSelect.addEventListener('change', updateConfig);
    freeWSelect.addEventListener('change', updateConfig);
    stdUoCheckbox.addEventListener('change', updateConfig);
    autoRestoreCheckbox.addEventListener('change', updateConfig);
    doubleSpaceCheckbox.addEventListener('change', updateConfig);
    autoCapitalizeCheckbox.addEventListener('change', updateConfig);
    nonStandardInitialsCheckbox.addEventListener('change', updateConfig);
    macroModeSelect.addEventListener('change', updateConfig);
    backspaceStyleSelect.addEventListener('change', updateConfig);

    resetBtn.addEventListener('click', () => {
        if (engine) {
            Module._lotus_core_reset(engine);
            editor.value = '';
            uiLog('info', 'Engine reset.');
            editor.focus();
        }
    });

    // Handle typing
    editor.addEventListener('keydown', (e) => {
        if (!engine || !resultPtr) return;

        // Skip IME composition events
        if (e.isComposing || e.keyCode === 229) {
            return;
        }

        // Only intercept normal character keys and backspace
        if (e.key.length > 1 && e.key !== 'Backspace' && e.key !== 'Enter') {
            if (['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown', 'Home', 'End', 'PageUp', 'PageDown'].includes(e.key)) {
                Module._lotus_core_reset(engine);
                uiLog('debug', `Navigation key '${e.key}' pressed, resetting engine buffer.`);
            }
            return;
        }

        if (e.ctrlKey || e.altKey || e.metaKey) {
            if (e.key === 'Backspace' || e.key === 'w' || e.key === 'W') {
                Module._lotus_core_reset(engine);
            }
            return; 
        }

        let keyChar = 0;
        if (e.key === 'Backspace') {
            keyChar = 8;
        } else if (e.key === 'Enter') {
            keyChar = 10;
        } else {
            keyChar = e.key.codePointAt(0);
        }

        const isShift = e.shiftKey ? 1 : 0;
        const isCaps = e.getModifierState("CapsLock") ? 1 : 0;

        // Call JS wrapper function
        Module._lotus_core_process_key_js(engine, keyChar, isShift, isCaps, resultPtr);

        // Fallback robust memory read for headless tests
        const heap8 = Module.HEAPU8 || window.HEAPU8;
        const heap32 = Module.HEAPU32 || window.HEAPU32;

        if (!heap8 || !heap32) {
             console.error("WASM HEAP arrays missing. Module not fully loaded.");
             return;
        }

        // Memory Dump & Verification
        let hexDump = [];
        for (let i = 0; i < 132; i++) {
            hexDump.push(heap8[resultPtr + i].toString(16).padStart(2, '0'));
        }
        uiLog('debug', "Memory Dump (132 bytes): " + hexDump.join(' '));

        // Read struct fields
        const action = heap8[resultPtr + 0];
        const backspaceCount = heap8[resultPtr + 1];
        const insertCount = heap8[resultPtr + 2];
        
        // Pass-through implementation
        if (action === 0 && backspaceCount === 0 && insertCount === 0) {
             // Let the browser handle standard typing, spaces, backspaces, and cursor moving naturally
             return;
        }

        e.preventDefault();

        // Build string to insert
        let insertStr = "";
        for (let i = 0; i < insertCount; i++) {
            // Correct the HEAPU32 access for word-indexed array. The char array starts at resultPtr + 4.
            const charCode = heap32[(resultPtr >> 2) + 1 + i];
            if (charCode !== 0) {
                insertStr += String.fromCodePoint(charCode);
            }
        }

        // Apply text updates using robust DOM manipulation
        editor.focus();
        const start = editor.selectionStart;
        const end = editor.selectionEnd;
        
        // Calculate the positions
        const deleteStart = Math.max(0, start - backspaceCount);
        
        // Build the new value:
        // 1. Prefix: Everything before the start of the deletion range
        // 2. New string to insert
        // 3. Suffix: Everything after the current selection end
        const prefix = editor.value.substring(0, deleteStart);
        const suffix = editor.value.substring(end);
        
        editor.value = prefix + insertStr + suffix;
        
        // Update the cursor position
        const newCursorPos = deleteStart + insertStr.length;
        editor.setSelectionRange(newCursorPos, newCursorPos);
    });
});
