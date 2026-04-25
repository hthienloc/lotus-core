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
#include "lotus_core/log.h"

#include <algorithm>

using namespace lotus_core;

using namespace constants;

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
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Initial: [" + unicode::to_utf8(s.initial) + "]"));

    pos += parse_glide(normalized_input, pos, s);
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Glide: [" + (s.glide.has_value() ? unicode::to_utf8(s.glide.value()) : "") + "]"));

    pos += parse_nucleus(normalized_input, pos, s);
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Nucleus: [" + unicode::to_utf8(s.vowel) + "], Tone: " + std::to_string(static_cast<int>(s.tone))));

    reorder_vowels(s);
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Reorder: Glide=[" + (s.glide.has_value() ? unicode::to_utf8(s.glide.value()) : "") + "], Nucleus=[" + unicode::to_utf8(s.vowel) + "]"));

    parse_coda(normalized_input, pos, s);
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Coda: [" + unicode::to_utf8(s.final_c) + "]"));

    return s;
}

void SyllableParser::reorder_vowels(Syllable& s) {
    if (s.vowel.empty() && !s.glide.has_value()) return;

    bool valid = true;
    if (!s.vowel.empty()) {
        valid = std::find(VALID_NUCLEI_U32.begin(), VALID_NUCLEI_U32.end(), s.vowel) != VALID_NUCLEI_U32.end();
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

    std::u32string combined;
    if (s.glide.has_value()) {
        char32_t raw_g = s.glide.value();
        Tone t = unicode::get_tone(raw_g);
        if (t != Tone::NONE && s.tone == Tone::NONE) s.tone = t;
        combined += unicode::strip_tone(raw_g);
    }
    for (char32_t raw_v : s.vowel) {
        Tone t = unicode::get_tone(raw_v);
        if (t != Tone::NONE && s.tone == Tone::NONE) s.tone = t;
        combined += unicode::strip_tone(raw_v);
    }

    if (combined.length() < 2) return;

    std::u32string base_combined;
    for (char32_t c : combined) {
        base_combined += unicode::to_lower(c);
    }

    auto apply_match = [&](std::u32string_view target_seq, bool has_glide) {
        std::u32string result;
        std::vector<bool> used(combined.length(), false);
        for (char32_t target_c : target_seq) {
            for (size_t i = 0; i < base_combined.length(); ++i) {
                if (!used[i] && base_combined[i] == target_c) {
                    used[i] = true;
                    result += combined[i];
                    break;
                }
            }
        }

        if (has_glide) {
            s.glide = result[0];
            s.vowel = result.substr(1);
        } else {
            s.glide = std::nullopt;
            s.vowel = result;
        }
    };

    // 1. Check no glide
    for (std::u32string_view n : VALID_NUCLEI_U32) {
        if (n.length() == base_combined.length() &&
            std::is_permutation(n.begin(), n.end(), base_combined.begin())) {
            apply_match(n, false);
            return;
        }
    }

    // 2. Check with glide
    for (std::u32string_view g_str : VALID_GLIDES_U32) {
        if (g_str.empty()) continue;
        char32_t g = g_str[0];

        for (std::u32string_view n : VALID_NUCLEI_U32) {
            if (n.empty() || (1 + n.length() != base_combined.length())) continue;

            std::u32string combo;
            combo += g;
            combo += n;

            if (std::is_permutation(combo.begin(), combo.end(), base_combined.begin())) {
                char32_t next_char = n[0];
                bool glide_ok = false;
                std::u32string lower_init = unicode::to_lower(s.initial);
                
                for (const auto& rule : phonology::GLIDE_RULES) {
                    if (rule.glide_char == g &&
                        (rule.initial_context.empty() || lower_init == rule.initial_context) &&
                        rule.valid_next_chars.find(next_char) != std::u32string_view::npos) {
                        glide_ok = true;
                        break;
                    }
                }

                if (glide_ok) {
                    apply_match(combo, true);
                    return;
                }
            }
        }
    }
}

}  // namespace lotus_core
