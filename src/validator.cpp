/**
 * @file validator.cpp
 * @brief Vietnamese linguistic validation logic.
 * 
 * Implements orthographic rules and syllable structure validation for the Vietnamese language.
 */

#include "lotus_engine/validator.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/constants.h"

#include <algorithm>

namespace lotus_engine {

using namespace lotus_engine::constants;

// ============================================================================
// [ Validator Implementation ]
// ============================================================================

/**
 * @brief Checks if a given initial consonant sequence is valid in Vietnamese.
 * @param initial The UTF-32 character sequence to validate.
 * @return True if the sequence is a valid initial consonant.
 */
bool Validator::is_valid_initial(std::u32string_view initial) {
    return std::find(VALID_INITIALS_U32.begin(), VALID_INITIALS_U32.end(), initial) != VALID_INITIALS_U32.end();
}

/**
 * @brief Finds the longest valid initial consonant at the specified position in a string.
 * @param input The full input string to search.
 * @param start_pos The starting index for the search.
 * @return The length (in code points) of the longest valid initial found.
 */
size_t Validator::find_longest_initial(const std::u32string& input, size_t start_pos) {
    size_t n = input.size();
    for (size_t len = 3; len >= 1; --len) {
        if (start_pos + len <= n) {
            std::u32string lower_prefix;
            for (auto cp : input.substr(start_pos, len)) lower_prefix += unicode::to_lower(cp);
            if (is_valid_initial(lower_prefix)) return len;
        }
    }
    return 0;
}

/**
 * @brief Comprehensive validation of a Syllable structure against Vietnamese linguistic rules.
 * @param syllable The syllable instance to validate.
 * @return True if the syllable is phonotactically and orthographically valid.
 */
bool Validator::is_valid(const Syllable& syllable) {
    if (syllable.vowel.empty() && !syllable.glide.has_value()) {
        if (syllable.initial.empty()) return false;
        std::u32string lower_i;
        for (char32_t cp : syllable.initial) lower_i += unicode::to_lower(cp);
        return is_valid_initial(lower_i);
    }

    // 1. Component Set Checks
    std::u32string lower_init = unicode::to_lower(syllable.initial);
    if (!lower_init.empty()) {
        if (!is_valid_initial(lower_init)) return false;
    }

    if (syllable.glide.has_value()) {
        char32_t lower_g = unicode::to_lower(syllable.glide.value());
        if (std::find(VALID_GLIDES_U32.begin(), VALID_GLIDES_U32.end(), std::u32string_view(&lower_g, 1)) ==
            VALID_GLIDES_U32.end())
            return false;
    }

    std::u32string stripped_nucleus;
    if (!syllable.vowel.empty()) {
        for (char32_t cp : syllable.vowel) {
            char32_t toned_cp = unicode::strip_tone(unicode::to_lower(cp));
            if (toned_cp >= 0x0300 && toned_cp <= 0x036F) continue;
            stripped_nucleus += toned_cp;
        }
        if (std::find(VALID_NUCLEI_U32.begin(), VALID_NUCLEI_U32.end(), stripped_nucleus) == VALID_NUCLEI_U32.end())
            return false;
    }

    if (!syllable.final_c.empty()) {
        std::u32string lower_f;
        for (char32_t cp : syllable.final_c) lower_f += unicode::to_lower(cp);
        if (std::find(VALID_FINALS_U32.begin(), VALID_FINALS_U32.end(), lower_f) == VALID_FINALS_U32.end()) return false;
    }

    // 2. Orthographic Rules

    // Q Rule: q always followed by glide u
    if (lower_init == U"q") {
        if (!syllable.glide.has_value() || unicode::to_lower(syllable.glide.value()) != 'u') return false;
    }

    char32_t nucleus_start = syllable.vowel.empty() ? 0 : unicode::to_lower(syllable.vowel[0]);
    char32_t affinity_char = syllable.glide.has_value() ? unicode::to_lower(syllable.glide.value()) : nucleus_start;

    if (!check_front_vowel_affinity(lower_init, affinity_char, nucleus_start, syllable.final_c)) return false;
    if (!check_coda_restrictions(nucleus_start, syllable.final_c)) return false;
    if (!check_diphthong_rules(stripped_nucleus, syllable.final_c)) return false;

    return true;
}

/**
 * @brief Checks for co-occurrence rules between initial consonants and vowels.
 * 
 * Implements rules like:
 * - 'k' only before front vowels (e, ê, i, y).
 * - 'c' only before back vowels.
 * - 'gh', 'ngh' only before front vowels (excluding 'y').
 * - '-ng' coda cannot follow 'e' or 'ê' (should use '-nh').
 */
bool Validator::check_front_vowel_affinity(std::u32string_view lower_init, char32_t affinity_char,
                                           char32_t nucleus_start, std::u32string_view final_c) {
    bool is_front = (affinity_char == 'e' || affinity_char == U'ê' || affinity_char == 'i' || affinity_char == 'y');
    bool is_front_no_y = (affinity_char == 'e' || affinity_char == U'ê' || affinity_char == 'i');

    if (lower_init == U"k" && !is_front) return false;
    if (lower_init == U"c" && is_front) return false;
    if (lower_init == U"gh" && !is_front_no_y) return false;
    if (lower_init == U"g" && is_front_no_y) return false;
    if (lower_init == U"ngh" && !is_front_no_y) return false;
    if (lower_init == U"ng" && is_front_no_y) return false;

    if (final_c == U"ng") {
        if (nucleus_start == 'e' || nucleus_start == U'ê') return false;
    }
    return true;
}

/**
 * @brief Checks restrictions on specific final consonants.
 * 
 * - 'ch' and 'nh' can only follow specific nuclei (a, ê, i, y).
 */
bool Validator::check_coda_restrictions(char32_t nucleus_start, std::u32string_view final_c) {
    if (final_c == U"ch" || final_c == U"nh") {
        if (nucleus_start != 'a' && nucleus_start != U'ê' && nucleus_start != 'i' && nucleus_start != 'y')
            return false;
    }
    return true;
}

/**
 * @brief Validates centering diphthong requirements.
 * 
 * - 'iê', 'uô', 'ươ', 'yê' MUST have a coda (terminal consonant).
 * - 'ia', 'ua', 'ưa' MUST NOT have a coda.
 */
bool Validator::check_diphthong_rules(std::u32string_view stripped_nucleus, std::u32string_view final_c) {
    if (stripped_nucleus == U"iê" || stripped_nucleus == U"uô" || stripped_nucleus == U"ươ" ||
        stripped_nucleus == U"yê") {
        if (final_c.empty()) return false;
    }
    if (stripped_nucleus == U"ia" || stripped_nucleus == U"ua" || stripped_nucleus == U"ưa") {
        if (!final_c.empty()) return false;
    }
    return true;
}

} // namespace lotus_engine
