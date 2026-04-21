#include "lotus_engine/parser.h"
#include "lotus_engine/validator.h"
#include "lotus_engine/unicode.h"
#include <algorithm>
#include <unordered_set>

namespace lotus_engine {

// Helper for UTF conversion
std::u32string SyllableParser::to_u32(const std::string& s) {
    return unicode::to_utf32(s);
}

std::string SyllableParser::from_u32(const std::u32string& s) {
    return unicode::to_utf8(s);
}

bool SyllableParser::is_vowel(char32_t c) {
    if (c < 128) {
        char ch = (char)unicode::to_lower(c);
        return ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' || ch == 'y';
    }
    
    // Combining Marks (0x0300-0x036F)
    if (c >= 0x0300 && c <= 0x036F) return true;

    // Vietnamese-specific vowel ranges (simplified check)
    // Most precomposed vowels are in these blocks
    if (c >= 0x00C0 && c <= 0x024F) return true; // Very broad Latin block
    if (c >= 0x1E00 && c <= 0x1EFF) return true; // Latin Extended Additional (includes most stacked marks)

    return false;
}

Syllable SyllableParser::parse(const std::string& raw) {
    Syllable s;
    std::u32string input = to_u32(raw);
    if (input.empty()) return s;

    size_t i = 0;
    size_t n = input.size();

    // 1. Find Initial (Longest match for known Vietnamese initials)
    // Try longest match (3, 2, 1 chars)
    for (size_t len = 3; len >= 1; --len) {
        if (i + len <= n) {
            std::string prefix = from_u32(input.substr(i, len));
            std::string lower_prefix = unicode::to_lower(prefix);
            if (Validator::is_valid_initial(lower_prefix)) {
                s.initial = prefix;
                i += len;
                
                break;
            }
        }
    }

    // Special Case: "q" as initial
    std::string lower_initial = unicode::to_lower(s.initial);
    if (lower_initial == "qu") {
        s.initial = s.initial.substr(0, s.initial.size() - 1); // "q" or "Q"
        s.glide = 'u';
    } else if (lower_initial == "gi" && n > i && is_vowel(input[i])) {
        // "gi" + vowel (e.g. gia): initial is "g", vowel is "ia"? 
        // No, Vietnamese tradition: initial "gi", vowel is the rest. 
        // BUT if it's just "gì", then initial "gi", vowel "i".
        // Let's keep "gi" as initial for now as per ALL_INITIALS.
    }

    // 2. Glide
    if (i < n && !s.glide.has_value()) {
        char32_t c = input[i];
        char32_t lower_c = unicode::to_lower(c);
        if ((lower_c == 'o' || lower_c == 'u') && i + 1 < n && is_vowel(input[i+1])) { // o, u
            // Special Case: 'ua' and 'uô' are diphthong nuclei, not glide+nucleus
            char32_t next_stripped = unicode::strip_tone(input[i+1]);
            if (lower_c == 'u' && (next_stripped == 'a' || next_stripped == U'ô')) { // u + (a, ô)
                // stay in nucleus
            } else {
                s.glide = (char)lower_c;
                i++;
            }
        }
    }

    // 3. Vowel Nucleus
    size_t vowel_start = i;
    while (i < n && is_vowel(input[i])) {
        i++;
    }
    if (i > vowel_start) {
        s.vowel = from_u32(input.substr(vowel_start, i - vowel_start));
    }

    // 4. Final
    if (i < n) {
        s.final_c = from_u32(input.substr(i));
    }

    return s;
}

} // namespace lotus_engine
