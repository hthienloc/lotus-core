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
    } else if (lower_initial == "gi") {
        // Vietnamese rule for 'gi':
        // 1. If followed by another vowel (e.g., 'gia', 'giáo'), 'gi' is the initial.
        // 2. If followed by nothing or a consonant (e.g., 'gì', 'gin'), 'g' is initial and 'i' is vowel.
        bool followed_by_vowel = (i < n && is_vowel(input[i]));
        if (!followed_by_vowel) {
            s.initial = s.initial.substr(0, s.initial.size() - 1); // "g"
            i--; // Put 'i' back to be parsed as the vowel nucleus
        }
    }

    // 2. Glide
    if (i < n && !s.glide.has_value()) {
        char32_t c = input[i];
        char32_t lower_c = unicode::to_lower(c);
        if (i + 1 < n && is_vowel(input[i+1])) {
            char32_t next_v = unicode::to_lower(unicode::strip_tone(input[i+1]));
            bool should_be_glide = false;
            
            if (lower_c == 'o') {
                // 'o' is glide only in: oa, oe, oă
                if (next_v == 'a' || next_v == 'e' || next_v == U'ă') {
                    should_be_glide = true;
                }
            } else if (lower_c == 'u') {
                // 'u' is glide only in: uâ, uê, uơ, uy
                // Note: 'ua', 'uô', 'ui', 'ưu' are nucleus diphthongs (except for 'q' which is handled)
                if (next_v == U'â' || next_v == U'ê' || next_v == U'ơ' || next_v == 'y') {
                    should_be_glide = true;
                }
            }

            if (should_be_glide) {
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
