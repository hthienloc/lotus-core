#include "lotus_engine/validator.h"

#include "lotus_engine/unicode.h"

#include <algorithm>
#include <array>
#include <string_view>

namespace lotus_engine {

namespace {

constexpr std::array<std::string_view, 28> VALID_INITIALS = {
    "b", "c", "d", "đ",  "g",  "h",  "k",  "l",  "m",  "n",  "p",  "q",  "r",  "s",
    "t", "v", "x", "ch", "gh", "gi", "kh", "ng", "nh", "ph", "qu", "th", "tr", "ngh"};

constexpr std::array<std::string_view, 2> VALID_GLIDES = {"o", "u"};

constexpr std::array<std::string_view, 40> VALID_NUCLEI = {
    // Monophthongs
    "a", "ă", "â", "e", "ê", "i", "o", "ô", "ơ", "u", "ư", "y",
    // Centering Diphthongs
    "ia", "iê", "ua", "uô", "ưa", "ươ", "yê", "ya",
    // Closing Diphthongs
    "ai", "ao", "au", "âu", "ay", "ây", "eo", "êu", "iu", "oi", "ôi", "ơi", "ui", "ưi", "ưu",
    // Triphthongs & Long Vowels
    "iêu", "yêu", "uôi", "ươi", "ươu"};

constexpr std::array<std::string_view, 12> VALID_FINALS = {"c", "ch", "m", "n", "ng", "nh",
                                                           "p", "t",  "i", "y", "o",  "u"};

}  // namespace

bool Validator::is_valid_initial(std::string_view initial) {
    return std::find(VALID_INITIALS.begin(), VALID_INITIALS.end(), initial) != VALID_INITIALS.end();
}

bool Validator::is_valid(const Syllable& syllable) {
    if (syllable.vowel.empty() && !syllable.glide.has_value()) {
        if (syllable.initial.empty())
            return false;
        return is_valid_initial(unicode::to_lower(syllable.initial));
    }

    // 1. Component Set Checks
    if (!syllable.initial.empty()) {
        auto lower_i = unicode::to_lower(syllable.initial);
        if (std::find(VALID_INITIALS.begin(), VALID_INITIALS.end(), lower_i) ==
            VALID_INITIALS.end())
            return false;
    }

    if (syllable.glide.has_value()) {
        std::string g(1, (char)unicode::to_lower((char32_t)syllable.glide.value()));
        if (std::find(VALID_GLIDES.begin(), VALID_GLIDES.end(), g) == VALID_GLIDES.end())
            return false;
    }

    if (!syllable.vowel.empty()) {
        std::string lower_v = syllable.vowel;
        // Strip tones for nucleus pattern matching
        std::u32string v32_raw = unicode::to_utf32(lower_v);
        std::string stripped_v = "";
        for (char32_t cp : v32_raw) {
            char32_t toned_cp = unicode::strip_tone(cp);
            // Skip combining marks (0x0300 range)
            if (toned_cp >= 0x0300 && toned_cp <= 0x036F)
                continue;
            stripped_v += unicode::to_utf8(toned_cp);
        }
        if (std::find(VALID_NUCLEI.begin(), VALID_NUCLEI.end(), stripped_v) == VALID_NUCLEI.end())
            return false;
    }

    if (!syllable.final_c.empty()) {
        std::string lower_f = syllable.final_c;
        if (std::find(VALID_FINALS.begin(), VALID_FINALS.end(), lower_f) == VALID_FINALS.end())
            return false;
    }

    // 2. Orthographic Rules

    // Q Rule: q always followed by glide u
    if (unicode::to_lower(syllable.initial) == "q") {
        if (!syllable.glide.has_value() ||
            unicode::to_lower((char32_t)syllable.glide.value()) != 'u')
            return false;
    }

    // Front Vowel Affinity (k, gh, ngh)
    std::u32string v32 = unicode::to_utf32(syllable.vowel);
    char32_t nucleus_start = v32.empty() ? 0 : v32[0];

    // Affinity char is the char IMMEDIATELY following the initial consonant
    char32_t affinity_char = nucleus_start;
    if (syllable.glide.has_value())
        affinity_char = unicode::to_lower((char32_t)syllable.glide.value());

    bool is_front = (affinity_char == 'e' || affinity_char == U'ê' || affinity_char == 'i' ||
                     affinity_char == 'y');
    bool is_front_no_y = (affinity_char == 'e' || affinity_char == U'ê' || affinity_char == 'i');

    std::string lower_init = unicode::to_lower(syllable.initial);

    if (lower_init == "k" && !is_front)
        return false;
    if (lower_init == "c" && is_front)
        return false;
    if (lower_init == "gh" && !is_front_no_y)
        return false;
    if (lower_init == "g" && is_front_no_y)
        return false;
    if (lower_init == "ngh" && !is_front_no_y)
        return false;
    if (lower_init == "ng" && is_front_no_y)
        return false;

    // Diphthong vs Open/Closed Syllable (ia/iê, ua/uô, ưa/ươ)
    bool has_coda = !syllable.final_c.empty();
    if (syllable.vowel == "ia" && has_coda)
        return false;
    if (syllable.vowel == "iê" && !has_coda)
        return false;
    if (syllable.vowel == "ua" && has_coda)
        return false;
    if (syllable.vowel == "uô" && !has_coda)
        return false;
    if (syllable.vowel == "ưa" && has_coda)
        return false;
    if (syllable.vowel == "ươ" && !has_coda)
        return false;

    // Final Coda Restrictions (ch, nh) - Follow the NUCLEUS
    if (syllable.final_c == "ch" || syllable.final_c == "nh") {
        if (nucleus_start != 'a' && nucleus_start != U'ê' && nucleus_start != 'i' &&
            nucleus_start != 'y')
            return false;
    }

    if (syllable.final_c == "ng") {
        // as per test expectations: should use -nh/ch for front vowels e/ê
        if (nucleus_start == 'e' || nucleus_start == U'ê')
            return false;
    }

    return true;
}

}  // namespace lotus_engine
