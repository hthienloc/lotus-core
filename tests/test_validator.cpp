#include "lotus_engine/parser.h"
#include "lotus_engine/validator.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

void test_validator_basic() {
    Syllable s;
    s.initial = U"h";
    s.vowel = U"a";
    assert(Validator::is_valid(s));

    // Invalid consonant
    s.initial = U"z";
    assert(!Validator::is_valid(s));

    // Invalid vowel pattern
    s.initial = U"h";
    s.vowel = U"io";  // In Vietnamese "io" is not a standard vowel pattern alone
    assert(!Validator::is_valid(s));

    std::cout << "test_validator_basic PASSED" << std::endl;
}

void test_validator_complex() {
    SyllableParser p;

    // Complex cases
    assert(Validator::is_valid(p.parse(U"ca")));
    assert(Validator::is_valid(p.parse(U"ke")));
    assert(!Validator::is_valid(p.parse(U"ce")));  // Should be ke
    assert(!Validator::is_valid(p.parse(U"ka")));  // Should be ca

    // Spelling rules: g/gh
    assert(Validator::is_valid(p.parse(U"ga")));
    assert(Validator::is_valid(p.parse(U"ghe")));
    assert(!Validator::is_valid(p.parse(U"ge")));   // Should be ghe
    assert(!Validator::is_valid(p.parse(U"gha")));  // Should be ga

    // Spelling rules: ng/ngh
    assert(Validator::is_valid(p.parse(U"nga")));
    assert(Validator::is_valid(p.parse(U"nghe")));
    assert(!Validator::is_valid(p.parse(U"nge")));   // Should be nghe
    assert(!Validator::is_valid(p.parse(U"ngha")));  // Should be nga

    // Rule 6: Final consonant constraints
    assert(Validator::is_valid(p.parse(U"anh")));
    assert(Validator::is_valid(p.parse(U"êch")));
    assert(Validator::is_valid(p.parse(U"inh")));
    assert(!Validator::is_valid(p.parse(U"ech")));  // Only valid with ê, a, i
    assert(!Validator::is_valid(p.parse(U"och")));  // och not valid in VN
    assert(!Validator::is_valid(p.parse(U"eng")));  // Should use -nh: anh/ênh

    printf("test_validator_complex PASSED\n");
}
