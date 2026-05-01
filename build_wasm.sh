#!/usr/bin/env bash
# Build Lotus Core as a WebAssembly module using Emscripten.

if ! command -v emcc &> /dev/null; then
    echo "Error: emcc not found. Please install and activate Emscripten (emsdk)."
    return 1 2>/dev/null || true
fi

echo "Building WASM module..."

mkdir -p web

emcc src/core/*.cpp \
     src/core/parser/*.cpp \
     src/features/*.cpp \
     src/linguistics/*.cpp \
     src/api/*.cpp \
     -Iinclude \
     -std=c++20 \
     -O3 \
     -s EXPORTED_FUNCTIONS="['_lotus_core_create', '_lotus_core_destroy', '_lotus_core_process_key', '_lotus_core_reset', '_lotus_core_set_method', '_lotus_core_set_tone_style', '_lotus_core_set_free_w', '_lotus_core_set_std_uo', '_lotus_core_add_shortcut', '_lotus_core_set_log_callback', '_lotus_core_set_auto_restore', '_lotus_core_set_allow_non_standard_initials', '_lotus_core_set_double_space_to_period', '_lotus_core_set_auto_capitalize', '_lotus_core_set_macro_mode', '_lotus_core_set_backspace_style', '_lotus_core_reclaim_last_word', '_lotus_core_process_string', '_lotus_core_set_tone_less', '_lotus_core_set_mark_less', '_lotus_core_clear_shortcuts', '_malloc', '_free']" \
     -s EXPORTED_RUNTIME_METHODS="['HEAPU8', 'HEAPU32', 'getValue', 'setValue', 'ccall', 'cwrap', 'addFunction', 'UTF8ToString', 'allocateUTF8']" \
     -s ALLOW_TABLE_GROWTH=1 \
     -o web/lotus_core.js

echo "Build successful! Output placed in web/ directory."
