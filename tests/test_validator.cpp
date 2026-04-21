#include "lotus_engine/parser.h"
#include "lotus_engine/validator.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

void test_validator_basic() {
    Syllable s;
    s.initial = "h";
    s.vowel = "a";
    assert(Validator::is_valid(s));

    // Invalid consonant
    s.initial = "z";
    assert(!Validator::is_valid(s));

    // Invalid vowel pattern
    s.initial = "h";
    s.vowel = "io";  // In Vietnamese "io" is not a standard vowel pattern alone
    assert(!Validator::is_valid(s));

    std::cout << "test_validator_basic PASSED" << std::endl;
}

void test_validator_complex() {
    SyllableParser p;

    // Complex cases
    assert(Validator::is_valid(p.parse("ca")));
    assert(Validator::is_valid(p.parse("ke")));
    assert(!Validator::is_valid(p.parse("ce")));  // Should be ke
    assert(!Validator::is_valid(p.parse("ka")));  // Should be ca

    // Spelling rules: g/gh
    assert(Validator::is_valid(p.parse("ga")));
    assert(Validator::is_valid(p.parse("ghe")));
    assert(!Validator::is_valid(p.parse("ge")));   // Should be ghe
    assert(!Validator::is_valid(p.parse("gha")));  // Should be ga

    // Spelling rules: ng/ngh
    assert(Validator::is_valid(p.parse("nga")));
    assert(Validator::is_valid(p.parse("nghe")));
    assert(!Validator::is_valid(p.parse("nge")));   // Should be nghe
    assert(!Validator::is_valid(p.parse("ngha")));  // Should be nga

    // Rule 6: Final consonant constraints
    assert(Validator::is_valid(p.parse("anh")));
    assert(Validator::is_valid(p.parse("êch")));
    assert(Validator::is_valid(p.parse("inh")));
    assert(!Validator::is_valid(p.parse("ech")));  // Only valid with ê, a, i
    assert(!Validator::is_valid(p.parse("och")));  // och not valid in VN
    assert(!Validator::is_valid(p.parse("eng")));  // Should use -nh: anh/ênh

    printf("test_validator_complex PASSED\n");
}
