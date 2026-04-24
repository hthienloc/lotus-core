#include "lotus_engine/parser_components.h"
#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/validator.h"

namespace lotus_engine {

/**
 * @brief Identifies and extracts the initial consonant.
 * @param input The raw input character sequence.
 * @param s OUT: The Syllable object to populate.
 * @param allow_non_standard Whether to allow z, w, j, f.
 * @return size_t The number of characters consumed as the initial consonant.
 */
size_t InitialParser::parse(const std::u32string& input, Syllable& s, bool allow_non_standard) {
    size_t initial_len = Validator::find_longest_initial(input, 0, allow_non_standard);
    if (initial_len == 0)
        return 0;

    s.initial = input.substr(0, initial_len);
    std::u32string lower_init = unicode::to_lower(s.initial);

    // Vietnamese rule for 'gi':
    if (lower_init == U"gi") {
        if (input.size() == 2 || !SyllableParser::is_vowel(input[2])) {
            s.initial = s.initial.substr(0, 1);
            initial_len = 1;
        }
    }
    // Vietnamese rule for 'qu':
    else if (lower_init == U"qu") {
        s.initial = s.initial.substr(0, 1);
        initial_len = 1;
    }

    return initial_len;
}

} // namespace lotus_engine
