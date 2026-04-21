#include "lotus_engine/types.h"
#include "lotus_engine/unicode.h"
#include <map>

namespace lotus_engine {

static const std::string TONE_MARKS[] = {
    "",         // NONE
    "\xCC\x81", // ACUTE (U+0301)
    "\xCC\x80", // GRAVE (U+0300)
    "\xCC\x89", // HOOK (U+0309)
    "\xCC\x83", // TILDE (U+0303)
    "\xCC\xA3"  // DOT (U+0323)
};

std::string Syllable::to_string(ToneStyle style) const {
    if (is_empty()) return "";

    std::string res = initial;
    std::string vowel_part = vowel;
    std::optional<char> current_glide = glide;

    if (tone != Tone::NONE && !vowel_part.empty()) {
        std::u32string v32 = unicode::to_utf32(vowel_part);
        
        // 1. Check for SPECIAL Case: (oa, oe, uy) without Coda
        // OLD style puts mark on GLIDE (hòa). NEW style puts on NUCLEUS (hoà).
        if (current_glide.has_value() && final_c.empty() && v32.size() == 1) {
            char g = (char)unicode::to_lower((char32_t)current_glide.value());
            if (style == ToneStyle::OLD && (g == 'o' || g == 'u')) {
                // Mark on glide
                std::string marked_glide = unicode::to_utf8((char32_t)current_glide.value());
                marked_glide += TONE_MARKS[static_cast<int>(tone)];
                res += unicode::normalize_nfc(marked_glide);
                res += vowel;
                res += final_c;
                return res;
            }
        } // missing closing brace for if (current_glide.has_value())

        // 2. Default Placement (Nucleus-centric)

        size_t target_idx = 0;
        if (v32.size() == 1) {
            target_idx = 0;
        } else if (v32.size() == 2) {
            char32_t v0 = v32[0];
            char32_t v1 = v32[1];
            // Stable diphthongs (centering) always put tone on 2nd char: iê, uô, ươ, yê, uơ
            if ((v0 == 'i' && v1 == U'ê') || (v0 == 'u' && v1 == U'ô') || 
                (v0 == U'ư' && v1 == U'ơ') || (v0 == 'y' && v1 == U'ê') ||
                (v0 == 'u' && v1 == U'ơ')) {
                target_idx = 1;
            } else {
                bool has_final = !final_c.empty();
                if (has_final) target_idx = 1; // e.g., "toán"
                else target_idx = 0;           // e.g., "tóa"
            }
        } else if (v32.size() == 3) {
            target_idx = 1; // e.g., "khuỷu"
        }
        
        std::string new_vowel = "";
        for (size_t i = 0; i < v32.size(); ++i) {
            if (i == target_idx) {
                std::string target_char = unicode::to_utf8(v32[i]);
                target_char += TONE_MARKS[static_cast<int>(tone)];
                new_vowel += target_char;
            } else {
                new_vowel += unicode::to_utf8(v32[i]);
            }
        }
        
        res += (current_glide.has_value() ? std::string(1, current_glide.value()) : "");
        res += new_vowel;
    } else {
        res += (current_glide.has_value() ? std::string(1, current_glide.value()) : "");
        res += vowel;
    }
    
    res += final_c;
    return unicode::normalize_nfc(res);
}

} // namespace lotus_engine
