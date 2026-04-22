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

constexpr std::string_view VOWELS = "aeiouyw";
constexpr std::string_view TONE_MARKERS = "srfxj";

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
    return TONE_MARKERS.find(static_cast<char>(::tolower(c))) != std::string_view::npos;
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
    return !word.empty() && contains_english_cluster(word);
}

/**
 * @brief Heuristic check to see if a word is likely English.
 * 
 * Checks for misplaced markers, impossible Vietnamese character combinations,
 * and English-specific consonant patterns.
 * 
 * @param word The word to analyze.
 * @return True if heuristics suggest it is English.
 */
bool Linguistics::is_likely_english(const std::string& word) {
    if (word.empty()) return false;
    if (is_definite_english(word)) return true;
    if (has_impossible_final(word)) return true;

    // Check for English 'y' consonant (y followed by o, u, a, e, i)
    // In Vietnamese, initial 'y' is only followed by 'ê' or nothing.
    if (word.length() >= 2 && ::tolower(word[0]) == 'y') {
        if (is_vowel(word[1]) && ::tolower(word[1]) != 'w') {
            return true;
        }
    }

    // Check for misplaced tone markers (s, r, f, x, j in the middle of a word)
    if (word.length() >= 3) {
        for (size_t i = 1; i < word.length() - 1; ++i) {
            char c = word[i];
            if (is_tone_marker(c)) {
                // If it's a double-typed escape (e.g. 'ass'), don't treat as misplaced
                if (::tolower(c) == ::tolower(word[i + 1]))
                    continue;
                
                // Allow tone markers if they follow a vowel (could be mid-word typing)
                if (is_vowel(word[i - 1]))
                    continue;

                // Exemption for 'j' following 'c' at the end of a word (standard Vietnamese 'việc')
                if (::tolower(c) == 'j' && i > 0 && ::tolower(word[i - 1]) == 'c' && i == word.length() - 1)
                    continue;
                return true;
            }
        }
    }
    
    return false;
}

/**
 * @brief Checks if the word starts with clusters impossible in Vietnamese but common in English.
 * @param word The word to check.
 * @return True if an English cluster is found.
 */
bool Linguistics::contains_english_cluster(const std::string& word) {
    if (word.length() < 2) return false;
    std::string lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    return std::any_of(ENGLISH_CLUSTERS.begin(), ENGLISH_CLUSTERS.end(),
                       [&lower](std::string_view cluster) { return lower.find(cluster) == 0; });
}

/**
 * @brief Checks if the word ends with a character or combination impossible in Vietnamese.
 * @param word The word to check.
 * @return True if the final sequence is invalid in Vietnamese.
 */
bool Linguistics::has_impossible_final(const std::string& word) {
    if (word.empty()) return false;
    char last = ::tolower(word.back());
    
    bool is_invalid_final = std::any_of(INVALID_FINALS.begin(), INVALID_FINALS.end(),
                                        [last](std::string_view f) { return last == f[0]; });

    if (is_invalid_final) {
        // Special Case: 'ng' is valid, but 'g' alone at end is not.
        if (last == 'g') {
            if (word.length() >= 2 && ::tolower(word[word.length() - 2]) == 'n') {
                return false; // Valid Vietnamese 'ng'
            }
        }
        
        // Special Case TELEX: s, r, x, f, j are markers.
        // If they are at the end, they MIGHT be tones.
        // However, if they follow another consonant, they are definitely English (e.g. 'is', 'bus')
        if (is_tone_marker(last)) {
            if (word.length() >= 2) {
                char prev = word[word.length() - 2];
                // Special Case: Double tone key is a TELEX escape (e.g. 'ass' -> 'as')
                if (::tolower(last) == ::tolower(prev)) return false;
                
                // If follows a consonant, it's English
                if (!is_vowel(prev)) {
                    return true;
                }
                return false; // Could be a tone mark for a vowel
            }
        }
        return true;
    }
    return false;
}

} // namespace lotus_engine
