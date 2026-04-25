#include "lotus_core/parser_components.h"
#include "lotus_core/parser.h"
#include "lotus_core/unicode.h"
#include "lotus_core/validator.h"
#include "lotus_core/phonology_data.h"

namespace lotus_core {

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

    // Apply context-sensitive overrides (e.g. 'gi' vs 'g', 'qu' vs 'q')
    for (const auto& rule : phonology::INITIAL_OVERRIDE_RULES) {
        if (lower_init == rule.initial) {
            bool condition_met = false;
            if (rule.requires_no_vowel_at_index) {
                if (input.size() <= rule.check_index || !SyllableParser::is_vowel(input[rule.check_index])) {
                    condition_met = true;
                }
            } else {
                condition_met = true;
            }

            if (condition_met) {
                s.initial = s.initial.substr(0, rule.new_len);
                initial_len = rule.new_len;
            }
            break;
        }
    }

    return initial_len;
}

} // namespace lotus_core
