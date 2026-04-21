#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace lotus_engine;

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

void test_engine_vni_basic() {
    Engine engine;
    assert(type_vni(engine, "a1") == "á");
    assert(type_vni(engine, "a2") == "à");
    assert(type_vni(engine, "a3") == "ả");
    assert(type_vni(engine, "a4") == "ã");
    assert(type_vni(engine, "a5") == "ạ");
    std::cout << "test_engine_vni_basic PASSED" << std::endl;
}

void test_engine_vni_vowels() {
    Engine engine;
    assert(type_vni(engine, "a6") == "\xC3\xA2");  // â
    assert(type_vni(engine, "e6") == "\xC3\xAA");  // ê
    assert(type_vni(engine, "o6") == "\xC3\xB4");  // ô
    assert(type_vni(engine, "u7") == "\xC6\xB0");  // ư
    assert(type_vni(engine, "o7") == "\xC6\xA1");  // ơ
    assert(type_vni(engine, "a8") == "\xC4\x83");  // ă
    assert(type_vni(engine, "d9") == "\xC4\x91");  // đ
    std::cout << "test_engine_vni_vowels PASSED" << std::endl;
}

void test_engine_vni_combined() {
    Engine engine;
    assert(type_vni(engine, "a61") == "ấ");           // â + sắc
    assert(type_vni(engine, "o72") == "ờ");           // ơ + huyền
    assert(type_vni(engine, "duong795") == "đượng");  // đ + ư + ơ + nặng + ng
    std::cout << "test_engine_vni_combined PASSED" << std::endl;
}
