#include "lotus_core/capi.h"
#include "lotus_core/log.h"
#include <string>

extern "C" {

/**
 * JS-friendly wrapper to avoid passing/returning structs by value.
 * @param result_ptr Pointer to a pre-allocated lotus_result_t struct (132 bytes)
 */
void lotus_core_process_key_js(lotus_core_t* engine, uint32_t key, bool shift, bool caps_lock, lotus_result_t* result_ptr) {
    if (!engine || !result_ptr) return;
    
    // Log the incoming values to verify integrity
    lotus_core::emit_log(lotus_core::LogLevel::DEBUG, 
        std::string("[WASM CAPI] Received key: ") + std::to_string(key) + 
        " shift: " + std::to_string(shift) + 
        " caps_lock: " + std::to_string(caps_lock));

    lotus_modifiers_t mods;
    mods.shift = shift;
    mods.caps_lock = caps_lock;
    *result_ptr = lotus_core_process_key(engine, key, mods);
}

}
