#include <iostream>
#include <cassert>
#include "lotus_engine/validator.h"
#include "lotus_engine/parser.h"

using namespace lotus_engine;

void test_rhymes_exhaustive() {
    SyllableParser p;
    
    // Rare / Complex Rhymes
    assert(Validator::is_valid(p.parse("oanh")));
    assert(Validator::is_valid(p.parse("oach")));
    assert(Validator::is_valid(p.parse("uynh")));
    assert(Validator::is_valid(p.parse("uych")));
    assert(Validator::is_valid(p.parse("uơ")));
    assert(Validator::is_valid(p.parse("uya")));
    assert(Validator::is_valid(p.parse("uây")));
    assert(Validator::is_valid(p.parse("iêu")));
    assert(Validator::is_valid(p.parse("hươu")));
    auto s_check = p.parse("ngoèo");
    std::cout << "DEBUG ngoèo: initial='" << s_check.initial << "', glide=" << (s_check.glide.has_value() ? std::string(1, s_check.glide.value()) : "none") 
              << ", vowel='" << s_check.vowel << "', final='" << s_check.final_c << "'" << std::endl;
    assert(Validator::is_valid(s_check));


    // Diphthong vs Coda rules
    assert(Validator::is_valid(p.parse("kia")));
    assert(Validator::is_valid(p.parse("kiên")));
    assert(!Validator::is_valid(p.parse("kiê")));  // Should be 'kia'
    assert(!Validator::is_valid(p.parse("kian"))); // Should be 'kiên'

    assert(Validator::is_valid(p.parse("mua")));
    assert(Validator::is_valid(p.parse("muôn")));
    assert(!Validator::is_valid(p.parse("muô")));  // Should be 'mua'
    assert(!Validator::is_valid(p.parse("muan"))); // Should be 'muôn'

    assert(Validator::is_valid(p.parse("mưa")));
    assert(Validator::is_valid(p.parse("mươn")));
    assert(!Validator::is_valid(p.parse("mươ")));  // Should be 'mưa'
    assert(!Validator::is_valid(p.parse("mưan"))); // Should be 'mươn'


    // Q + Glide rules
    assert(Validator::is_valid(p.parse("qua")));
    assert(Validator::is_valid(p.parse("quân")));
    assert(!Validator::is_valid(p.parse("qa")));  // Q must have U
    assert(!Validator::is_valid(p.parse("qoa"))); // Should use u glide for q

    // Initial affinity
    assert(Validator::is_valid(p.parse("nghê")));
    assert(Validator::is_valid(p.parse("ngủ")));
    assert(!Validator::is_valid(p.parse("ngê")));   // Should be nghê
    assert(Validator::is_valid(p.parse("ngheo")));  // Correct: ngh + front vowel e (in eo)
    assert(!Validator::is_valid(p.parse("ngeo")));  // Incorrect: ng before front vowel e
    
    assert(Validator::is_valid(p.parse("ghe")));
    assert(!Validator::is_valid(p.parse("ge")));    // Should be ghe
    assert(Validator::is_valid(p.parse("go")));
    assert(!Validator::is_valid(p.parse("gho")));   // Should be go

    std::cout << "test_rhymes_exhaustive PASSED" << std::endl;
}
