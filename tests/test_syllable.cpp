/**
 * @file test_syllable.cpp
 * @brief Unit tests for the Syllable structure.
 */

#include "lotus_engine/types.h"

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
    std::cout << "  [PASS] Syllable basic to_string" << std::endl;
}

/**
 * @brief Verifies the 'is_empty' logic for various syllable states.
 */
void test_syllable_is_empty() {
    Syllable s;
    assert(s.is_empty());
    s.initial = U"b";
    assert(!s.is_empty());
    std::cout << "  [PASS] Syllable is_empty check" << std::endl;
}
