#pragma once

#include "lotus_core/types.h"

#include <string>
#include <vector>

namespace lotus_core {

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
     * @param allow_non_standard Whether to allow z, w, j, f.
     * @return Syllable The parsed syllable components.
     */
    static Syllable parse(const std::u32string& input, bool allow_non_standard = false);

    /**
     * @brief Checks whether a UTF-32 codepoint is a Vietnamese vowel.
     * @param c The character to test.
     * @return true if the character is a vowel or combining mark.
     */
    static bool is_vowel(char32_t c);

   private:
    static size_t parse_initial(const std::u32string& input, Syllable& s);
    static size_t parse_glide(const std::u32string& input, size_t pos, Syllable& s);
    static size_t parse_nucleus(const std::u32string& input, size_t pos, Syllable& s);
    static size_t parse_coda(const std::u32string& input, size_t pos, Syllable& s);

    static void reorder_vowels(Syllable& s);

    static std::u32string to_u32(const std::string& s);
    static std::string from_u32(const std::u32string& s);
};

}  // namespace lotus_core
