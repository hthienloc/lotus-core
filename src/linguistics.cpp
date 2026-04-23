/**
 * @file linguistics.cpp
 * @brief Heuristics and linguistic analysis for language detection.
 *
 * Implements detection logic to distinguish between Vietnamese and English text,
 * preventing false transformations on English words.
 */

#include "lotus_engine/linguistics.h"

#include "lotus_engine/constants.h"
#include "lotus_engine/unicode.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

namespace lotus_engine {

using namespace lotus_engine::constants;

// ============================================================================
// [ Internal Data ]
// ============================================================================

namespace {

/**
 * @brief Common English words that are short and conflict with TELEX markers.
 */
const std::vector<std::string_view> ENGLISH_WHITELIST = {
    // To be populated later.
};

/**
 * @brief Checks if a character is a vowel (including 'y' and 'w' for TELEX purposes).
 */
inline bool is_vowel(char c) {
    return VOWELS.find(static_cast<char>(::tolower(c))) != std::string_view::npos;
}

/**
 * @brief Checks if a character is a TELEX tone marker.
 */
inline bool is_tone_marker(char c) {
    return TELEX_TONE_MARKERS.find(static_cast<char>(::tolower(c))) != std::string_view::npos;
}

}  // namespace

// ============================================================================
// [ Linguistics Implementation ]
// ============================================================================

/**
 * @brief Checks if a word is present in the protected English whitelist.
 * @param word The word to check.
 * @return True if the word is whitelisted.
 */
bool Linguistics::is_on_whitelist(const std::string& word) {
    return std::any_of(ENGLISH_WHITELIST.begin(), ENGLISH_WHITELIST.end(),
                       [&word](std::string_view w) { return word == w; });
}

/**
 * @brief Determines if a word is definitively English based on character clusters.
 * @param word The word to analyze.
 * @return True if it contains definite English clusters.
 */
bool Linguistics::is_definite_english(const std::string& word) {
    if (word.length() < 2)
        return false;
    std::string lower = unicode::to_lower(word);

    if (has_english_x_pattern(lower))
        return true;

    if (has_english_start_cluster(lower))
        return true;

    return false;
}

/**
 * @brief Heuristic check to see if a word is likely English.
 *
 * @param word The word to analyze.
 * @return True if heuristics suggest it is English.
 */
bool Linguistics::is_likely_english(const std::string& word) {
    if (word.empty())
        return false;

    std::string lower = unicode::to_lower(word);

    if (is_definite_english(word))
        return true;

    if (has_impossible_final(lower))
        return true;

    // Check for non-Vietnamese consonant clusters anywhere in the word.
    // We use this in is_likely_english instead of is_definite_english
    // because some clusters might temporarily appear during typing (e.g., 'st' in 'test').
    if (contains_english_cluster(lower))
        return true;

    if (has_english_y_pattern(lower))
        return true;

    return false;
}

// ============================================================================
// [ Private Helper Methods ]
// ============================================================================

/**
 * @brief Checks if the word contains 'x' in positions unlikely for Vietnamese.
 *
 * 1. Heuristic: If it contains 'x' in positions that are unlikely for
 * Vietnamese (which only allows initial 'x') but common for English.
 * For example, 'mixi', 'boxer', 'taxi'.
 *
 * @param lower The lowercased word to check.
 * @return True if it contains an English 'x' pattern.
 */
bool Linguistics::has_english_x_pattern(const std::string& lower) {
    if (lower.length() >= 3) {
        size_t x_pos = lower.find('x');
        if (x_pos != std::string::npos && x_pos > 0 && x_pos < lower.length() - 1) {
            // 'x' in the middle followed by a vowel is definitely English.
            // If followed by another 'x', it's a Telex escape.
            if (is_vowel(lower[x_pos + 1]))
                return true;
        }
    }
    return false;
}

/**
 * @brief Checks for definite English clusters at the start.
 * @param lower The lowercased word to check.
 * @return True if it starts with an English cluster.
 */
bool Linguistics::has_english_start_cluster(const std::string& lower) {
    // 2. Check for definite English clusters at the start.
    return std::any_of(ENGLISH_CLUSTERS.begin(), ENGLISH_CLUSTERS.end(),
                       [&lower](std::string_view cluster) { return lower.find(cluster) == 0; });
}

/**
 * @brief Checks for English 'y' consonant patterns (e.g., 'yes', 'yard').
 *
 * In Vietnamese, initial 'y' is strictly followed by 'ê' or nothing.
 *
 * @param lower The lowercased word to check.
 * @return True if it contains an English 'y' pattern.
 */
bool Linguistics::has_english_y_pattern(const std::string& lower) {
    if (lower.length() >= 2 && lower[0] == 'y') {
        if (is_vowel(lower[1]) && lower[1] != 'w' && lower.find("ê") == std::string::npos) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks for non-Vietnamese consonant clusters (sh, wh, br, str, etc.)
 */
bool Linguistics::contains_english_cluster(const std::string& lower) {
    if (lower.length() < 2)
        return false;

    return std::any_of(
        ENGLISH_CLUSTERS.begin(), ENGLISH_CLUSTERS.end(), [&lower](std::string_view cluster) {
            size_t pos = lower.find(cluster);
            if (pos == std::string::npos)
                return false;

            // Special Case 1: 'st' at the end is common in both English and
            // temporary Vietnamese typing (e.g., 'test' -> 'tét').
            if (cluster == "st" && pos == lower.length() - 2)
                return false;

            // Special Case 2: Double-tap escape markers at the end (e.g., 'as' -> 'á', 'ass' ->
            // 'as') Telex allows double tapping any tone marker to escape it.
            if (cluster.length() == 2 && cluster[0] == cluster[1] && pos == lower.length() - 2) {
                if (TELEX_TONE_MARKERS.find(cluster[0]) != std::string_view::npos)
                    return false;
            }

            return true;
        });
}

/**
 * @brief Checks if the word ends with a character or combination impossible in Vietnamese.
 * @param lower The lowercased word to check.
 * @return True if the final sequence is invalid in Vietnamese.
 */
bool Linguistics::has_impossible_final(const std::string& lower) {
    if (lower.empty())
        return false;
    char last = lower.back();

    bool is_invalid_base = std::any_of(INVALID_FINALS.begin(), INVALID_FINALS.end(),
                                       [last](std::string_view f) { return last == f[0]; });

    if (is_invalid_base) {
        // Special Case: 'ng' is valid, but 'g' alone at end is not.
        if (last == 'g') {
            if (lower.length() >= 2 && lower[lower.length() - 2] == 'n')
                return false;
        }

        // TELEX tone markers at the end are allowed if they follow a vowel (likely Vietnamese).
        // If they follow a consonant, they are likely English (e.g., 'was').
        if (is_tone_marker(last) && lower.length() >= 2) {
            char prev = lower[lower.length() - 2];
            if (last == prev)
                return false;        // Telex double-tap escape
            return !is_vowel(prev);  // English if consonant + marker
        }
        return true;
    }
    return false;
}

}  // namespace lotus_engine
