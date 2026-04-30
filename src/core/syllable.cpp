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
// [ CharState Implementation ]
// ============================================================================

CharState CharState::from_unicode(char32_t cp) {
    CharState state;
    state.upper = (unicode::to_upper(cp) == cp && cp >= 'A' && unicode::to_lower(cp) != cp);
    char32_t lower_cp = unicode::to_lower(cp);
    
    state.tone = unicode::get_tone(lower_cp);
    char32_t stripped = unicode::strip_tone(lower_cp);

    if (stripped == U'â' || stripped == U'ê' || stripped == U'ô') {
        state.base = (stripped == U'â') ? 'a' : ((stripped == U'ê') ? 'e' : 'o');
        state.modifier = Modifier::HAT;
    } else if (stripped == U'ơ' || stripped == U'ư') {
        state.base = (stripped == U'ơ') ? 'o' : 'u';
        state.modifier = Modifier::HOOK;
    } else if (stripped == U'ă') {
        state.base = 'a';
        state.modifier = Modifier::BREVE;
    } else if (stripped == U'đ') {
        state.base = 'd';
        state.modifier = Modifier::BAR;
    } else {
        state.base = stripped;
        state.modifier = Modifier::NONE;
    }
    
    return state;
}

char32_t CharState::to_unicode() const {
    char32_t result = base;
    
    // 1. Apply modifier
    if (modifier == Modifier::HAT) {
        if (base == 'a') result = U'â';
        else if (base == 'e') result = U'ê';
        else if (base == 'o') result = U'ô';
    } else if (modifier == Modifier::HOOK) {
        if (base == 'o') result = U'ơ';
        else if (base == 'u') result = U'ư';
    } else if (modifier == Modifier::BREVE) {
        if (base == 'a') result = U'ă';
    } else if (modifier == Modifier::BAR) {
        if (base == 'd') result = U'đ';
    }

    // 2. Apply tone
    if (tone != Tone::NONE) {
        char32_t tone_mark = TONE_MARKS_U32[static_cast<int>(tone)][0];
        if (tone_mark) {
            auto it = phonology::COMPOSITION_MAP.find({result, tone_mark});
            if (it != phonology::COMPOSITION_MAP.end()) {
                result = it->second;
            }
        }
    }

    // 3. Apply casing
    if (upper) {
        result = unicode::to_upper(result);
    }
    
    return result;
}

// ============================================================================
// [ Syllable Implementation ]
// ============================================================================

void Syllable::reposition_tone(CharStateArray& states, ToneStyle style) const {
    if (tone == Tone::NONE) return;

    size_t nucleus_start = initial.size();
    size_t nucleus_len = states.size() - nucleus_start - final_c.size();

    if (nucleus_len == 0) return; // Should not happen for a valid syllable

    // OLD Style Check (oa, oe, uy)
    if (glide.has_value() && final_c.empty() && vowel.size() == 1) {
        char32_t g = unicode::to_lower(glide.value());
        if (style == ToneStyle::OLD && (g == 'o' || g == 'u')) {
            states[nucleus_start].tone = tone; // Put tone on glide
            return;
        }
    }

    if (!vowel.empty()) {
        size_t target_idx = 0;
        size_t v_start = nucleus_start + (glide.has_value() ? 1 : 0);

        if (vowel.size() == 2) {
            char32_t v0 = unicode::to_lower(vowel[0]);
            char32_t v1 = unicode::to_lower(vowel[1]);
            // Diphthongs with 'iê', 'yê', 'uô', 'ươ': tone goes on the SECOND vowel.
            // Or if there is a coda, the tone shifts to the second vowel in diphthongs.
            if ((v0 == 'i' && (v1 == U'ê' || v1 == 'e')) ||
                (v0 == 'u' && (v1 == U'ô' || v1 == 'o' || v1 == U'ơ')) ||
                (v0 == U'ư' && (v1 == U'ơ' || v1 == 'o')) ||
                (v0 == 'y' && (v1 == U'ê' || v1 == 'e')) || !final_c.empty()) {
                target_idx = 1;
            }
        } else if (vowel.size() == 3) {
            // Triphthongs: tone goes on the MIDDLE nucleus.
            target_idx = 1;
        }

        states[v_start + target_idx].tone = tone;
    } else if (glide.has_value()) {
        // Tone on glide if no vowel
        states[nucleus_start].tone = tone;
    }
}

/**
 * @brief Decomposes the syllable into a sequence of CharStates.
 * @param style The tone placement style to use for determining where the tone goes.
 * @return CharStateArray representing the decomposed semantic units of the syllable.
 */
CharStateArray Syllable::to_char_states(ToneStyle style) const {
    CharStateArray states;
    if (is_empty()) return states;

    // 1. Initial
    for (char32_t c : initial) {
        states.push_back(CharState::from_unicode(c));
    }

    // 2. Glide
    if (glide.has_value()) {
        states.push_back(CharState::from_unicode(glide.value()));
    }

    // 3. Vowel
    for (char32_t c : vowel) {
        states.push_back(CharState::from_unicode(c));
    }

    // 4. Final Coda
    for (char32_t c : final_c) {
        states.push_back(CharState::from_unicode(c));
    }

    // 5. Reposition tone
    reposition_tone(states, style);

    return states;
}

/**
 * @brief Converts the syllable structure to a UTF-8 string with Vietnamese tone placement.
 * @param style The tone placement style (Modern vs. Old).
 * @return Standardized UTF-8 Vietnamese syllable string.
 */
std::string Syllable::to_string(ToneStyle style) const {
    if (is_empty()) return "";

    CharStateArray states = to_char_states(style);
    std::u32string res;
    
    for (size_t i = 0; i < states.size(); ++i) {
        char32_t cp = states[i].to_unicode();
        res += cp;
    }

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
                vowel.push_back(glide.value());
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
 * @return A StaticString of character keys that would produce this syllable.
 */
StaticString Syllable::to_keys(InputMethod method) const {
    StaticString keys;

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
