#include "lotus_core/parser_components.h"
#include "lotus_core/constants.h"
#include "lotus_core/unicode.h"
#include "lotus_core/phonology_data.h"

#include <algorithm>

namespace lotus_core {

/**
 * @brief Identifies and extracts the glide ('o' or 'u' following an initial).
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the glide (0 or 1).
 */
size_t GlideParser::parse(const std::u32string& input, size_t pos, Syllable& s) {
    size_t n = input.size();
    if (pos >= n)
        return 0;

    bool has_glide = false;
    char32_t raw_char = input[pos];
    char32_t first_char = unicode::to_lower(raw_char);

    if (pos + 1 < n) {
        char32_t next_char = unicode::strip_tone(unicode::to_lower(input[pos + 1]));
        std::u32string lower_init = unicode::to_lower(s.initial);
        
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
        s.glide = unicode::strip_tone(raw_char);
        return 1;
    }

    return 0;
}

} // namespace lotus_core
