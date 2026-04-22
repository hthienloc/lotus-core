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
        if (std::find(VALID_GLIDES_U32.begin(), VALID_GLIDES_U32.end(), std::u32string_view(&lower_g, 1)) == VALID_GLIDES_U32.end())
            return false;
    }

    if (!syllable.vowel.empty()) {
        std::u32string stripped_v;
        for (char32_t cp : syllable.vowel) {
            char32_t toned_cp = unicode::strip_tone(unicode::to_lower(cp));
            if (toned_cp >= 0x0300 && toned_cp <= 0x036F) continue;
            stripped_v += toned_cp;
        }
        if (std::find(VALID_NUCLEI_U32.begin(), VALID_NUCLEI_U32.end(), stripped_v) == VALID_NUCLEI_U32.end())
            return false;
    }

    if (!syllable.final_c.empty()) {
        std::u32string lower_f;
        for (char32_t cp : syllable.final_c) lower_f += unicode::to_lower(cp);
        if (std::find(VALID_FINALS_U32.begin(), VALID_FINALS_U32.end(), lower_f) == VALID_FINALS_U32.end())
            return false;
    }

    // 2. Orthographic Rules

    // Q Rule: q always followed by glide u
    if (lower_init == U"q") {
        if (!syllable.glide.has_value() || unicode::to_lower(syllable.glide.value()) != 'u')
            return false;
    }

    // Front Vowel Affinity (k, gh, ngh)
    char32_t nucleus_start = syllable.vowel.empty() ? 0 : unicode::to_lower(syllable.vowel[0]);
    // Affinity char is the char IMMEDIATELY following the initial consonant
    char32_t affinity_char = syllable.glide.has_value() ? unicode::to_lower(syllable.glide.value()) : nucleus_start;

    bool is_front = (affinity_char == 'e' || affinity_char == U'ê' || affinity_char == 'i' || affinity_char == 'y');
    bool is_front_no_y = (affinity_char == 'e' || affinity_char == U'ê' || affinity_char == 'i');

    if (lower_init == U"k" && !is_front) return false;
    if (lower_init == U"c" && is_front) return false;
    if (lower_init == U"gh" && !is_front_no_y) return false;
    if (lower_init == U"g" && is_front_no_y) return false;
    if (lower_init == U"ngh" && !is_front_no_y) return false;
    if (lower_init == U"ng" && is_front_no_y) return false;

    if (syllable.final_c == U"ng") {
        // as per test expectations: should use -nh/ch for front vowels e/ê
        if (nucleus_start == 'e' || nucleus_start == U'ê') return false;
    }

    // Final Coda Restrictions (ch, nh) - Follow the NUCLEUS
    if (syllable.final_c == U"ch" || syllable.final_c == U"nh") {
        if (nucleus_start != 'a' && nucleus_start != U'ê' && nucleus_start != 'i' && nucleus_start != 'y')
            return false;
    }

    // Centering Diphthongs terminal requirement
    std::u32string stripped_nucleus;
    for (char32_t cp : syllable.vowel) {
        char32_t toned_cp = unicode::strip_tone(unicode::to_lower(cp));
        if (toned_cp >= 0x0300 && toned_cp <= 0x036F) continue;
        stripped_nucleus += toned_cp;
    }
    
    // Diphthong vs Open/Closed Syllable (ia/iê, ua/uô, ưa/ươ)
    if (stripped_nucleus == U"iê" || stripped_nucleus == U"uô" || stripped_nucleus == U"ươ" || stripped_nucleus == U"yê") {
        // Centering diphthongs MUST have a coda (e.g., tiên, đuôi)
        if (syllable.final_c.empty()) return false;
    }
    if (stripped_nucleus == U"ia" || stripped_nucleus == U"ua" || stripped_nucleus == U"ưa") {
        // Open diphthongs MUST NOT have a coda (e.g., mía, mùa, mưa)
        if (!syllable.final_c.empty()) return false;
    }

    return true;
}

} // namespace lotus_engine
