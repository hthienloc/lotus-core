#include "lotus_engine/linguistics.h"

#include <algorithm>
#include <string_view>
#include <vector>

namespace lotus_engine {

namespace {

const std::vector<std::string_view> ENGLISH_CLUSTERS = {
    "br", "cl", "cr", "dr", "dw", "fl", "fr", "gl", "gr", "pl", "pr", "sc", "scr", "sh", "shr",
    "sk", "sl", "sm", "sn", "sp", "spl", "spr", "squ", "st", "str", "sw"
};

const std::vector<std::string_view> INVALID_FINALS = {
    "b", "d", "f", "g", "j", "k", "l", "r", "s", "v", "x", "z"
};

// Common English words that are short and conflict with TELEX markers
const std::vector<std::string_view> ENGLISH_WHITELIST = {};

} // namespace

bool Linguistics::is_on_whitelist(const std::string& word) {
    for (auto w : ENGLISH_WHITELIST) {
        if (word == w) return true;
    }
    return false;
}

bool Linguistics::is_definite_english(const std::string& word) {
    if (word.empty()) return false;
    if (contains_english_cluster(word)) return true;
    return false;
}

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

    // Check for misplaced tone markers (s, r, f, x, j in the middle)
    if (word.length() >= 3) {        for (size_t i = 1; i < word.length() - 1; ++i) {
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
