#pragma once

#include "lotus_engine/types.h"

#include <string>
#include <vector>

namespace lotus_engine {

/**
 * @brief Parses a UTF-32 encoded string into a structured @ref Syllable.
 *
 * Implements context-sensitive Vietnamese parsing rules including:
 * - Longest-match initial consonant detection.
 * - Glide disambiguation (e.g., 'gi' vs 'g' + vowel 'i').
 * - Pre-composed tone extraction.
 */
class SyllableParser {
   public:
    /**
     * @brief Parses a UTF-32 character sequence into a Syllable structure.
     * @param input The raw input string in UTF-32 encoding.
     * @return Syllable The parsed syllable components.
     */
    static Syllable parse(const std::u32string& input);

    /**
     * @brief Checks whether a UTF-32 codepoint is a Vietnamese vowel.
     * @param c The character to test.
     * @return true if the character is a vowel or combining mark.
     */
    static bool is_vowel(char32_t c);

   private:
    static std::u32string to_u32(const std::string& s);
    static std::string from_u32(const std::u32string& s);
};

}  // namespace lotus_engine
