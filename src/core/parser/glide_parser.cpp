#include "lotus_engine/parser_components.h"
#include "lotus_engine/constants.h"
#include "lotus_engine/unicode.h"

#include <algorithm>

namespace lotus_engine {

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
    char32_t first_char = unicode::to_lower(input[pos]);

    if (pos + 1 < n) {
        char32_t next_char = unicode::strip_tone(unicode::to_lower(input[pos + 1]));
        if (first_char == 'o') {
            if (std::find(constants::GLIDE_O_NEXT.begin(), constants::GLIDE_O_NEXT.end(), next_char) != constants::GLIDE_O_NEXT.end())
                has_glide = true;
        } else if (first_char == 'u') {
            std::u32string lower_init = unicode::to_lower(s.initial);
            bool is_qu = (lower_init == U"q");
            if (is_qu) {
                if (std::find(constants::GLIDE_U_NEXT_QU.begin(), constants::GLIDE_U_NEXT_QU.end(), next_char) != constants::GLIDE_U_NEXT_QU.end())
                    has_glide = true;
            } else {
                if (std::find(constants::GLIDE_U_NEXT.begin(), constants::GLIDE_U_NEXT.end(), next_char) != constants::GLIDE_U_NEXT.end())
                    has_glide = true;
            }
        }
    }

    if (has_glide) {
        s.glide = first_char;
        return 1;
    }

    return 0;
}

} // namespace lotus_engine
