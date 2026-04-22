#pragma once

#include "lotus_engine/types.h"

#include <string>
#include <string_view>
#include <unordered_set>

namespace lotus_engine {

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
     * @return true if the initial is in the valid inventory.
     */
    static bool is_valid_initial(std::u32string_view initial);

    /**
     * @brief Finds the length of the longest valid initial at the given position.
     * @param input The full UTF-32 input string.
     * @param start_pos The position to start matching from.
     * @return The length (in codepoints) of the matched initial, or 0 if none.
     */
    static size_t find_longest_initial(const std::u32string& input, size_t start_pos);

    /**
     * @brief Performs comprehensive phonotactic validation of a syllable.
     *
     * Validates all component sets and applies orthographic co-occurrence rules.
     * @param syllable The syllable to validate.
     * @return true if the syllable is phonotactically valid Vietnamese.
     */
    static bool is_valid(const Syllable& syllable);
};

}  // namespace lotus_engine
