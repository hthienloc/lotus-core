/**
 * @file validator.cpp
 * @brief Vietnamese linguistic validation logic.
 *
 * Implements orthographic rules and syllable structure validation for the Vietnamese language.
 */

#include "lotus_core/validator.h"

#include "lotus_core/constants.h"
#include "lotus_core/unicode.h"

#include <algorithm>

using namespace lotus_core;
using namespace constants;

namespace lotus_core {

// ============================================================================
// [ Validator Implementation ]
// ============================================================================

/**
 * @brief Checks if a given initial consonant sequence is valid in Vietnamese.
 * @param initial The UTF-32 character sequence to validate.
 * @param allow_non_standard If true, allows 'z', 'w', 'j', 'f' as valid initials.
 * @return True if the sequence is a valid initial consonant.
 */
bool Validator::is_valid_initial(std::u32string_view initial, bool allow_non_standard) {
    if (allow_non_standard) {
        if (std::find(NON_STANDARD_INITIALS_U32.begin(), NON_STANDARD_INITIALS_U32.end(), initial) !=
            NON_STANDARD_INITIALS_U32.end()) {
            return true;
        }
    }
    return std::find(VALID_INITIALS_U32.begin(), VALID_INITIALS_U32.end(), initial) !=
           VALID_INITIALS_U32.end();
}

/**
 * @brief Finds the longest valid initial consonant at the specified position in a string.
 * @param input The full input string to search.
 * @param start_pos The starting index for the search.
 * @param allow_non_standard If true, allows 'z', 'w', 'j', 'f'.
 * @return The length (in code points) of the longest valid initial found.
 */
size_t Validator::find_longest_initial(std::u32string_view input, size_t start_pos, bool allow_non_standard) {
    size_t n = input.size();
    for (size_t len = 3; len >= 1; --len) {
        if (start_pos + len <= n) {
            StaticString lower_prefix;
            for (auto cp : input.substr(start_pos, len))
                lower_prefix += unicode::to_lower(cp);
            if (is_valid_initial(lower_prefix.view(), allow_non_standard))
                return len;
        }
    }
    return 0;
}

/**
 * @brief Comprehensive validation of a Syllable structure against Vietnamese linguistic rules.
 * @param syllable The syllable instance to validate.
 * @param diagnostic_code Optional pointer to a DiagnosticCode that will be populated with the reason for
 * failure if the syllable is invalid.
 * @param allow_non_standard If true, allows 'z', 'w', 'j', 'f'.
 * @return True if the syllable is phonotactically and orthographically valid.
 */
bool Validator::validate_glide_compatibility(const Syllable& syllable, std::u32string_view lower_init,
                                             DiagnosticCode* diagnostic_code) {
    if (syllable.glide.has_value()) {
        char32_t lower_g = unicode::to_lower(syllable.glide.value());
        if (std::find(VALID_GLIDES_U32.begin(), VALID_GLIDES_U32.end(),
                      std::u32string_view(&lower_g, 1)) == VALID_GLIDES_U32.end()) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_GLIDE;
            return false;
        }
    }

    // Q Rule: q always followed by glide u
    if (lower_init == U"q") {
        if (!syllable.glide.has_value() || unicode::to_lower(syllable.glide.value()) != 'u') {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_GLIDE;
            return false;
        }
    }
    return true;
}

bool Validator::validate_tone_placement(const Syllable& syllable, std::u32string& stripped_nucleus,
                                        DiagnosticCode* diagnostic_code) {
    if (!syllable.vowel.empty()) {
        for (char32_t cp : syllable.vowel) {
            char32_t toned_cp = unicode::strip_tone(unicode::to_lower(cp));
            if (toned_cp >= 0x0300 && toned_cp <= 0x036F)
                continue;
            stripped_nucleus += toned_cp;
        }
        if (std::find(VALID_NUCLEI_U32.begin(), VALID_NUCLEI_U32.end(), stripped_nucleus) ==
            VALID_NUCLEI_U32.end()) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_NUCLEUS;
            return false;
        }
    }
    return true;
}

bool Validator::check_coda_compatibility(const Syllable& syllable, char32_t nucleus_start,
                                         std::u32string_view stripped_nucleus,
                                         DiagnosticCode* diagnostic_code) {
    if (!syllable.final_c.empty()) {
        static const std::vector<std::u32string_view> CLOSING_DIPHTHONGS = {
            U"ai", U"ao", U"au", U"âu", U"ay", U"ây", U"eo", U"êu", U"iu",
            U"oi", U"ôi", U"ơi", U"ui", U"ưi", U"ưu", U"iêu", U"yêu", U"uôi", U"ươi", U"ươu"
        };
        std::u32string lower_v;
        for (char32_t cp : syllable.vowel)
            lower_v += unicode::to_lower(cp);
        if (std::find(CLOSING_DIPHTHONGS.begin(), CLOSING_DIPHTHONGS.end(), lower_v) !=
            CLOSING_DIPHTHONGS.end()) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_CODA;
            return false;
        }

        std::u32string lower_f;
        for (char32_t cp : syllable.final_c.view())
            lower_f += unicode::to_lower(cp);
        if (std::find(VALID_FINALS_U32.begin(), VALID_FINALS_U32.end(), lower_f) ==
            VALID_FINALS_U32.end()) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_CODA;
            return false;
        }
    }

    if (!check_coda_restrictions(nucleus_start, syllable.final_c.view(), diagnostic_code))
        return false;
    if (!check_diphthong_rules(stripped_nucleus, syllable.final_c.view(), diagnostic_code))
        return false;

    return true;
}

bool Validator::is_valid(const Syllable& syllable, DiagnosticCode* diagnostic_code, bool allow_non_standard) {
    if (syllable.vowel.empty() && !syllable.glide.has_value()) {
        if (syllable.initial.empty()) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
            return false;
        }
        std::u32string lower_i = unicode::to_lower(syllable.initial.view());
        bool valid_init = is_valid_initial(lower_i, allow_non_standard);
        if (!valid_init && diagnostic_code) {
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        }
        return valid_init;
    }

    // 1. Component Set Checks
    std::u32string lower_init = unicode::to_lower(syllable.initial.view());
    if (!lower_init.empty()) {
        if (!is_valid_initial(lower_init, allow_non_standard)) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
            return false;
        }
    }

    if (!validate_glide_compatibility(syllable, lower_init, diagnostic_code))
        return false;

    std::u32string stripped_nucleus;
    if (!validate_tone_placement(syllable, stripped_nucleus, diagnostic_code))
        return false;

    // 2. Orthographic Rules

    char32_t nucleus_start = syllable.vowel.empty() ? 0 : unicode::to_lower(syllable.vowel[0]);
    char32_t affinity_char =
        syllable.glide.has_value() ? unicode::to_lower(syllable.glide.value()) : nucleus_start;

    if (!check_initial_vowel_affinity(lower_init, affinity_char, nucleus_start, syllable.final_c.view(),
                                      diagnostic_code))
        return false;

    if (!check_coda_compatibility(syllable, nucleus_start, stripped_nucleus, diagnostic_code))
        return false;

    return true;
}

bool Validator::is_front_vowel(char32_t c) {
    return std::find(FRONT_VOWELS.begin(), FRONT_VOWELS.end(), c) != FRONT_VOWELS.end();
}

bool Validator::is_front_vowel_strict(char32_t c) {
    return std::find(FRONT_VOWELS_STRICT.begin(), FRONT_VOWELS_STRICT.end(), c) != FRONT_VOWELS_STRICT.end();
}

bool Validator::is_e_vowel(char32_t c) {
    return std::find(E_VOWELS.begin(), E_VOWELS.end(), c) != E_VOWELS.end();
}

bool Validator::is_valid_ch_nh_nucleus(char32_t c) {
    return std::find(CH_NH_NUCLEI.begin(), CH_NH_NUCLEI.end(), c) != CH_NH_NUCLEI.end();
}

bool Validator::is_centering_diphthong_requiring_coda(std::u32string_view v) {
    return std::find(CENTERING_DIPHTHONGS_REQ_CODA.begin(), CENTERING_DIPHTHONGS_REQ_CODA.end(), v) != CENTERING_DIPHTHONGS_REQ_CODA.end();
}

bool Validator::is_centering_diphthong_forbidding_coda(std::u32string_view v) {
    return std::find(CENTERING_DIPHTHONGS_NO_CODA.begin(), CENTERING_DIPHTHONGS_NO_CODA.end(), v) != CENTERING_DIPHTHONGS_NO_CODA.end();
}

/**
 * @brief Checks for co-occurrence rules between initial consonants and vowels.
 *
 * Implements rules like:
 * - 'k' only before front vowels (e, ê, i, y).
 * - 'c' only before back vowels.
 * - 'gh', 'ngh' only before front vowels (excluding 'y').
 * - '-ng' coda cannot follow 'e' or 'ê' (should use '-nh').
 *
 * @param lower_init The initial consonant sequence in lowercase.
 * @param affinity_char The character used for affinity checking (glide if present, else nucleus
 * start).
 * @param nucleus_start The first character of the vowel nucleus.
 * @param final_c The final consonant (coda).
 * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
 * @return True if the combination is valid.
 */
bool Validator::check_initial_vowel_affinity(std::u32string_view lower_init, char32_t affinity_char,
                                             char32_t nucleus_start, std::u32string_view final_c,
                                             DiagnosticCode* diagnostic_code) {
    bool is_front = is_front_vowel(affinity_char);
    bool is_front_no_y = is_front_vowel_strict(affinity_char);

    if (lower_init == U"k" && !is_front) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        return false;
    }
    if (lower_init == U"c" && is_front) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        return false;
    }
    if (lower_init == U"gh" && !is_front_no_y) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        return false;
    }
    if (lower_init == U"g" && is_front_no_y) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        return false;
    }
    if (lower_init == U"ngh" && !is_front_no_y) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        return false;
    }
    if (lower_init == U"ng" && is_front_no_y) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_INITIAL;
        return false;
    }

    if (final_c == U"ng" && is_e_vowel(nucleus_start)) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_CODA;
        return false;
    }

    return true;
}

/**
 * @brief Checks restrictions on specific final consonants.
 *
 * - 'ch' and 'nh' can only follow specific nuclei (a, ê, i, y).
 *
 * @param nucleus_start The first character of the vowel nucleus.
 * @param final_c The final consonant (coda).
 * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
 * @return True if the coda restriction is satisfied.
 */
bool Validator::check_coda_restrictions(char32_t nucleus_start, std::u32string_view final_c,
                                        DiagnosticCode* diagnostic_code) {
    if (final_c == U"ch" || final_c == U"nh") {
        if (!is_valid_ch_nh_nucleus(nucleus_start)) {
            if (diagnostic_code)
                *diagnostic_code = DiagnosticCode::INVALID_CODA;
            return false;
        }
    }
    return true;
}

/**
 * @brief Validates centering diphthong requirements.
 *
 * - 'iê', 'uô', 'ươ', 'yê' MUST have a coda (terminal consonant).
 * - 'ia', 'ua', 'ưa' MUST NOT have a coda.
 *
 * @param stripped_nucleus The vowel nucleus without tone marks.
 * @param final_c The final consonant (coda).
 * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
 * @return True if diphthong rules are respected.
 */
bool Validator::check_diphthong_rules(std::u32string_view stripped_nucleus,
                                      std::u32string_view final_c, DiagnosticCode* diagnostic_code) {
    if (is_centering_diphthong_requiring_coda(stripped_nucleus) && final_c.empty()) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_NUCLEUS;
        return false;
    }
    if (is_centering_diphthong_forbidding_coda(stripped_nucleus) && !final_c.empty()) {
        if (diagnostic_code)
            *diagnostic_code = DiagnosticCode::INVALID_NUCLEUS;
        return false;
    }
    return true;
}

}  // namespace lotus_core
