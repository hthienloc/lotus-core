#include "lotus_core/capi.h"
#include "lotus_core/engine.h"
#include "lotus_core/log.h"
#include "lotus_core/charset.h"
#include "lotus_core/unicode.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace lotus_core;

struct lotus_core_t {
    Engine core;
};

struct lotus_dict_t {
    std::unordered_set<std::string> words;
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
    if (!engine) return {0, 0, 0, {0}, 0};
    
    Modifiers m = {mods.shift, mods.caps_lock};
    EngineResult res = engine->core.process_key(static_cast<char32_t>(key), m);
    
    lotus_result_t r;
    r.action = static_cast<uint8_t>(res.action);
    r.backspace = res.backspace;
    r.count = res.count;
    for (size_t i = 0; i < r.count && i < 128; ++i) {
        r.chars[i] = res.chars[i];
    }
    r.diagnostic = static_cast<uint8_t>(res.diagnostic);
    return r;
}

lotus_result_t lotus_core_process_string(lotus_core_t* engine, const char* str) {
    if (!engine || !str) return {0, 0, 0, {0}, 0};
    
    EngineResult res = engine->core.process_string(str);
    
    lotus_result_t r;
    r.action = static_cast<uint8_t>(res.action);
    r.backspace = res.backspace;
    r.count = res.count;
    for (size_t i = 0; i < r.count && i < 128; ++i) {
        r.chars[i] = res.chars[i];
    }
    r.diagnostic = static_cast<uint8_t>(res.diagnostic);
    return r;
}

lotus_result_t lotus_core_reclaim_last_word(lotus_core_t* engine) {
    if (!engine) return {0, 0, 0, {0}, 0};
    
    EngineResult res = engine->core.reclaim_last_word();
    
    lotus_result_t r;
    r.action = static_cast<uint8_t>(res.action);
    r.backspace = res.backspace;
    r.count = res.count;
    for (size_t i = 0; i < r.count && i < 128; ++i) {
        r.chars[i] = res.chars[i];
    }
    r.diagnostic = static_cast<uint8_t>(res.diagnostic);
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

void lotus_core_clear_shortcuts(lotus_core_t* engine) {
    if (engine) engine->core.clear_shortcuts();
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

void lotus_core_set_tone_less(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_tone_less(enabled);
}

void lotus_core_set_mark_less(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_mark_less(enabled);
}

void lotus_core_export_tracing(const char* filepath) {
    if (filepath) {
        export_tracing(filepath);
    }
}

/**
 * @brief Create a dictionary from a newline-separated word list file.
 */
lotus_dict_t* lotus_dict_create(const char* filepath) {
    if (!filepath) return nullptr;

    std::ifstream file(filepath);
    if (!file.is_open()) return nullptr;

    auto* dict = new lotus_dict_t();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        // Convert to lowercase
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        dict->words.insert(line);
    }
    return dict;
}

void lotus_dict_destroy(lotus_dict_t* dict) {
    delete dict;
}

bool lotus_dict_contains(lotus_dict_t* dict, const char* word) {
    if (!dict || !word) return false;
    std::string w = word;
    std::transform(w.begin(), w.end(), w.begin(), ::tolower);
    return dict->words.find(w) != dict->words.end();
}

void lotus_core_set_dictionary(lotus_core_t* engine, lotus_dict_t* dict) {
    if (engine && dict) {
        engine->core.set_dictionary(std::unordered_set<std::string>(dict->words));
    }
}

void lotus_core_set_spell_check(lotus_core_t* engine, bool enabled) {
    if (engine) engine->core.set_spell_check(enabled);
}

void lotus_core_set_charset(lotus_core_t* engine, lotus_charset_t charset) {
    if (engine) engine->core.set_output_charset(static_cast<OutputCharset>(charset));
}

size_t lotus_charset_encode(lotus_charset_t charset, const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) return 0;
    std::string encoded = charset_encode(static_cast<OutputCharset>(charset), input);
    size_t len = std::min(encoded.size(), output_size - 1);
    std::memcpy(output, encoded.c_str(), len);
    output[len] = '\0';
    return len;
}

int lotus_core_get_input_method_count() {
    return 5;
}

const char* lotus_core_get_input_method_name(int index) {
    static const char* names[] = {"Telex", "VNI", "Telex + VNI", "VIQR", "Telex + VNI + VIQR"};
    if (index >= 0 && index < 5) return names[index];
    return "Unknown";
}

int lotus_core_get_charset_count() {
    return charset_get_count();
}

const char* lotus_core_get_charset_name(int index) {
    return charset_get_name(index);
}

void lotus_core_rebuild_from_text(lotus_core_t* engine, const char* text) {
    if (engine && text) {
        engine->core.rebuild_from_text(text);
    }
}

size_t lotus_core_encode_result(lotus_core_t* engine, const lotus_result_t* result, char* output, size_t output_size) {
    if (!engine || !result || !output || output_size == 0) return 0;

    std::u32string u32;
    for (uint8_t i = 0; i < result->count && i < 128; ++i) {
        if (result->chars[i] == 0) break;
        u32 += result->chars[i];
    }

    std::string utf8 = unicode::to_utf8(u32);
    OutputCharset cs = engine->core.get_output_charset();
    std::string encoded = charset_encode(cs, utf8);

    size_t len = std::min(encoded.size(), output_size - 1);
    std::memcpy(output, encoded.c_str(), len);
    output[len] = '\0';
    return len;
}
