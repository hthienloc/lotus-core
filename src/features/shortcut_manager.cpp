#include "lotus_engine/shortcut_manager.h"
#include "lotus_engine/unicode.h"

#include <algorithm>
#include <cctype>

namespace lotus_engine {

void ShortcutManager::add_shortcut(const std::string& trigger, const std::string& replacement) {
    shortcuts[trigger] = replacement;
}

bool ShortcutManager::handle(char32_t key, const std::u32string& buffer, EngineResult& result) {
    if (buffer.empty())
        return false;
    std::string trigger_raw = unicode::to_utf8(buffer);
    std::string trigger_lower = unicode::to_lower(trigger_raw);
    if (shortcuts.count(trigger_lower)) {
        std::string replacement = shortcuts[trigger_lower];
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
            if (!temp_u32.empty())
                temp_u32[0] = unicode::to_upper(temp_u32[0]);
            replacement = unicode::to_utf8(temp_u32);
        }
        std::u32string repl_u32 = unicode::to_utf32(replacement);
        repl_u32.push_back(key);
        result = _make_transformation_result(repl_u32, buffer.size());
        return true;
    }
    return false;
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
