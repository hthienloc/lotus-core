#include "lotus_core/parser_components.h"
#include "lotus_core/parser.h"
#include "lotus_core/unicode.h"
#include "lotus_core/validator.h"
#include "lotus_core/phonology_data.h"

using namespace lotus_core;

namespace lotus_core {

/**
 * @brief Identifies and extracts the initial consonant.
 * @param input The raw input character sequence.
 * @param allow_non_standard Whether to allow z, w, j, f.
 * @return InitialParseResult The parsed initial and the number of characters consumed.
 */
InitialParseResult InitialParser::parse(std::u32string_view input, bool allow_non_standard) {
    InitialParseResult result;
    size_t initial_len = Validator::find_longest_initial(input, 0, allow_non_standard);
    if (initial_len == 0)
        return result;

    result.initial = input.substr(0, initial_len);
    StaticString lower_init_str;
    for (char32_t c : result.initial.view()) {
        lower_init_str += unicode::to_lower(c);
    }
    std::u32string_view lower_init = lower_init_str.view();

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
                result.initial = result.initial.substr(0, rule.new_len);
                initial_len = rule.new_len;
            }
            break;
        }
    }

    result.consumed = initial_len;
    return result;
}

} // namespace lotus_core
