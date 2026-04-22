/**
 * @file parser.cpp
 * @brief Vietnamese syllable parsing logic.
 * 
 * Implements the decomposition of raw character sequences into linguistic components
 * (initial, glide, vowel nucleus, and final coda).
 */

#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/validator.h"
#include "lotus_engine/constants.h"

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
    return stripped == 'a' || stripped == U'ă' || stripped == U'â' || 
           stripped == 'e' || stripped == U'ê' || stripped == 'i' || 
           stripped == 'o' || stripped == U'ô' || stripped == U'ơ' || 
           stripped == 'u' || stripped == U'ư' || stripped == 'y' ||
           (c >= 0x0300 && c <= 0x036F); // Combining Marks
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
    if (input.empty()) return s;

    size_t n = input.size();

    // 1. Find Initial (Longest match for known Vietnamese initials)
    size_t initial_len = Validator::find_longest_initial(input, 0);
    if (initial_len > 0) {
        s.initial = input.substr(0, initial_len);
        // Vietnamese rule for 'gi':
        // 1. If followed by another vowel (e.g., 'gia', 'giáo'), 'gi' is the initial.
        // 2. If followed by nothing or a consonant (e.g., 'gì', 'gin'), 'g' is initial and 'i' is vowel.
        if (s.initial == U"gi" || s.initial == U"Gi") {
            if (input.size() == 2 || !is_vowel(input[2])) {
                s.initial = s.initial.substr(0, 1);
                initial_len = 1;
            }
        }
    }

    size_t pos = initial_len;
    if (pos >= n) return s;

    // 2. Glide
    bool has_glide = false;
    char32_t first_char = unicode::to_lower(input[pos]);
    
    if (pos + 1 < n) {
        char32_t next_char = unicode::strip_tone(unicode::to_lower(input[pos + 1]));
        if (first_char == 'o') {
            if (next_char == 'a' || next_char == 'e' || next_char == U'ă') has_glide = true;
        } else if (first_char == 'u') {
            bool is_qu = (s.initial == U"q" || s.initial == U"Q");
            if (is_qu) {
                if (next_char == 'a' || next_char == 'e' || next_char == 'i' || 
                    next_char == U'â' || next_char == U'ê' || next_char == 'o' ||
                    next_char == U'ô' || next_char == U'ơ' || next_char == 'u' ||
                    next_char == U'ư' || next_char == 'y') has_glide = true;
            } else {
                if (next_char == U'ê' || next_char == 'y' || 
                    next_char == U'â' || next_char == U'ơ' || next_char == U'ô') has_glide = true;
            }
        }
    }

    if (has_glide) {
        s.glide = first_char;
        pos++;
    }

    // 3. Vowel Nucleus
    while (pos < n && is_vowel(input[pos])) {
        // Collect pre-composed tone if any
        if (s.tone == Tone::NONE) {
            Tone t = unicode::get_tone(input[pos]);
            if (t != Tone::NONE) s.tone = t;
        }
        s.vowel += unicode::strip_tone(input[pos]);
        pos++;
    }

    // 4. Final Coda
    if (pos < n) {
        s.final_c = input.substr(pos);
    }

    return s;
}

} // namespace lotus_engine
