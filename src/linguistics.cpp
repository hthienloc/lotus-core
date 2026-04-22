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
    "for", "to", "if", "of", "is", "was", "by", "from", "are", "with", "the"
};

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
    for (auto w : ENGLISH_WHITELIST) {
        if (word == w) return true;
    }
    return false;
}

/**
 * @brief Determines if a word is definitively English based on character clusters.
 * @param word The word to analyze.
 * @return True if it contains definite English clusters.
 */
bool Linguistics::is_definite_english(const std::string& word) {
    if (word.empty()) return false;
    if (contains_english_cluster(word)) return true;
    return false;
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
        char second = ::tolower(word[1]);
        if (second == 'o' || second == 'u' || second == 'a' || second == 'e' || second == 'i') {
            return true;
        }
    }

    // Check for misplaced tone markers (s, r, f, x, j in the middle of a word)
    if (word.length() >= 3) {
        for (size_t i = 1; i < word.length() - 1; ++i) {
            char c = ::tolower(word[i]);
            if (c == 's' || c == 'r' || c == 'f' || c == 'x' || c == 'j') {
                // If it's a double-typed escape (e.g. 'ass'), don't treat as misplaced
                if (c == ::tolower(word[i + 1]))
                    continue;
                // Exemption for 'j' following 'c' at the end of a word (standard Vietnamese 'việc')
                // If anything follows the 'j' (like 'viecje'), it's likely English.
                if (c == 'j' && i > 0 && ::tolower(word[i - 1]) == 'c' && i == word.length() - 1)
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
    
    for (auto cluster : ENGLISH_CLUSTERS) {
        if (lower.find(cluster) == 0) {
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
    char last = ::tolower(word.back());
    
    for (auto f : INVALID_FINALS) {
        if (last == f[0]) {
            // Special Case: 'ng' is valid, but 'g' alone at end is not.
            if (last == 'g') {
                if (word.length() >= 2 && ::tolower(word[word.length()-2]) == 'n') {
                    return false; // Valid Vietnamese 'ng'
                }
            }
            // Special Case TELEX: s, r, x, f, j are markers.
            // If they are at the end, they MIGHT be tones.
            // However, if they follow another consonant, they are definitely English (e.g. 'is', 'bus')
            if (last == 's' || last == 'r' || last == 'x' || last == 'f' || last == 'j') {
                 if (word.length() >= 2) {
                     char prev = ::tolower(word[word.length()-2]);
                     // Special Case: Double tone key is a TELEX escape (e.g. 'ass' -> 'as')
                     if (last == prev) return false;
                     
                     // If follows a consonant (not a,e,o,u,i,y), it's English
                     if (prev != 'a' && prev != 'e' && prev != 'o' && prev != 'u' && prev != 'i' && prev != 'y' && prev != 'w') {
                         return true;
                     }
                     return false; // Could be a tone mark for a vowel
                 }
            }
            return true;
        }
    }
    return false;
}

} // namespace lotus_engine
