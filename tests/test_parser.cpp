/**
 * @file test_parser.cpp
 * @brief Unit tests for the Vietnamese syllable parser.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Tests basic syllable parsing including initial, vowel, and final components.
 */
void test_parser_basic() {
    auto s = SyllableParser::parse(U"nghieng");
    assert(s.initial == U"ngh");
    assert(s.vowel == U"ie");
    assert(s.final_c == U"ng");

    auto s2 = SyllableParser::parse(U"hoa");
    assert(s2.initial == U"h");
    assert(s2.glide.has_value() && s2.glide.value() == 'o');
    assert(s2.vowel == U"a");

    std::cout << "  \033[1;32m[PASS]\033[0m Basic syllable parsing" << std::endl;
}

/**
 * @brief Tests parsing of special cases like 'qu-' and 'gi-'.
 */
void test_parser_special() {
    auto s = SyllableParser::parse(U"qua");
    // New logic: q is initial, u is glide
    assert(s.initial == U"q");
    assert(s.glide.has_value() && s.glide.value() == 'u');
    assert(s.vowel == U"a");

    auto s2 = SyllableParser::parse(U"giai");
    assert(s2.initial == U"gi");
    assert(s2.vowel == U"ai");

    std::cout << "  \033[1;32m[PASS]\033[0m Special syllable cases (qu, gi)" << std::endl;
}
