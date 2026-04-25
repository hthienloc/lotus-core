#include "lotus_core/capi.h"
#include "lotus_core/engine.h"
#include "lotus_core/log.h"

using namespace lotus_core;

struct lotus_core_t {
    Engine core;
};

static lotus_log_callback_t g_capi_callback = nullptr;

/**
 * @brief Bridge between internal C++ logging and public C callback.
 */
static void capi_log_bridge(LogLevel level, const std::string& stage, double time_us, const std::string& message) {
    if (g_capi_callback) {
        g_capi_callback(static_cast<lotus_log_level_t>(level), stage.c_str(), time_us, message.c_str());
    }
}

lotus_core_t* lotus_core_create() {
    return new lotus_core_t();
}

void lotus_core_destroy(lotus_core_t* engine) {
    delete engine;
}

lotus_result_t lotus_core_process_key(lotus_core_t* engine, uint32_t key, lotus_modifiers_t mods) {
    if (!engine) return {0, 0, 0, {0}};
    
    Modifiers m = {mods.shift, mods.caps_lock};
    EngineResult res = engine->core.process_key(static_cast<char32_t>(key), m);
    
    lotus_result_t r;
    r.action = static_cast<uint8_t>(res.action);
    r.backspace = res.backspace;
    r.count = res.count;
    for (size_t i = 0; i < r.count && i < 32; ++i) {
        r.chars[i] = res.chars[i];
    }
    return r;
}

void lotus_core_reset(lotus_core_t* engine) {
    if (engine) engine->core.reset();
}

void lotus_core_set_method(lotus_core_t* engine, lotus_method_t method) {
    if (engine) engine->core.set_method(static_cast<InputMethod>(method));
}

void lotus_core_set_tone_style(lotus_core_t* engine, lotus_tone_style_t style) {
    if (engine) engine->core.set_tone_style(static_cast<ToneStyle>(style));
}

void lotus_core_set_free_w(lotus_core_t* engine, lotus_free_w_t option) {
    if (engine) engine->core.set_free_w(static_cast<FreeWOption>(option));
}

void lotus_core_set_std_uo(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_std_uo(enabled);
}

void lotus_core_add_shortcut(lotus_core_t* engine, const char* trigger, const char* replacement) {
    if (engine) engine->core.add_shortcut(trigger, replacement);
}

void lotus_core_set_log_callback(lotus_log_callback_t callback) {
    g_capi_callback = callback;
    if (callback) {
        set_log_callback(capi_log_bridge);
    } else {
        set_log_callback(nullptr);
    }
}

void lotus_core_set_auto_restore(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_auto_restore(enabled);
}

void lotus_core_set_allow_non_standard_initials(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_allow_non_standard_initials(enabled);
}

void lotus_core_set_double_space_to_period(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_double_space_to_period(enabled);
}

void lotus_core_set_auto_capitalize(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_auto_capitalize(enabled);
}

void lotus_core_set_macro_mode(lotus_core_t* engine, lotus_macro_mode_t mode) {
    if (engine) engine->core.set_macro_mode(static_cast<MacroMode>(mode));
}

void lotus_core_set_backspace_style(lotus_core_t* engine, lotus_backspace_style_t style) {
    if (engine) engine->core.set_backspace_style(static_cast<BackspaceStyle>(style));
}

void lotus_core_export_tracing(const char* filepath) {
    if (filepath) {
        export_tracing(filepath);
    }
}
