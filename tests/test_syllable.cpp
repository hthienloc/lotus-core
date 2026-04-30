/**
 * @file test_syllable.cpp
 * @brief Unit tests for the Syllable structure.
 */

#include "lotus_core/types.h"
#include "lotus_core/parser.h"

#include <cassert>
#include <iostream>

using namespace lotus_core;

// ============================================================================
// [ Syllable Tests ]
// ============================================================================

/**
 * @brief Tests basic syllable-to-string conversion without complex tones.
 */
void test_syllable_to_string_basic() {
    Syllable s;
    s.initial = std::u32string_view(U"h");
    s.vowel = std::u32string_view(U"a");
    assert(s.to_string() == "ha");
    std::cout << "  \033[1;32m[PASS]\033[0m Syllable basic to_string" << std::endl;
}

/**
 * @brief Verifies the 'is_empty' logic for various syllable states.
 */
void test_syllable_is_empty() {
    Syllable s;
    assert(s.is_empty());
    s.initial = std::u32string_view(U"b");
    assert(!s.is_empty());
    std::cout << "  \033[1;32m[PASS]\033[0m Syllable is_empty check" << std::endl;
}

/**
 * @brief Tests out-of-order vowel typing (Bamboo-style reordering) and casing.
 */
void test_vowel_reordering() {
    // 1. u + ơ + ư -> ươu
    Syllable s1 = SyllableParser::parse(U"uơư");
    assert(!s1.glide.has_value());
    assert(s1.vowel == U"ươu");

    // 2. h + i + o + a -> hoai
    Syllable s2 = SyllableParser::parse(U"hioa");
    assert(s2.initial == U"h");
    assert(s2.glide.has_value() && s2.glide.value() == 'o');
    assert(s2.vowel == U"ai");

    // 3. k + h + y + u + a -> khuya
    Syllable s3 = SyllableParser::parse(U"khyua");
    assert(s3.initial == U"kh");
    assert(s3.glide.has_value() && s3.glide.value() == 'u');
    assert(s3.vowel == U"ya");
    
    // 4. t + u + i + ê -> tiêu (iêu)
    Syllable s4 = SyllableParser::parse(U"tuiê");
    assert(s4.initial == U"t");
    assert(!s4.glide.has_value());
    assert(s4.vowel == U"iêu");

    // 5. Casing check: U + Ơ + Ư -> ƯƠU
    Syllable s5 = SyllableParser::parse(U"UƠƯ");
    assert(!s5.glide.has_value());
    assert(s5.vowel == U"ƯƠU");

    // 6. Casing check: h + I + o + a -> hoI
    Syllable s6 = SyllableParser::parse(U"hIoa");
    assert(s6.glide.has_value() && s6.glide.value() == 'o');
    assert(s6.vowel == U"aI");

    std::cout << "  \033[1;32m[PASS]\033[0m Syllable vowel reordering (Bamboo-style)" << std::endl;
}

/**
 * @brief Tests reposition_tone directly through to_char_states and to_string.
 */
void test_reposition_tone() {
    // 1. Diphthong (iê): Tone on second vowel
    Syllable s1 = SyllableParser::parse(U"tiếng");
    s1.tone = Tone::ACUTE; // tiengs -> tiếng
    CharStateArray c1 = s1.to_char_states(ToneStyle::NEW);
    assert(c1[1].to_unicode() == U'i'); // no tone
    assert(c1[2].to_unicode() == U'ế'); // tone on ê

    // 2. Diphthong (oa) without final: Tone on first vowel (NEW style)
    Syllable s2 = SyllableParser::parse(U"hoa");
    s2.tone = Tone::GRAVE; // hoaf -> hoà
    CharStateArray c2_new = s2.to_char_states(ToneStyle::NEW);
    assert(c2_new[1].to_unicode() == U'o'); // no tone
    assert(c2_new[2].to_unicode() == U'à'); // tone on a
    
    // Tone on glide (OLD style)
    CharStateArray c2_old = s2.to_char_states(ToneStyle::OLD);
    assert(c2_old[1].to_unicode() == U'ò'); // tone on o
    assert(c2_old[2].to_unicode() == U'a'); // no tone

    // 3. Triphthong (iêu): Tone on middle nucleus
    Syllable s3 = SyllableParser::parse(U"tiêu");
    s3.tone = Tone::HOOK; // tieur -> tiểu
    CharStateArray c3 = s3.to_char_states(ToneStyle::NEW);
    assert(c3[1].to_unicode() == U'i'); // no tone
    assert(c3[2].to_unicode() == U'ể'); // tone on ê
    assert(c3[3].to_unicode() == U'u'); // no tone

    std::cout << "  \033[1;32m[PASS]\033[0m Syllable tone repositioning" << std::endl;
}
