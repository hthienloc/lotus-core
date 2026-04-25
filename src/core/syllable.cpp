/**
 * @file syllable.cpp
 * @brief Implementation of Vietnamese syllable structure and transformations.
 */

#include "lotus_core/types.h"
#include "lotus_core/unicode.h"

#include <map>

using namespace lotus_core;

namespace lotus_core {

// ============================================================================
// [ Constants ]
// ============================================================================

static const std::u32string TONE_MARKS_U32[] = {
    U"",        // NONE
    U"\u0301",  // ACUTE
    U"\u0300",  // GRAVE
    U"\u0309",  // HOOK
    U"\u0303",  // TILDE
    U"\u0323"   // DOT
};

// ============================================================================
// [ Syllable Implementation ]
// ============================================================================

/**
 * @brief Converts the syllable structure to a UTF-8 string with Vietnamese tone placement.
 * @param style The tone placement style (Modern vs. Old).
 * @return Standardized UTF-8 Vietnamese syllable string.
 */
std::string Syllable::to_string(ToneStyle style) const {
    if (is_empty())
        return "";

    std::u32string res = initial;

    if (tone != Tone::NONE) {
        bool tone_placed = false;

        // 1. OLD Style Check (oa, oe, uy)
        if (glide.has_value() && final_c.empty() && vowel.size() == 1) {
            char32_t g = unicode::to_lower(glide.value());
            if (style == ToneStyle::OLD && (g == 'o' || g == 'u')) {
                res += glide.value();
                res += TONE_MARKS_U32[static_cast<int>(tone)];
                res += vowel;
                tone_placed = true;
            }
        }

        if (!tone_placed) {
            // Handle glide if not already handled
            if (glide.has_value())
                res += glide.value();

            if (!vowel.empty()) {
                size_t target_idx = 0;
                if (vowel.size() == 2) {
                    char32_t v0 = vowel[0];
                    char32_t v1 = vowel[1];
                    // Centering diphthongs or coda presence
                    if ((v0 == 'i' && (v1 == U'ê' || v1 == 'e')) ||
                        (v0 == 'u' && (v1 == U'ô' || v1 == 'o' || v1 == U'ơ')) ||
                        (v0 == U'ư' && (v1 == U'ơ' || v1 == 'o')) ||
                        (v0 == 'y' && (v1 == U'ê' || v1 == 'e')) || !final_c.empty()) {
                        target_idx = 1;
                    }
                } else if (vowel.size() == 3) {
                    target_idx = 1;
                }

                for (size_t i = 0; i < vowel.size(); ++i) {
                    res += vowel[i];
                    if (i == target_idx)
                        res += TONE_MARKS_U32[static_cast<int>(tone)];
                }
            } else if (glide.has_value()) {
                // Tone on glide if no vowel
                res.pop_back();  // Remove glide added above
                res += glide.value();
                res += TONE_MARKS_U32[static_cast<int>(tone)];
            }
            tone_placed = true;
        }
    } else {
        if (glide.has_value())
            res += glide.value();
        res += vowel;
    }

    res += final_c;
    return unicode::normalize_nfc(unicode::to_utf8(res));
}

/**
 * @brief Removes the last logical component of the syllable.
 *
 * Handles backspacing logic by stripping components in reverse linguistic order:
 * Final -> Vowel -> Glide -> Initial.
 */
void Syllable::remove_last_char() {
    if (!final_c.empty()) {
        final_c.pop_back();
    } else if (!vowel.empty()) {
        vowel.pop_back();
        if (vowel.empty()) {
            if (glide.has_value()) {
                vowel = {glide.value()};
                glide = std::nullopt;
            } else {
                tone = Tone::NONE;
            }
        }
    } else if (glide.has_value()) {
        glide = std::nullopt;
        tone = Tone::NONE;
    } else if (!initial.empty()) {
        initial.pop_back();
        tone = Tone::NONE;
    }
}

/**
 * @brief Deconstructs the syllable back into a sequence of input keys.
 * @param method The input method (Telex/VNI) to assume for key mapping.
 * @return A vector of character keys that would produce this syllable.
 */
std::vector<char32_t> Syllable::to_keys(InputMethod method) const {
    std::vector<char32_t> keys;

    // 1. Initial
    for (char32_t c : initial) {
        if (c == U'đ' || c == U'Đ') {
            keys.push_back((c == U'Đ') ? 'D' : 'd');
            keys.push_back((method == InputMethod::TELEX) ? (char32_t)((c == U'Đ') ? 'D' : 'd')
                                                          : (char32_t)'9');
        } else
            keys.push_back(c);
    }

    // 2. Glide
    if (glide.has_value())
        keys.push_back(glide.value());

    // 3. Vowel
    for (char32_t c : vowel) {
        char32_t l = unicode::to_lower(c);
        char32_t base = unicode::strip_tone(l);
        if (base == U'â' || base == U'ê' || base == U'ô') {
            char32_t bk = (base == U'â') ? 'a' : (base == U'ê') ? 'e' : 'o';
            char32_t mk = (method == InputMethod::TELEX) ? bk : '6';
            keys.push_back((unicode::to_upper(c) == c) ? unicode::to_upper(bk) : bk);
            keys.push_back((unicode::to_upper(c) == c) ? unicode::to_upper(mk) : mk);
        } else if (base == U'ă' || base == U'ư' || base == U'ơ') {
            char32_t bk = (base == U'ă') ? 'a' : (base == U'ư') ? 'u' : 'o';
            char32_t mk = (method == InputMethod::TELEX) ? 'w' : (base == U'ă' ? '8' : '7');
            keys.push_back((unicode::to_upper(c) == c) ? unicode::to_upper(bk) : bk);
            keys.push_back((unicode::to_upper(c) == c) ? unicode::to_upper(mk) : mk);
        } else {
            keys.push_back(c);
        }
    }

    // 4. Final
    for (char32_t c : final_c)
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

}  // namespace lotus_core
