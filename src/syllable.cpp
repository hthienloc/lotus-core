#include "lotus_engine/types.h"
#include "lotus_engine/unicode.h"

#include <map>

namespace lotus_engine {

static const std::string TONE_MARKS[] = {
    "",  // NONE
    "́",  // ACUTE (U+0301)
    "̀",  // GRAVE (U+0300)
    "̉",  // HOOK (U+0309)
    "̃",  // TILDE (U+0303)
    "̣"   // DOT (U+0323)
};

std::string Syllable::to_string(ToneStyle style) const {
    if (is_empty())
        return "";

    std::string res = initial;
    std::u32string v32 = unicode::to_utf32(vowel);
    std::optional<char> current_glide = glide;

    if (tone != Tone::NONE) {
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
        }

        // Special Case: Handle Empty Nucleus by putting tone on Glide
        if (v32.empty() && current_glide.has_value()) {
            std::string marked_glide = unicode::to_utf8((char32_t)current_glide.value());
            marked_glide += TONE_MARKS[static_cast<int>(tone)];
            res += unicode::normalize_nfc(marked_glide);
            res += final_c;
            return res;
        }

        if (!v32.empty()) {
            // Default Placement (Nucleus-centric)
            size_t target_idx = 0;
            if (v32.size() == 2) {
                char32_t v0 = v32[0];
                char32_t v1 = v32[1];
                // Centering diphthongs: iê, uô, ươ, yê, uơ (and ASCII equivalents during typing)
                if ((v0 == 'i' && (v1 == U'ê' || v1 == 'e')) ||
                    (v0 == 'u' && (v1 == U'ô' || v1 == 'o' || v1 == U'ơ')) ||
                    (v0 == U'ư' && (v1 == U'ơ' || v1 == 'o')) ||
                    (v0 == 'y' && (v1 == U'ê' || v1 == 'e'))) {
                    target_idx = 1;
                } else if (!final_c.empty()) {
                    target_idx = 1;  // e.g., "toán"
                }
            } else if (v32.size() == 3) {
                target_idx = 1;
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
        }
    } else {
        res += (current_glide.has_value() ? std::string(1, current_glide.value()) : "");
        res += vowel;
    }

    res += final_c;
    return unicode::normalize_nfc(res);
}

void Syllable::remove_last_char() {
    if (!final_c.empty()) {
        std::u32string f32 = unicode::to_utf32(final_c);
        f32.pop_back();
        final_c = unicode::to_utf8(f32);
    } else if (!vowel.empty()) {
        std::u32string v32 = unicode::to_utf32(vowel);
        v32.pop_back();
        vowel = unicode::to_utf8(v32);
    } else if (glide.has_value()) {
        glide = std::nullopt;
    } else if (!initial.empty()) {
        std::u32string i32 = unicode::to_utf32(initial);
        i32.pop_back();
        initial = unicode::to_utf8(i32);
    }
}

std::vector<char32_t> Syllable::to_keys(InputMethod method) const {
    std::vector<char32_t> keys;

    // 1. Initial
    std::u32string i32 = unicode::to_utf32(initial);
    for (char32_t c : i32) {
        if (method == InputMethod::TELEX && c == U'đ') {
            keys.push_back('d');
            keys.push_back('d');
        } else if (method == InputMethod::VNI && c == U'đ') {
            keys.push_back('d');
            keys.push_back('9');
        } else
            keys.push_back(c);
    }

    // 2. Glide
    if (glide.has_value())
        keys.push_back((char32_t)glide.value());

    // 3. Vowel
    std::u32string v32 = unicode::to_utf32(vowel);
    for (char32_t c : v32) {
        if (method == InputMethod::TELEX) {
            if (c == U'â') {
                keys.push_back('a');
                keys.push_back('a');
            } else if (c == U'ê') {
                keys.push_back('e');
                keys.push_back('e');
            } else if (c == U'ô') {
                keys.push_back('o');
                keys.push_back('o');
            } else if (c == U'ă') {
                keys.push_back('a');
                keys.push_back('w');
            } else if (c == U'ư') {
                keys.push_back('u');
                keys.push_back('w');
            } else if (c == U'ơ') {
                keys.push_back('o');
                keys.push_back('w');
            } else
                keys.push_back(c);
        } else {
            if (c == U'â') {
                keys.push_back('a');
                keys.push_back('6');
            } else if (c == U'ê') {
                keys.push_back('e');
                keys.push_back('6');
            } else if (c == U'ô') {
                keys.push_back('o');
                keys.push_back('6');
            } else if (c == U'ă') {
                keys.push_back('a');
                keys.push_back('8');
            } else if (c == U'ư') {
                keys.push_back('u');
                keys.push_back('7');
            } else if (c == U'ơ') {
                keys.push_back('o');
                keys.push_back('7');
            } else
                keys.push_back(c);
        }
    }

    // 4. Final
    std::u32string f32 = unicode::to_utf32(final_c);
    for (char32_t c : f32)
        keys.push_back(c);

    // 5. Tone
    if (tone != Tone::NONE) {
        if (method == InputMethod::TELEX) {
            static const char tone_keys[] = "\0sfrxj";
            keys.push_back((char32_t)tone_keys[static_cast<int>(tone)]);
        } else {
            static const char tone_keys[] = "\012345";
            keys.push_back((char32_t)tone_keys[static_cast<int>(tone)]);
        }
    }
    return keys;
}

}  // namespace lotus_engine
