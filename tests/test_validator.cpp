/**
 * @file test_validator.cpp
 * @brief Unit tests for the Vietnamese syllable validator.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_core/parser.h"
#include "lotus_core/validator.h"
#include "lotus_core/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_core;

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Tests basic syllable validation including consonant and vowel checks.
 */
void test_validator_basic() {
    Syllable s;
    s.initial = std::u32string_view(U"h");
    s.vowel = std::u32string_view(U"a");
    assert(Validator::is_valid(s));

    DiagnosticCode reason;

    // Invalid consonant
    s.initial = U"z";
    bool r_z = Validator::is_valid(s, &reason);
    if (r_z != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [z]: " << to_string(reason) << std::endl;
    }
    assert(r_z == false);

    // Invalid vowel pattern
    s.initial = std::u32string_view(U"h");
    s.vowel = U"io";  // In Vietnamese "io" is not a standard vowel pattern alone
    bool r_io = Validator::is_valid(s, &reason);
    if (r_io != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [" << unicode::to_utf8(s.vowel.view()) << "]: " << to_string(reason) << std::endl;
    }
    assert(r_io == false);

    std::cout << "  \033[1;32m[PASS]\033[0m Basic validator checks" << std::endl;
}

/**
 * @brief Tests complex Vietnamese spelling rules and constraints.
 */
void test_validator_complex() {
    SyllableParser p;
    DiagnosticCode reason;

    // Complex cases
    assert(Validator::is_valid(p.parse(U"ca")));
    assert(Validator::is_valid(p.parse(U"ke")));
    bool r_ce = Validator::is_valid(p.parse(U"ce"), &reason);
    if (r_ce != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [ce]: " << to_string(reason) << std::endl;
    }
    assert(r_ce == false);
    bool r_ka = Validator::is_valid(p.parse(U"ka"), &reason);
    if (r_ka != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [ka]: " << to_string(reason) << std::endl;
    }
    assert(r_ka == false);
    
    // Spelling rules: g/gh
    assert(Validator::is_valid(p.parse(U"ga")));
    assert(Validator::is_valid(p.parse(U"ghe")));
    bool r_ge = Validator::is_valid(p.parse(U"ge"), &reason);
    if (r_ge != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [ge]: " << to_string(reason) << std::endl;
    }
    assert(r_ge == false);
    bool r_gha = Validator::is_valid(p.parse(U"gha"), &reason);
    if (r_gha != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [gha]: " << to_string(reason) << std::endl;
    }
    assert(r_gha == false);

    // Spelling rules: ng/ngh
    assert(Validator::is_valid(p.parse(U"nga")));
    assert(Validator::is_valid(p.parse(U"nghe")));
    bool r_nge = Validator::is_valid(p.parse(U"nge"), &reason);
    if (r_nge != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [nge]: " << to_string(reason) << std::endl;
    }
    assert(r_nge == false);
    bool r_ngha = Validator::is_valid(p.parse(U"ngha"), &reason);
    if (r_ngha != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [ngha]: " << to_string(reason) << std::endl;
    }
    assert(r_ngha == false);

    // Rule 6: Final consonant constraints
    assert(Validator::is_valid(p.parse(U"anh")));
    assert(Validator::is_valid(p.parse(U"êch")));
    assert(Validator::is_valid(p.parse(U"inh")));
    bool r_ech = Validator::is_valid(p.parse(U"ech"), &reason);
    if (r_ech != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [ech]: " << to_string(reason) << std::endl;
    }
    assert(r_ech == false);
    bool r_och = Validator::is_valid(p.parse(U"och"), &reason);
    if (r_och != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [och]: " << to_string(reason) << std::endl;
    }
    assert(r_och == false);
    bool r_eng = Validator::is_valid(p.parse(U"eng"), &reason);
    if (r_eng != false) {
        std::cout << "    \033[1;31m[FAIL]\033[0m Diagnostics [eng]: " << to_string(reason) << std::endl;
    }
    assert(r_eng == false);

    std::cout << "  \033[1;32m[PASS]\033[0m Complex spelling rules" << std::endl;
}
