#include "lotus_engine/capi.h"

#include "lotus_engine/engine.h"
#include "lotus_engine/log.h"

#include <cstring>

using namespace lotus_engine;

struct lotus_engine_t {
    Engine core;
};

extern "C" {

lotus_engine_t* lotus_engine_create() {
    return new lotus_engine_t();
}

void lotus_engine_destroy(lotus_engine_t* engine) {
    delete engine;
}

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

void lotus_engine_reset(lotus_engine_t* engine) {
    if (engine)
        engine->core.reset();
}

void lotus_engine_set_method(lotus_engine_t* engine, lotus_method_t method) {
    if (!engine)
        return;
    InputMethod m = (method == LOTUS_METHOD_VNI) ? InputMethod::VNI : InputMethod::TELEX;
    engine->core.set_method(m);
}

void lotus_engine_set_tone_style(lotus_engine_t* engine, lotus_tone_style_t style) {
    if (!engine)
        return;
    ToneStyle s = (style == LOTUS_TONE_OLD) ? ToneStyle::OLD : ToneStyle::NEW;
    engine->core.set_tone_style(s);
}

void lotus_engine_add_shortcut(lotus_engine_t* engine, const char* trigger,
                               const char* replacement) {
    if (!engine || !trigger || !replacement)
        return;
    engine->core.add_shortcut(trigger, replacement);
}

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
