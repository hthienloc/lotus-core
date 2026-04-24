#include "lotus_engine/capi.h"

extern "C" {

/**
 * JS-friendly wrapper to avoid passing/returning structs by value.
 * @param result_ptr Pointer to a pre-allocated lotus_result_t struct (132 bytes)
 */
void lotus_engine_process_key_js(lotus_engine_t* engine, uint32_t key, bool shift, bool caps_lock, lotus_result_t* result_ptr) {
    if (!engine || !result_ptr) return;
    lotus_modifiers_t mods;
    mods.shift = shift;
    mods.caps_lock = caps_lock;
    *result_ptr = lotus_engine_process_key(engine, key, mods);
}

}
