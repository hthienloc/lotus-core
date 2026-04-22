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

} // namespace

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
    if (word.length() < 2) return false;
    std::string lower = unicode::to_lower(word);
    return std::any_of(ENGLISH_CLUSTERS.begin(), ENGLISH_CLUSTERS.end(),
                       [&lower](std::string_view cluster) { return lower.find(cluster) == 0; });
}

/**
 * @brief Heuristic check to see if a word is likely English.
 */
bool Linguistics::is_likely_english(const std::string& word) {
    if (word.empty()) return false;
    if (is_definite_english(word)) return true;
    if (has_impossible_final(word)) return true;

    // Check for English 'y' consonant patterns (e.g., 'yes', 'yard')
    // In Vietnamese, initial 'y' is strictly followed by 'ê' or nothing.
    std::string lower = unicode::to_lower(word);
    if (lower.length() >= 2 && lower[0] == 'y') {
        if (is_vowel(lower[1]) && lower[1] != 'w' && lower.find("ê") == std::string::npos) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Checks if the word ends with a character or combination impossible in Vietnamese.
 * @param word The word to check.
 * @return True if the final sequence is invalid in Vietnamese.
 */
bool Linguistics::has_impossible_final(const std::string& word) {
    if (word.empty()) return false;
    char last = static_cast<char>(::tolower(word.back()));
    
    bool is_invalid_base = std::any_of(INVALID_FINALS.begin(), INVALID_FINALS.end(),
                                       [last](std::string_view f) { return last == f[0]; });

    if (is_invalid_base) {
        // Special Case: 'ng' is valid, but 'g' alone at end is not.
        if (last == 'g') {
            if (word.length() >= 2 && ::tolower(word[word.length() - 2]) == 'n') return false;
        }
        
        // TELEX tone markers at the end are allowed if they follow a vowel (likely Vietnamese).
        // If they follow a consonant, they are likely English (e.g., 'box', 'bus').
        if (is_tone_marker(last) && word.length() >= 2) {
            char prev = static_cast<char>(::tolower(word[word.length() - 2]));
            if (last == prev) return false; // Telex double-tap escape
            return !is_vowel(prev);         // English if consonant + marker
        }
        return true;
    }
    return false;
}

} // namespace lotus_engine
