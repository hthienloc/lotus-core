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
 * @param allow_non_standard Whether to allow z, w, j, f.
 * @return InitialParseResult The parsed initial and consumed count.
 */
InitialParseResult SyllableParser::parse_initial(std::u32string_view input, bool allow_non_standard) {
    return InitialParser::parse(input, allow_non_standard);
}

/**
 * @brief Identifies and extracts the glide ('o' or 'u' following an initial).
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param initial The already parsed initial.
 * @return GlideParseResult The parsed glide and consumed count.
 */
GlideParseResult SyllableParser::parse_glide(std::u32string_view input, size_t pos, std::u32string_view initial) {
    return GlideParser::parse(input, pos, initial);
}

/**
 * @brief Extracts the vowel nucleus sequence and identifies the tone mark if present.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @return NucleusParseResult The parsed nucleus, tone, and consumed count.
 */
NucleusParseResult SyllableParser::parse_nucleus(std::u32string_view input, size_t pos) {
    return NucleusParser::parse(input, pos);
}

/**
 * @brief Extracts the remaining characters as the final coda.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @return CodaParseResult The parsed coda and consumed count.
 */
CodaParseResult SyllableParser::parse_coda(std::u32string_view input, size_t pos) {
    return CodaParser::parse(input, pos);
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
    std::u32string_view view = normalized_input;

    size_t pos = 0;
    InitialParseResult init_res = parse_initial(view, allow_non_standard);
    s.initial = init_res.initial;
    pos += init_res.consumed;
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Initial: [" + unicode::to_utf8(s.initial.view()) + "]"));

    GlideParseResult glide_res = parse_glide(view, pos, s.initial.view());
    s.glide = glide_res.glide;
    pos += glide_res.consumed;
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Glide: [" + (s.glide.has_value() ? unicode::to_utf8(s.glide.value()) : "") + "]"));

    NucleusParseResult nuc_res = parse_nucleus(view, pos);
    s.vowel = nuc_res.vowel;
    s.tone = nuc_res.tone;
    pos += nuc_res.consumed;
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Nucleus: [" + unicode::to_utf8(s.vowel.view()) + "], Tone: " + std::to_string(static_cast<int>(s.tone))));

    reorder_vowels(s);
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Reorder: Glide=[" + (s.glide.has_value() ? unicode::to_utf8(s.glide.value()) : "") + "], Nucleus=[" + unicode::to_utf8(s.vowel.view()) + "]"));

    CodaParseResult coda_res = parse_coda(view, pos);
    s.final_c = coda_res.final_c;
    LOTUS_LOG_DEBUG(format_log_message("PARSER", "Coda: [" + unicode::to_utf8(s.final_c.view()) + "]"));

    return s;
}

void SyllableParser::reorder_vowels(Syllable& s) {
    if (s.vowel.empty() && !s.glide.has_value()) return;

    bool valid = true;
    if (!s.vowel.empty()) {
        valid = std::find(VALID_NUCLEI_U32.begin(), VALID_NUCLEI_U32.end(), s.vowel.view()) != VALID_NUCLEI_U32.end();
    }

    StaticString lower_init_str;
    for (char32_t c : s.initial.view()) {
        lower_init_str += unicode::to_lower(c);
    }
    std::u32string_view lower_init = lower_init_str.view();
    
    if (valid && s.glide.has_value()) {
        char32_t g = unicode::strip_tone(unicode::to_lower(s.glide.value()));
        char32_t next_char = s.vowel.empty() ? 0 : unicode::strip_tone(unicode::to_lower(s.vowel[0]));
        
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

    StaticString combined_orig;
    StaticString combined_base;

    if (s.glide.has_value()) {
        char32_t raw_g = s.glide.value();
        Tone t = unicode::get_tone(raw_g);
        if (t != Tone::NONE && s.tone == Tone::NONE) s.tone = t;
        char32_t stripped = unicode::strip_tone(raw_g);
        combined_orig += stripped;
        combined_base += unicode::to_lower(stripped);
    }
    for (char32_t raw_v : s.vowel.view()) {
        Tone t = unicode::get_tone(raw_v);
        if (t != Tone::NONE && s.tone == Tone::NONE) s.tone = t;
        char32_t stripped = unicode::strip_tone(raw_v);
        combined_orig += stripped;
        combined_base += unicode::to_lower(stripped);
    }

    if (combined_base.size() < 2) return;

    // Sort the base string to match against our precomputed table
    StaticString sorted_base = combined_base;
    std::sort(sorted_base.begin(), sorted_base.end());
    std::u32string_view sorted_view = sorted_base.view();

    const phonology::ReorderPattern* matched_pattern = nullptr;

    for (const auto& pattern : phonology::REORDER_PATTERNS) {
        if (pattern.sorted_chars == sorted_view) {
            if (pattern.has_glide) {
                char32_t next_char = pattern.vowel.empty() ? 0 : pattern.vowel[0];
                bool glide_ok = false;
                for (const auto& rule : phonology::GLIDE_RULES) {
                    if (rule.glide_char == pattern.glide &&
                        (rule.initial_context.empty() || lower_init == rule.initial_context) &&
                        rule.valid_next_chars.find(next_char) != std::u32string_view::npos) {
                        glide_ok = true;
                        break;
                    }
                }
                if (!glide_ok) continue;
            }
            matched_pattern = &pattern;
            break; // found the first valid pattern
        }
    }

    if (!matched_pattern) return;

    // Reconstruct using original casing
    StaticString new_vowel;
    std::optional<char32_t> new_glide = std::nullopt;

    bool used[8] = {false}; // Max length is safe to bound

    auto extract_orig_char = [&](char32_t target_c) -> char32_t {
        for (size_t i = 0; i < combined_base.size(); ++i) {
            if (!used[i] && combined_base[i] == target_c) {
                used[i] = true;
                return combined_orig[i];
            }
        }
        return target_c; // Should never happen if pattern matches
    };

    if (matched_pattern->has_glide) {
        new_glide = extract_orig_char(matched_pattern->glide);
    }

    for (char32_t target_c : matched_pattern->vowel) {
        new_vowel += extract_orig_char(target_c);
    }

    s.glide = new_glide;
    s.vowel = new_vowel;
}

}  // namespace lotus_core
