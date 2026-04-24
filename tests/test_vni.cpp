/**
 * @file test_vni.cpp
 * @brief Unit tests for the VNI input method.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace lotus_engine;

// ============================================================================
// [ Helpers ]
// ============================================================================

/**
 * @brief Simulates typing into the engine using the VNI input method.
 * @param engine The engine instance.
 * @param input The raw key sequence.
 * @return The resulting UTF-8 string.
 */
static std::string type_vni(Engine& engine, const std::string& input) {
    engine.reset();
    engine.set_method(InputMethod::VNI);
    std::u32string screen;
    Modifiers mods;
    for (char c : input) {
        auto res = engine.process_key((char32_t)c, mods);
        for (int i = 0; i < res.backspace && !screen.empty(); i++)
            screen.pop_back();
        for (int i = 0; i < res.count; i++)
            screen.push_back(res.chars[i]);
    }
    return unicode::to_utf8(screen);
}

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Tests basic VNI tone markers (1-5).
 */
void test_engine_vni_basic() {
    Engine engine;
    assert(type_vni(engine, "a1") == "á");
    assert(type_vni(engine, "a2") == "à");
    assert(type_vni(engine, "a3") == "ả");
    assert(type_vni(engine, "a4") == "ã");
    assert(type_vni(engine, "a5") == "ạ");
    std::cout << "  \033[1;32m[PASS]\033[0m Basic VNI tones" << std::endl;
}

/**
 * @brief Tests VNI vowel markers (6-8) and stroke (9).
 */
void test_engine_vni_vowels() {
    Engine engine;
    assert(type_vni(engine, "a6") == "\xC3\xA2");  // â
    assert(type_vni(engine, "e6") == "\xC3\xAA");  // ê
    assert(type_vni(engine, "o6") == "\xC3\xB4");  // ô
    assert(type_vni(engine, "u7") == "\xC6\xB0");  // ư
    assert(type_vni(engine, "o7") == "\xC6\xA1");  // ơ
    assert(type_vni(engine, "a8") == "\xC4\x83");  // ă
    assert(type_vni(engine, "d9") == "\xC4\x91");  // đ
    std::cout << "  \033[1;32m[PASS]\033[0m VNI vowel markers and stroke" << std::endl;
}

/**
 * @brief Tests combined VNI vowel and tone markers.
 */
void test_engine_vni_combined() {
    Engine engine;
    assert(type_vni(engine, "a61") == "ấ");           // â + sắc
    assert(type_vni(engine, "o72") == "ờ");           // ơ + huyền
    assert(type_vni(engine, "duong795") == "đượng");  // đ + ư + ơ + nặng + ng
    assert(type_vni(engine, "Vie65c") == "Việc");
    assert(type_vni(engine, "hoa8c5") == "hoặc");
    assert(type_vni(engine, "hoa7c5") == "hoạc");
    std::cout << "  \033[1;32m[PASS]\033[0m Combined VNI transformations" << std::endl;
}
