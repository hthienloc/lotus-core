#include <iostream>
#include <cassert>
#include "lotus_engine/parser.h"

using namespace lotus_engine;

void test_parser_basic() {
    auto s = SyllableParser::parse("nghieng");
    assert(s.initial == "ngh");
    assert(s.vowel == "ie");
    assert(s.final_c == "ng");

    auto s2 = SyllableParser::parse("hoa");
    assert(s2.initial == "h");
    assert(s2.glide.has_value() && s2.glide.value() == 'o');
    assert(s2.vowel == "a");
    
    std::cout << "test_parser_basic PASSED" << std::endl;
}

void test_parser_special() {
    auto s = SyllableParser::parse("qua");
    // New logic: q is initial, u is glide
    assert(s.initial == "q");
    assert(s.glide.has_value() && s.glide.value() == 'u');
    assert(s.vowel == "a");

    auto s2 = SyllableParser::parse("giai");
    assert(s2.initial == "gi");
    assert(s2.vowel == "ai");

    std::cout << "test_parser_special PASSED" << std::endl;
}
