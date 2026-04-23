#include "lotus_engine/shortcut_manager.h"
#include "lotus_engine/unicode.h"

#include <algorithm>
#include <cctype>

namespace lotus_engine {

void ShortcutManager::add_shortcut(const std::string& trigger, const std::string& replacement) {
    shortcuts[unicode::to_lower(trigger)] = replacement;
    exact_shortcuts[trigger] = replacement;
}

void ShortcutManager::clear() {
    shortcuts.clear();
    exact_shortcuts.clear();
}

bool ShortcutManager::is_trigger_key(char32_t key) const {
    return key == ' ' || key == '\t' || key == '\n' || key == '\r' ||
           key == ',' || key == '.' || key == '!' || key == '?' ||
           key == ':' || key == ';';
}

bool ShortcutManager::handle(char32_t key, const std::u32string& buffer, EngineResult& result, MacroMode mode) {
    if (mode == MacroMode::OFF || buffer.empty() || !is_trigger_key(key))
        return false;
        
    std::string trigger_raw = unicode::to_utf8(buffer);
    std::string trigger_lower = unicode::to_lower(trigger_raw);
    
    std::string replacement;
    
    if (mode == MacroMode::EXACT) {
        if (exact_shortcuts.count(trigger_raw)) {
            replacement = exact_shortcuts[trigger_raw];
        } else {
            return false;
        }
    } else if (mode == MacroMode::FIXED) {
        if (shortcuts.count(trigger_lower)) {
            replacement = shortcuts[trigger_lower]; // return exact registered replacement regardless of typed case
        } else {
            return false;
        }
    } else if (mode == MacroMode::ADAPTIVE) {
        if (shortcuts.count(trigger_lower)) {
            replacement = shortcuts[trigger_lower];
            bool is_all_upper = std::all_of(trigger_raw.begin(), trigger_raw.end(),
                                            [](unsigned char c) { return !isalpha(c) || isupper(c); });
            bool is_first_upper = isupper((unsigned char)trigger_raw[0]);
            
            if (is_all_upper) {
                std::u32string temp_u32 = unicode::to_utf32(replacement);
                for (auto& c : temp_u32)
                    c = unicode::to_upper(c);
                replacement = unicode::to_utf8(temp_u32);
            } else if (is_first_upper) {
                std::u32string temp_u32 = unicode::to_utf32(replacement);
                bool new_word = true;
                for (auto& c : temp_u32) {
                    if (new_word && unicode::is_alpha(c)) {
                        c = unicode::to_upper(c);
                        new_word = false;
                    } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                        new_word = true;
                    }
                }
                replacement = unicode::to_utf8(temp_u32);
            } else {
                std::u32string temp_u32 = unicode::to_utf32(replacement);
                for (auto& c : temp_u32)
                    c = unicode::to_lower(c);
                replacement = unicode::to_utf8(temp_u32);
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    
    std::u32string repl_u32 = unicode::to_utf32(replacement);
    repl_u32.push_back(key);
    result = _make_transformation_result(repl_u32, buffer.size());
    return true;
}

EngineResult ShortcutManager::_make_transformation_result(const std::u32string& final_u32, size_t prev_size) const {
    EngineResult result{};
    result.action = 1;
    result.backspace = (uint8_t)prev_size;
    result.count = (uint8_t)std::min((size_t)32, final_u32.size());
    for (int i = 0; i < result.count; i++)
        result.chars[i] = final_u32[i];
    return result;
}

} // namespace lotus_engine
