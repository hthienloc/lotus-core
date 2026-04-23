#include "lotus_engine/smart_typing.h"
#include "lotus_engine/unicode.h"

namespace lotus_engine {

bool SmartTyping::handle(char32_t& key, bool double_space_to_period, bool auto_capitalize, char32_t last_boundary_key, bool at_sentence_start, const std::u32string& buffer, EngineResult& result, std::u32string& last_committed_text) {
    if (double_space_to_period && key == ' ' && last_boundary_key == ' ') {
        result.action = 1;
        result.backspace = 1;
        result.count = 2;
        result.chars[0] = '.';
        result.chars[1] = ' ';
        last_committed_text = U". ";
        return true; // We handled the double space to period
    }

    if (auto_capitalize && at_sentence_start && key != 0 && buffer.empty()) {
        char32_t upper = unicode::to_upper(key);
        if (upper != key)
            key = upper;
    }
    return false;
}

} // namespace lotus_engine
