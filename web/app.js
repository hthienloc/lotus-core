document.addEventListener("DOMContentLoaded", () => {
    const editor = document.getElementById('editor');
    const logOutput = document.getElementById('log-output');
    const methodSelect = document.getElementById('method-select');
    const toneStyleSelect = document.getElementById('tone-style-select');
    const freeWSelect = document.getElementById('free-w-select');
    const stdUoCheckbox = document.getElementById('std-uo-checkbox');
    const autoRestoreCheckbox = document.getElementById('auto-restore-checkbox');
    const resetBtn = document.getElementById('reset-btn');

    let engine = null;
    let resultPtr = null;

    // Helper to log messages to UI
    function uiLog(level, msg) {
        const div = document.createElement('div');
        div.className = `log-entry ${level}`;
        div.textContent = `[${level.toUpperCase()}] ${msg}`;
        logOutput.appendChild(div);
        logOutput.scrollTop = logOutput.scrollHeight;
    }

    // Define Module for Emscripten
    const basePath = window.location.pathname.includes('/lotus-engine/') ? '/lotus-engine/' : '/';
    
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

        // Initialize Lotus Engine
        engine = Module._lotus_engine_create();

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

        Module._lotus_engine_set_log_callback(logCallback);

        uiLog('info', 'Lotus Engine initialized.');
        
        updateConfig();
    };

    // Load the WASM script dynamically
    const script = document.createElement('script');
    script.src = basePath + 'lotus_engine.js';
    document.body.appendChild(script);

    function updateConfig() {
        if (!engine) return;
        Module._lotus_engine_set_method(engine, parseInt(methodSelect.value));
        Module._lotus_engine_set_tone_style(engine, parseInt(toneStyleSelect.value));
        Module._lotus_engine_set_free_w(engine, parseInt(freeWSelect.value));
        Module._lotus_engine_set_std_uo(engine, stdUoCheckbox.checked ? 1 : 0);
        Module._lotus_engine_set_auto_restore(engine, autoRestoreCheckbox.checked ? 1 : 0);
        uiLog('info', 'Engine configuration updated.');
        editor.focus();
    }

    methodSelect.addEventListener('change', updateConfig);
    toneStyleSelect.addEventListener('change', updateConfig);
    freeWSelect.addEventListener('change', updateConfig);
    stdUoCheckbox.addEventListener('change', updateConfig);
    autoRestoreCheckbox.addEventListener('change', updateConfig);

    resetBtn.addEventListener('click', () => {
        if (engine) {
            Module._lotus_engine_reset(engine);
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
                Module._lotus_engine_reset(engine);
                uiLog('debug', `Navigation key '${e.key}' pressed, resetting engine buffer.`);
            }
            return;
        }

        if (e.ctrlKey || e.altKey || e.metaKey) {
            if (e.key === 'Backspace' || e.key === 'w' || e.key === 'W') {
                Module._lotus_engine_reset(engine);
            }
            return; 
        }

        e.preventDefault();

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
        Module._lotus_engine_process_key_js(engine, keyChar, isShift, isCaps, resultPtr);

        // Read struct fields
        const action = Module.HEAPU8[resultPtr + 0];
        const backspaceCount = Module.HEAPU8[resultPtr + 1];
        const insertCount = Module.HEAPU8[resultPtr + 2];
        // Note: alignment means chars starts at offset 4
        const charsStartOffset = resultPtr + 4;
        
        // Manual backspace implementation if engine just passed it through
        if (keyChar === 8 && action === 0 && backspaceCount === 0 && insertCount === 0) {
             const start = editor.selectionStart;
             const end = editor.selectionEnd;
             if (start === end && start > 0) {
                 editor.value = editor.value.substring(0, start - 1) + editor.value.substring(end);
                 editor.selectionStart = editor.selectionEnd = start - 1;
             } else if (start !== end) {
                 editor.value = editor.value.substring(0, start) + editor.value.substring(end);
                 editor.selectionStart = editor.selectionEnd = start;
             }
             return;
        }

        // Standard Engine Transformation
        const start = editor.selectionStart;
        const end = editor.selectionEnd;

        // Apply backspaces
        let newValue = editor.value.substring(0, start - backspaceCount) + editor.value.substring(end);
        let newCursorPos = start - backspaceCount;

        // Build string to insert
        let insertStr = "";
        for (let i = 0; i < insertCount; i++) {
            const charCode = Module.HEAPU32[(charsStartOffset >> 2) + i];
            insertStr += String.fromCodePoint(charCode);
        }

        // Insert new string
        newValue = newValue.substring(0, newCursorPos) + insertStr + newValue.substring(newCursorPos);
        newCursorPos += insertStr.length;

        // Update DOM
        editor.value = newValue;
        editor.selectionStart = editor.selectionEnd = newCursorPos;
    });
});
