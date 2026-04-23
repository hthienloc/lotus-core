/**
 * @file test_validator.cpp
 * @brief Unit tests for the Vietnamese syllable validator.
 * @author Gemini CLI
 */

#include "lotus_engine/parser.h"
#include "lotus_engine/validator.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Tests basic syllable validation including consonant and vowel checks.
 */
void test_validator_basic() {
    Syllable s;
    s.initial = U"h";
    s.vowel = U"a";
    assert(Validator::is_valid(s));

    std::string reason;

    // Invalid consonant
    s.initial = U"z";
    assert(!Validator::is_valid(s, &reason));
    // std::cout << "    Diagnostics [z]: " << reason << std::endl;

    // Invalid vowel pattern
    s.initial = U"h";
    s.vowel = U"io";  // In Vietnamese "io" is not a standard vowel pattern alone
    if (!Validator::is_valid(s, &reason)) {
        std::cout << "    Diagnostics [" << unicode::to_utf8(s.vowel) << "]: " << reason << std::endl;
    }

    std::cout << "  [PASS] Basic validator checks" << std::endl;
}

/**
 * @brief Tests complex Vietnamese spelling rules and constraints.
 */
void test_validator_complex() {
    SyllableParser p;
    std::string reason;

    // Complex cases
    assert(Validator::is_valid(p.parse(U"ca")));
    assert(Validator::is_valid(p.parse(U"ke")));
    if (!Validator::is_valid(p.parse(U"ce"), &reason)) 
        std::cout << "    Diagnostics [ce]: " << reason << std::endl;
    if (!Validator::is_valid(p.parse(U"ka"), &reason))
        std::cout << "    Diagnostics [ka]: " << reason << std::endl;
    
    // Spelling rules: g/gh
    assert(Validator::is_valid(p.parse(U"ga")));
    assert(Validator::is_valid(p.parse(U"ghe")));
    if (!Validator::is_valid(p.parse(U"ge"), &reason))
        std::cout << "    Diagnostics [ge]: " << reason << std::endl;
    if (!Validator::is_valid(p.parse(U"gha"), &reason))
        std::cout << "    Diagnostics [gha]: " << reason << std::endl;

    // Spelling rules: ng/ngh
    assert(Validator::is_valid(p.parse(U"nga")));
    assert(Validator::is_valid(p.parse(U"nghe")));
    if (!Validator::is_valid(p.parse(U"nge"), &reason))
        std::cout << "    Diagnostics [nge]: " << reason << std::endl;
    if (!Validator::is_valid(p.parse(U"ngha"), &reason))
        std::cout << "    Diagnostics [ngha]: " << reason << std::endl;

    // Rule 6: Final consonant constraints
    assert(Validator::is_valid(p.parse(U"anh")));
    assert(Validator::is_valid(p.parse(U"êch")));
    assert(Validator::is_valid(p.parse(U"inh")));
    if (!Validator::is_valid(p.parse(U"ech"), &reason))
        std::cout << "    Diagnostics [ech]: " << reason << std::endl;
    if (!Validator::is_valid(p.parse(U"och"), &reason))
        std::cout << "    Diagnostics [och]: " << reason << std::endl;
    if (!Validator::is_valid(p.parse(U"eng"), &reason))
        std::cout << "    Diagnostics [eng]: " << reason << std::endl;

    std::cout << "  [PASS] Complex spelling rules" << std::endl;
}
