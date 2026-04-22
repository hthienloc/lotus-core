#include "lotus_engine/parser.h"
#include "lotus_engine/validator.h"
#include "lotus_engine/unicode.h"
#include <cassert>
#include <iostream>
#include <vector>

using namespace lotus_engine;

void test_rhymes_exhaustive() {
    SyllableParser p;
    // Standard Vietnamese
    assert(Validator::is_valid(p.parse(U"nghiêng")));
    assert(Validator::is_valid(p.parse(U"toán")));
    assert(Validator::is_valid(p.parse(U"oach")));
    assert(Validator::is_valid(p.parse(U"uynh")));
    assert(Validator::is_valid(p.parse(U"uych")));
    assert(Validator::is_valid(p.parse(U"uơ")));
    assert(Validator::is_valid(p.parse(U"uya")));
    assert(Validator::is_valid(p.parse(U"uây")));
    assert(Validator::is_valid(p.parse(U"iêu")));
    assert(Validator::is_valid(p.parse(U"hươu")));

    Syllable s_check = p.parse(U"ngoèo");
    std::cout << "DEBUG ngoèo: initial='" << unicode::to_utf8(s_check.initial) 
              << "', glide=" << (s_check.glide.has_value() ? (char)s_check.glide.value() : ' ') 
              << ", vowel='" << unicode::to_utf8(s_check.vowel) << "', final='" 
              << unicode::to_utf8(s_check.final_c) << "'" << std::endl;
    assert(Validator::is_valid(s_check));

    // Diphthong vs Coda rules
    assert(Validator::is_valid(p.parse(U"kia")));
    assert(Validator::is_valid(p.parse(U"kiên")));
    assert(!Validator::is_valid(p.parse(U"kiê")));
    assert(!Validator::is_valid(p.parse(U"kian")));

    std::cout << "test_rhymes_exhaustive PASSED" << std::endl;
}

void test_syllable_parts() {
    SyllableParser p;
    auto s = p.parse(U"nghiêng");
    assert(s.initial == U"ngh");
    assert(s.vowel == U"iê");
    assert(s.final_c == U"ng");

    auto s2 = p.parse(U"toán");
    assert(s2.initial == U"t");
    assert(s2.glide.has_value() && s2.glide.value() == 'o');
    assert(s2.vowel == U"an");

    auto s3 = p.parse(U"khuyên");
    assert(s3.initial == U"kh");
    assert(s3.glide.has_value() && s3.glide.value() == 'u');
    assert(s3.vowel == U"yên");
    
    std::cout << "test_syllable_parts PASSED" << std::endl;
}

void test_orthography_rules() {
    SyllableParser p;
    // Standard Vietnamese
    assert(Validator::is_valid(p.parse(U"quân")));
    assert(Validator::is_valid(p.parse(U"quả")));
    assert(!Validator::is_valid(p.parse(U"qoa")));

    // Spelling rules: ng/ngh
    assert(Validator::is_valid(p.parse(U"nghê")));
    assert(Validator::is_valid(p.parse(U"ngủ")));
    assert(!Validator::is_valid(p.parse(U"ngê")));
    assert(Validator::is_valid(p.parse(U"ngheo")));
    assert(!Validator::is_valid(p.parse(U"ngeo")));

    assert(Validator::is_valid(p.parse(U"ghe")));
    assert(!Validator::is_valid(p.parse(U"ge")));
    assert(Validator::is_valid(p.parse(U"go")));
    assert(!Validator::is_valid(p.parse(U"gho")));

    std::cout << "test_orthography_rules PASSED" << std::endl;
}
