/**
 * @file parser.cpp
 * @brief Vietnamese syllable parsing logic.
 *
 * Implements the decomposition of raw character sequences into linguistic components
 * (initial, glide, vowel nucleus, and final coda).
 */

#include "lotus_engine/parser.h"

#include "lotus_engine/constants.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/parser_components.h"

#include <algorithm>

namespace lotus_engine {

// ============================================================================
// [ Parser Implementation ]
// ============================================================================

/**
 * @brief Utility to check if a character is a Vietnamese vowel or combining mark.
 * @param c The UTF-32 character to check.
 * @return True if the character is a vowel component.
 */
bool SyllableParser::is_vowel(char32_t c) {
    char32_t low = unicode::to_lower(c);
    char32_t stripped = unicode::strip_tone(low);
    // Standard Vietnamese vowels (and their toned versions)
    return stripped == 'a' || stripped == U'ă' || stripped == U'â' || stripped == 'e' ||
           stripped == U'ê' || stripped == 'i' || stripped == 'o' || stripped == U'ô' ||
           stripped == U'ơ' || stripped == 'u' || stripped == U'ư' || stripped == 'y' ||
           (c >= 0x0300 && c <= 0x036F);  // Combining Marks
}

/**
 * @brief Identifies and extracts the initial consonant.
 * @param input The raw input character sequence.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the initial consonant.
 */
size_t SyllableParser::parse_initial(const std::u32string& input, Syllable& s) {
    return InitialParser::parse(input, s);
}

/**
 * @brief Identifies and extracts the glide ('o' or 'u' following an initial).
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the glide (0 or 1).
 */
size_t SyllableParser::parse_glide(const std::u32string& input, size_t pos, Syllable& s) {
    return GlideParser::parse(input, pos, s);
}

/**
 * @brief Extracts the vowel nucleus sequence and identifies the tone mark if present.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the nucleus.
 */
size_t SyllableParser::parse_nucleus(const std::u32string& input, size_t pos, Syllable& s) {
    return NucleusParser::parse(input, pos, s);
}

/**
 * @brief Extracts the remaining characters as the final coda.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the coda.
 */
size_t SyllableParser::parse_coda(const std::u32string& input, size_t pos, Syllable& s) {
    return CodaParser::parse(input, pos, s);
}

/**
 * @brief Parses a raw string into a structured Syllable object.
 *
 * Performs linguistic analysis to identify the initial consonant,
 * the glide (if present), the vowel nucleus, and the final coda.
 *
 * @param input The raw character sequence to parse.
 * @return A Syllable object containing the identified components.
 */
Syllable SyllableParser::parse(const std::u32string& input) {
    Syllable s;
    if (input.empty())
        return s;

    size_t pos = 0;
    pos += parse_initial(input, s);
    pos += parse_glide(input, pos, s);
    pos += parse_nucleus(input, pos, s);
    parse_coda(input, pos, s);

    return s;
}

}  // namespace lotus_engine
