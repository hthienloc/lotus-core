/**
 * @file capi.cpp
 * @brief C-compatible API wrapper for the Lotus Engine.
 *
 * Provides an 'extern "C"' interface to allow integration with non-C++
 * languages and applications.
 */

#include "lotus_engine/capi.h"

#include "lotus_engine/engine.h"
#include "lotus_engine/log.h"

#include <cstring>

using namespace lotus_engine;

/**
 * @struct lotus_engine_t
 * @brief Opaque handle to the internal Engine instance.
 */
struct lotus_engine_t {
    Engine core;  ///< The underlying C++ Engine instance.
};

// ============================================================================
// [ C API Implementation ]
// ============================================================================

extern "C" {

/**
 * @brief Creates a new engine instance.
 * @return A pointer to the newly created engine handle.
 */
lotus_engine_t* lotus_engine_create() {
    return new lotus_engine_t();
}

/**
 * @brief Destroys an engine instance and frees its memory.
 * @param engine The handle to destroy.
 */
void lotus_engine_destroy(lotus_engine_t* engine) {
    delete engine;
}

/**
 * @brief Processes a key press via the C API.
 * @param engine The engine handle.
 * @param key The UTF-32 key code.
 * @param mods Keyboard modifiers.
 * @return A C-compatible result structure.
 */
lotus_result_t lotus_engine_process_key(lotus_engine_t* engine, uint32_t key,
                                        lotus_modifiers_t mods) {
    if (!engine)
        return {};

    Modifiers core_mods;
    core_mods.shift = mods.shift;
    core_mods.caps_lock = mods.caps_lock;

    EngineResult res = engine->core.process_key((char32_t)key, core_mods);

    lotus_result_t out{};
    out.action = res.action;
    out.backspace = res.backspace;
    out.count = res.count;
    for (int i = 0; i < res.count; i++) {
        out.chars[i] = (uint32_t)res.chars[i];
    }

    return out;
}

/**
 * @brief Resets the engine state.
 * @param engine The engine handle.
 */
void lotus_engine_reset(lotus_engine_t* engine) {
    if (engine)
        engine->core.reset();
}

/**
 * @brief Sets the input method (Telex or VNI).
 * @param engine The engine handle.
 * @param method The desired input method.
 */
void lotus_engine_set_method(lotus_engine_t* engine, lotus_method_t method) {
    if (!engine)
        return;
    InputMethod m = (method == LOTUS_METHOD_VNI) ? InputMethod::VNI : InputMethod::TELEX;
    engine->core.set_method(m);
}

/**
 * @brief Configures the tone placement style.
 * @param engine The engine handle.
 * @param style The desired tone style (Modern/Old).
 */
void lotus_engine_set_tone_style(lotus_engine_t* engine, lotus_tone_style_t style) {
    if (!engine)
        return;
    ToneStyle s = (style == LOTUS_TONE_OLD) ? ToneStyle::OLD : ToneStyle::NEW;
    engine->core.set_tone_style(s);
}

/**
 * @brief Configures the standalone 'w' key behavior.
 * @param engine The engine handle.
 * @param option The desired behavior option.
 */
void lotus_engine_set_free_w(lotus_engine_t* engine, lotus_free_w_t option) {
    if (!engine)
        return;
    FreeWOption o;
    if (option == LOTUS_FREE_W_OFF)
        o = FreeWOption::OFF;
    else if (option == LOTUS_FREE_W_NON_START)
        o = FreeWOption::NON_START;
    else
        o = FreeWOption::ALWAYS;
    engine->core.set_free_w(o);
}

/**
 * @brief Enables or disables standard manual hook keys ([, ]).
 * @param engine The engine handle.
 * @param enabled True to enable.
 */
void lotus_engine_set_std_uo(lotus_engine_t* engine, bool enabled) {
    if (engine)
        engine->core.set_std_uo(enabled);
}

/**
 * @brief Configures English auto-restoration behavior.
 * @param engine The engine handle.
 * @param enabled True to enable.
 */
void lotus_engine_set_auto_restore(lotus_engine_t* engine, bool enabled) {
    if (engine)
        engine->core.set_auto_restore(enabled);
}

void lotus_engine_set_allow_non_standard_initials(lotus_engine_t* engine, bool enabled) {
    if (engine)
        engine->core.set_allow_non_standard_initials(enabled);
}

/**
 * @brief Adds a text expansion shortcut.
 * @param engine The engine handle.
 * @param trigger UTF-8 trigger string.
 * @param replacement UTF-8 replacement string.
 */
void lotus_engine_add_shortcut(lotus_engine_t* engine, const char* trigger,
                               const char* replacement) {
    if (!engine || !trigger || !replacement)
        return;
    engine->core.add_shortcut(trigger, replacement);
}

/**
 * @brief Registers a callback for engine log events.
 * @param callback The function to call for log messages.
 */
void lotus_engine_set_log_callback(lotus_log_callback_t callback) {
    if (!callback) {
        lotus_engine::set_log_callback(nullptr);
    } else {
        lotus_engine::set_log_callback(
            [callback](lotus_engine::LogLevel level, const std::string& message) {
                lotus_log_level_t c_level;
                if (level == lotus_engine::LogLevel::ERROR)
                    c_level = LOTUS_LOG_LEVEL_ERROR;
                else if (level == lotus_engine::LogLevel::INFO)
                    c_level = LOTUS_LOG_LEVEL_INFO;
                else
                    c_level = LOTUS_LOG_LEVEL_DEBUG;

                callback(c_level, message.c_str());
            });
    }
}

}  // extern "C"
