#include <iostream>
#include <cassert>
#include "lotus_engine/types.h"

using namespace lotus_engine;

void test_syllable_to_string_basic() {
    Syllable s;
    s.initial = "h";
    s.vowel = "a";
    assert(s.to_string() == "ha");
    std::cout << "test_syllable_to_string_basic PASSED" << std::endl;
}

void test_syllable_is_empty() {
    Syllable s;
    assert(s.is_empty());
    s.initial = "b";
    assert(!s.is_empty());
    std::cout << "test_syllable_is_empty PASSED" << std::endl;
}

// In a real project we'd use a framework, but for this task I'll call them in a simple registry if needed.
// For now, I'll just add them to test_main later or call them in a constructor.
// Actually, let's keep it simple.
