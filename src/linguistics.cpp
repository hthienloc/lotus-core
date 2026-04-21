#include "lotus_engine/linguistics.h"

#include <algorithm>
#include <array>
#include <string_view>

namespace lotus_engine {

namespace {

constexpr std::array<std::string_view, 19> ENGLISH_CLUSTERS = {
    "br", "cl", "cr", "dr", "fl", "fr", "gl", "gr", "pl", "pr", "sc", "sh", 
    "sk", "sl", "sm", "sn", "sp", "st", "str"
};

constexpr std::array<std::string_view, 10> INVALID_FINALS = {
    "b", "d", "g", "k", "l", "r", "s", "v", "x", "z"
};

// Common English words that are short and conflict with TELEX markers
constexpr std::array<std::string_view, 12> ENGLISH_WHITELIST = {
    "are", "were", "she", "for", "and", "the", "always", "after", "how", "what", "where", "when"
};

} // namespace

bool Linguistics::is_on_whitelist(const std::string& word) {
    for (auto w : ENGLISH_WHITELIST) {
        if (word == w) return true;
    }
    return false;
}

bool Linguistics::is_definite_english(const std::string& word) {
    if (word.empty()) return false;
    if (is_on_whitelist(word)) return true;
    if (contains_english_cluster(word)) return true;
    return false;
}

bool Linguistics::is_likely_english(const std::string& word) {
    if (word.empty()) return false;
    if (is_definite_english(word)) return true;
    if (has_impossible_final(word)) return true;
    return false;
}

bool Linguistics::contains_english_cluster(const std::string& word) {
    if (word.length() < 2) return false;
    std::string lower = word;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    for (auto cluster : ENGLISH_CLUSTERS) {
        if (lower.find(cluster) != std::string::npos) {
            return true;
        }
    }
    return false;
}

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
