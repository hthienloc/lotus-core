/**
 * @file test_syllable.cpp
 * @brief Unit tests for the Syllable structure.
 */

#include "lotus_engine/types.h"
#include "lotus_engine/parser.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

// ============================================================================
// [ Syllable Tests ]
// ============================================================================

/**
 * @brief Tests basic syllable-to-string conversion without complex tones.
 */
void test_syllable_to_string_basic() {
    Syllable s;
    s.initial = U"h";
    s.vowel = U"a";
    assert(s.to_string() == "ha");
    std::cout << "  \033[1;32m[PASS]\033[0m Syllable basic to_string" << std::endl;
}

/**
 * @brief Verifies the 'is_empty' logic for various syllable states.
 */
void test_syllable_is_empty() {
    Syllable s;
    assert(s.is_empty());
    s.initial = U"b";
    assert(!s.is_empty());
    std::cout << "  \033[1;32m[PASS]\033[0m Syllable is_empty check" << std::endl;
}

/**
 * @brief Tests out-of-order vowel typing (Bamboo-style reordering) and casing.
 */
void test_vowel_reordering() {
    // 1. u + ơ + ư -> ươu
    Syllable s1 = lotus_engine::SyllableParser::parse(U"uơư");
    assert(!s1.glide.has_value());
    assert(s1.vowel == U"ươu");

    // 2. h + i + o + a -> hoai
    Syllable s2 = lotus_engine::SyllableParser::parse(U"hioa");
    assert(s2.initial == U"h");
    assert(s2.glide.has_value() && s2.glide.value() == 'o');
    assert(s2.vowel == U"ai");

    // 3. k + h + y + u + a -> khuya
    Syllable s3 = lotus_engine::SyllableParser::parse(U"khyua");
    assert(s3.initial == U"kh");
    assert(s3.glide.has_value() && s3.glide.value() == 'u');
    assert(s3.vowel == U"ya");
    
    // 4. t + u + i + ê -> tiêu (iêu)
    Syllable s4 = lotus_engine::SyllableParser::parse(U"tuiê");
    assert(s4.initial == U"t");
    assert(!s4.glide.has_value());
    assert(s4.vowel == U"iêu");

    // 5. Casing check: U + Ơ + Ư -> ƯƠU
    Syllable s5 = lotus_engine::SyllableParser::parse(U"UƠƯ");
    assert(!s5.glide.has_value());
    assert(s5.vowel == U"ƯƠU");

    // 6. Casing check: h + I + o + a -> hoI
    Syllable s6 = lotus_engine::SyllableParser::parse(U"hIoa");
    assert(s6.glide.has_value() && s6.glide.value() == 'o');
    assert(s6.vowel == U"aI");

    std::cout << "  \033[1;32m[PASS]\033[0m Syllable vowel reordering (Bamboo-style)" << std::endl;
}
