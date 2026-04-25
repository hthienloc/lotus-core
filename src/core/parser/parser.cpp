/**
 * @file parser.cpp
 * @brief Vietnamese syllable parsing logic.
 *
 * Implements the decomposition of raw character sequences into linguistic components
 * (initial, glide, vowel nucleus, and final coda).
 */

#include "lotus_core/parser.h"

#include "lotus_core/constants.h"
#include "lotus_core/unicode.h"
#include "lotus_core/parser_components.h"
#include "lotus_core/phonology_data.h"

#include <algorithm>

namespace lotus_core {

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
    return phonology::BASE_VOWELS.find(stripped) != std::u32string_view::npos ||
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
Syllable SyllableParser::parse(const std::u32string& input, bool allow_non_standard) {
    Syllable s;
    if (input.empty())
        return s;

    std::u32string normalized_input = unicode::normalize_nfc(input);

    size_t pos = 0;
    pos += InitialParser::parse(normalized_input, s, allow_non_standard);
    pos += parse_glide(normalized_input, pos, s);
    pos += parse_nucleus(normalized_input, pos, s);

    reorder_vowels(s);

    parse_coda(normalized_input, pos, s);

    return s;
}

void SyllableParser::reorder_vowels(Syllable& s) {
    if (s.vowel.empty() && !s.glide.has_value()) return;

    bool valid = true;
    if (!s.vowel.empty()) {
        valid = std::find(constants::VALID_NUCLEI_U32.begin(), constants::VALID_NUCLEI_U32.end(), s.vowel) != constants::VALID_NUCLEI_U32.end();
    }
    
    if (valid && s.glide.has_value()) {
        char32_t g = unicode::strip_tone(unicode::to_lower(s.glide.value()));
        char32_t next_char = s.vowel.empty() ? 0 : unicode::strip_tone(unicode::to_lower(s.vowel[0]));
        std::u32string lower_init = unicode::to_lower(s.initial);
        
        valid = false;
        for (const auto& rule : phonology::GLIDE_RULES) {
            if (rule.glide_char == g) {
                if (rule.initial_context.empty() || lower_init == rule.initial_context) {
                    if (rule.valid_next_chars.find(next_char) != std::u32string_view::npos) {
                        valid = true;
                        break;
                    }
                }
            }
        }
    }
    
    if (valid) return;

    // Zero-allocation, case-preserving reordering
    char32_t buf[8];
    size_t len = 0;
    if (s.glide.has_value()) {
        char32_t raw_g = s.glide.value();
        Tone t = unicode::get_tone(raw_g);
        if (t != Tone::NONE && s.tone == Tone::NONE) s.tone = t;
        buf[len++] = unicode::strip_tone(raw_g);
    }
    for (char32_t raw_v : s.vowel) {
        if (len < 8) {
            Tone t = unicode::get_tone(raw_v);
            if (t != Tone::NONE && s.tone == Tone::NONE) s.tone = t;
            buf[len++] = unicode::strip_tone(raw_v);
        }
    }
    
    if (len < 2) return;

    char32_t base_buf[8];
    char32_t sorted_base[8];
    for (size_t i = 0; i < len; ++i) {
        base_buf[i] = unicode::to_lower(buf[i]);
        sorted_base[i] = base_buf[i];
    }
    std::sort(sorted_base, sorted_base + len);

    auto is_permutation = [&](std::u32string_view test_seq) {
        if (test_seq.length() != len) return false;
        char32_t test_sorted[8];
        for (size_t i = 0; i < len; ++i) test_sorted[i] = test_seq[i];
        std::sort(test_sorted, test_sorted + len);
        for (size_t i = 0; i < len; ++i) {
            if (test_sorted[i] != sorted_base[i]) return false;
        }
        return true;
    };

    auto apply_match = [&](std::u32string_view test_seq, bool has_glide) {
        char32_t output[8];
        bool used[8] = {false};
        for (size_t i = 0; i < len; ++i) {
            char32_t target_base = test_seq[i];
            for (size_t j = 0; j < len; ++j) {
                if (!used[j] && base_buf[j] == target_base) {
                    used[j] = true;
                    output[i] = buf[j];
                    break;
                }
            }
        }
        
        if (has_glide) {
            s.glide = output[0];
            s.vowel.clear();
            for (size_t i = 1; i < len; ++i) s.vowel += output[i];
        } else {
            s.glide = std::nullopt;
            s.vowel.clear();
            for (size_t i = 0; i < len; ++i) s.vowel += output[i];
        }
    };

    // 1. Check no glide
    for (std::u32string_view n : constants::VALID_NUCLEI_U32) {
        if (is_permutation(n)) {
            apply_match(n, false);
            return;
        }
    }

    // 2. Check with glide
    for (std::u32string_view g_str : constants::VALID_GLIDES_U32) {
        if (g_str.empty()) continue;
        char32_t g = g_str[0];

        for (std::u32string_view n : constants::VALID_NUCLEI_U32) {
            if (n.empty()) continue;
            if (1 + n.length() != len) continue;

            char32_t combo[8];
            combo[0] = g;
            for (size_t i = 0; i < n.length(); ++i) combo[i+1] = n[i];
            std::u32string_view combo_view(combo, len);

            if (is_permutation(combo_view)) {
                char32_t next_char = n[0];
                bool glide_ok = false;
                std::u32string lower_init = unicode::to_lower(s.initial);
                for (const auto& rule : phonology::GLIDE_RULES) {
                    if (rule.glide_char == g) {
                        if (rule.initial_context.empty() || lower_init == rule.initial_context) {
                            if (rule.valid_next_chars.find(next_char) != std::u32string_view::npos) {
                                glide_ok = true;
                                break;
                            }
                        }
                    }
                }

                if (glide_ok) {
                    apply_match(combo_view, true);
                    return;
                }
            }
        }
    }
}

}  // namespace lotus_core
