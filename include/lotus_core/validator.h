#pragma once

#include "lotus_core/types.h"

#include <string>
#include <string_view>
#include <unordered_set>

namespace lotus_core {

/**
 * @brief Performs phonotactic validation of Vietnamese syllable components.
 *
 * Checks each component of a @ref Syllable against the Vietnamese phonological
 * inventory, including orthographic rules (e.g., 'k' before front vowels,
 * 'gh'/'ngh' restriction to non-back contexts, centering diphthong coda rules).
 */
class Validator {
   public:
    /**
     * @brief Checks whether a string is a valid Vietnamese initial consonant.
     * @param initial The initial consonant to check, in lowercase UTF-32.
     * @param allow_non_standard If true, allows 'z', 'w', 'j', 'f' as valid initials.
     * @return true if the initial is in the valid inventory.
     */
    static bool is_valid_initial(std::u32string_view initial, bool allow_non_standard = false);

    /**
     * @brief Finds the length of the longest valid initial at the given position.
     * @param input The full UTF-32 input string.
     * @param start_pos The position to start matching from.
     * @param allow_non_standard If true, allows 'z', 'w', 'j', 'f'.
     * @return The length (in codepoints) of the matched initial, or 0 if none.
     */
    static size_t find_longest_initial(std::u32string_view input, size_t start_pos, bool allow_non_standard = false);

    /**
     * @brief Performs comprehensive phonotactic validation of a syllable.
     *
     * Validates all component sets and applies orthographic co-occurrence rules.
     * @param syllable The syllable to validate.
     * @param diagnostic_code Optional pointer to a DiagnosticCode to be populated with the reason
     * for failure if the syllable is invalid.
     * @param allow_non_standard If true, allows 'z', 'w', 'j', 'f'.
     * @return true if the syllable is phonotactically valid Vietnamese.
     */
    static bool is_valid(const Syllable& syllable, DiagnosticCode* diagnostic_code = nullptr, bool allow_non_standard = false);

   private:
    /**
     * @brief Validates glide compatibility including the orthographic 'q' rule.
     * @param syllable The syllable being validated.
     * @param lower_init The lowercase initial consonant.
     * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
     */
    static bool validate_glide_compatibility(const Syllable& syllable, std::u32string_view lower_init,
                                             DiagnosticCode* diagnostic_code = nullptr);

    /**
     * @brief Validates tone placement and extracts the stripped nucleus.
     * @param syllable The syllable being validated.
     * @param stripped_nucleus Output parameter to store the nucleus without tone marks.
     * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
     */
    static bool validate_tone_placement(const Syllable& syllable, std::u32string& stripped_nucleus,
                                        DiagnosticCode* diagnostic_code = nullptr);

    /**
     * @brief Checks orthographic affinity between initial consonants and following vowels/glides.
     * @param lower_init Lowercase initial consonant.
     * @param affinity_char The character immediately following the initial (glide or nucleus
     * start).
     * @param nucleus_start The first character of the vowel nucleus.
     * @param final_c The final consonant (coda).
     * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
     */
    static bool check_initial_vowel_affinity(std::u32string_view lower_init, char32_t affinity_char,
                                             char32_t nucleus_start, std::u32string_view final_c,
                                             DiagnosticCode* diagnostic_code = nullptr);

    /**
     * @brief High-level orchestrator for checking coda compatibility, internally calling restrictions and diphthong rules.
     * @param syllable The syllable being validated.
     * @param nucleus_start The first character of the vowel nucleus.
     * @param stripped_nucleus The vowel nucleus without tone marks.
     * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
     */
    static bool check_coda_compatibility(const Syllable& syllable, char32_t nucleus_start,
                                         std::u32string_view stripped_nucleus,
                                         DiagnosticCode* diagnostic_code = nullptr);

    /**
     * @brief Checks restrictions on specific codas (nh, ch) based on the nucleus.
     * @param nucleus_start The first character of the vowel nucleus.
     * @param final_c The final consonant (coda).
     * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
     */
    static bool check_coda_restrictions(char32_t nucleus_start, std::u32string_view final_c,
                                        DiagnosticCode* diagnostic_code = nullptr);

    /**
     * @brief Checks centering diphthong rules (ia/iê, ua/uô, ưa/ươ) regarding codas.
     * @param stripped_nucleus The vowel nucleus without tone marks.
     * @param final_c The final consonant (coda).
     * @param diagnostic_code Optional pointer to a DiagnosticCode to populate on failure.
     */
    static bool check_diphthong_rules(std::u32string_view stripped_nucleus,
                                      std::u32string_view final_c,
                                      DiagnosticCode* diagnostic_code = nullptr);

    // --- Repetitive cluster helpers ---
    static bool is_front_vowel(char32_t c);
    static bool is_front_vowel_strict(char32_t c);
    static bool is_e_vowel(char32_t c);
    static bool is_valid_ch_nh_nucleus(char32_t c);
    static bool is_centering_diphthong_requiring_coda(std::u32string_view v);
    static bool is_centering_diphthong_forbidding_coda(std::u32string_view v);
};

}  // namespace lotus_core
