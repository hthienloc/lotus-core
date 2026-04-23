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
#include "lotus_engine/validator.h"

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
 */
size_t SyllableParser::parse_initial(const std::u32string& input, Syllable& s) {
    size_t initial_len = Validator::find_longest_initial(input, 0);
    if (initial_len == 0)
        return 0;

    s.initial = input.substr(0, initial_len);
    std::u32string lower_init = unicode::to_lower(s.initial);

    // Vietnamese rule for 'gi':
    if (lower_init == U"gi") {
        if (input.size() == 2 || !is_vowel(input[2])) {
            s.initial = s.initial.substr(0, 1);
            initial_len = 1;
        }
    }
    // Vietnamese rule for 'qu':
    else if (lower_init == U"qu") {
        s.initial = s.initial.substr(0, 1);
        initial_len = 1;
    }

    return initial_len;
}

/**
 * @brief Identifies and extracts the glide ('o' or 'u' following an initial).
 */
size_t SyllableParser::parse_glide(const std::u32string& input, size_t pos, Syllable& s) {
    size_t n = input.size();
    if (pos >= n)
        return 0;

    bool has_glide = false;
    char32_t first_char = unicode::to_lower(input[pos]);

    if (pos + 1 < n) {
        char32_t next_char = unicode::strip_tone(unicode::to_lower(input[pos + 1]));
        if (first_char == 'o') {
            if (next_char == 'a' || next_char == 'e' || next_char == U'ă')
                has_glide = true;
        } else if (first_char == 'u') {
            std::u32string lower_init = unicode::to_lower(s.initial);
            bool is_qu = (lower_init == U"q");
            if (is_qu) {
                if (next_char == 'a' || next_char == 'e' || next_char == 'i' || next_char == U'â' ||
                    next_char == U'ê' || next_char == 'o' || next_char == U'ô' ||
                    next_char == U'ơ' || next_char == 'u' || next_char == U'ư' || next_char == 'y')
                    has_glide = true;
            } else {
                if (next_char == U'ê' || next_char == 'y' || next_char == U'â' ||
                    next_char == U'ơ' || next_char == U'ô')
                    has_glide = true;
            }
        }
    }

    if (has_glide) {
        s.glide = first_char;
        return 1;
    }

    return 0;
}

/**
 * @brief Extracts the vowel nucleus sequence and identifies the tone mark if present.
 */
size_t SyllableParser::parse_nucleus(const std::u32string& input, size_t pos, Syllable& s) {
    size_t n = input.size();
    size_t len = 0;

    while (pos + len < n && is_vowel(input[pos + len])) {
        if (s.tone == Tone::NONE) {
            Tone t = unicode::get_tone(input[pos + len]);
            if (t != Tone::NONE)
                s.tone = t;
        }
        s.vowel += unicode::strip_tone(input[pos + len]);
        len++;
    }

    return len;
}

/**
 * @brief Extracts the remaining characters as the final coda.
 */
size_t SyllableParser::parse_coda(const std::u32string& input, size_t pos, Syllable& s) {
    if (pos < input.size()) {
        s.final_c = input.substr(pos);
        return s.final_c.size();
    }
    return 0;
}

/**
 * @brief Parses a raw string into a structured Syllable object.
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
