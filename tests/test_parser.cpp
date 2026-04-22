#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

void test_parser_basic() {
    auto s = SyllableParser::parse(U"nghieng");
    assert(s.initial == U"ngh");
    assert(s.vowel == U"ie");
    assert(s.final_c == U"ng");

    auto s2 = SyllableParser::parse(U"hoa");
    assert(s2.initial == U"h");
    assert(s2.glide.has_value() && s2.glide.value() == 'o');
    assert(s2.vowel == U"a");

    std::cout << "test_parser_basic PASSED" << std::endl;
}

void test_parser_special() {
    auto s = SyllableParser::parse(U"qua");
    // New logic: q is initial, u is glide
    assert(s.initial == U"q");
    assert(s.glide.has_value() && s.glide.value() == 'u');
    assert(s.vowel == U"a");

    auto s2 = SyllableParser::parse(U"giai");
    assert(s2.initial == U"gi");
    assert(s2.vowel == U"ai");

    std::cout << "test_parser_special PASSED" << std::endl;
}
