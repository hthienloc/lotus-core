#include "lotus_core/parser_components.h"
#include "lotus_core/constants.h"
#include "lotus_core/unicode.h"
#include "lotus_core/phonology_data.h"

#include <algorithm>

using namespace lotus_core;

namespace lotus_core {

/**
 * @brief Identifies and extracts the glide ('o' or 'u' following an initial).
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param initial The already parsed initial.
 * @return GlideParseResult The parsed glide and the number of characters consumed.
 */
GlideParseResult GlideParser::parse(std::u32string_view input, size_t pos, std::u32string_view initial) {
    GlideParseResult result;
    size_t n = input.size();
    if (pos >= n)
        return result;

    bool has_glide = false;
    char32_t raw_char = input[pos];
    char32_t first_char = unicode::to_lower(raw_char);

    if (pos + 1 < n) {
        char32_t next_char = unicode::strip_tone(unicode::to_lower(input[pos + 1]));
        StaticString lower_init_str;
        for (char32_t c : initial) {
            lower_init_str += unicode::to_lower(c);
        }
        std::u32string_view lower_init = lower_init_str.view();
        
        for (const auto& rule : phonology::GLIDE_RULES) {
            if (rule.glide_char == first_char) {
                if (rule.initial_context.empty() || lower_init == rule.initial_context) {
                    if (rule.valid_next_chars.find(next_char) != std::u32string_view::npos) {
                        has_glide = true;
                        break;
                    }
                }
            }
        }
    }

    if (has_glide) {
        result.glide = unicode::strip_tone(raw_char);
        result.consumed = 1;
    }

    return result;
}

} // namespace lotus_core
