#include <iostream>
#include <cassert>
#include <string>
#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

using namespace lotus_engine;

void assert_typing(Engine& engine, const std::string& keys, const std::string& expected) {
    engine.reset();
    Modifiers mods;
    std::u32string screen;
    
    for (char c : keys) {
        auto res = engine.process_key((char32_t)c, mods);
        for (int i = 0; i < res.backspace && !screen.empty(); i++) screen.pop_back();
        for (int i = 0; i < res.count; i++) screen.push_back(res.chars[i]);
    }
    
    if (unicode::to_utf8(screen) != expected) {
        std::cerr << "Assertion failed: Input '" << keys << "' expected '" << expected 
                  << "', got '" << unicode::to_utf8(screen) << "' (hex: ";
        for (unsigned char c : unicode::to_utf8(screen)) std::cerr << std::hex << (int)c << " ";
        std::cerr << ")" << std::endl;
        assert(false);
    }
}

void test_engine_telex_basic() {
    Engine engine;
    assert_typing(engine, "h", "h");
    assert_typing(engine, "ha", "ha");
    assert_typing(engine, "has", "há");
    std::cout << "test_engine_telex_basic PASSED" << std::endl;
}

void test_engine_telex_vowels() {
    Engine engine;
    assert_typing(engine, "aa", "â");
    assert_typing(engine, "ee", "ê");
    assert_typing(engine, "oo", "ô");
    assert_typing(engine, "aw", "ă");
    assert_typing(engine, "ow", "ơ");
    assert_typing(engine, "uw", "ư");
    assert_typing(engine, "uow", "ươ");
    std::cout << "test_engine_telex_vowels PASSED" << std::endl;
}

void test_engine_telex_recovery() {
    Engine engine;
    assert_typing(engine, "hasz", "ha");
    std::cout << "test_engine_telex_recovery PASSED" << std::endl;
}

void test_engine_telex_stroke() {
    Engine engine;
    assert_typing(engine, "dd", "đ");
    std::cout << "test_engine_telex_stroke PASSED" << std::endl;
}

void test_engine_telex_revert() {
    Engine engine;
    assert_typing(engine, "aa", "â");
    assert_typing(engine, "aaa", "aa");
    assert_typing(engine, "as", "á");
    assert_typing(engine, "ass", "as");
    assert_typing(engine, "dd", "đ");
    assert_typing(engine, "ddd", "dd");
    std::cout << "test_engine_telex_revert PASSED" << std::endl;
}

void test_engine_production_features() {
    Engine engine;
    Modifiers mods;

    // 1. Backspace Recovery
    assert_typing(engine, "hello ", "hello ");
    engine.reset();
    engine.process_key('h', mods);
    engine.process_key('e', mods);
    engine.process_key('l', mods);
    engine.process_key('l', mods);
    engine.process_key('o', mods);
    engine.process_key(' ', mods);
    
    // Recovery deletes space
    auto res = engine.process_key(8, mods); 
    assert(res.action == 1);
    assert(res.backspace == 1);
    assert(res.count == 0); 

    // Next key is boundary -> commit and reset
    res = engine.process_key('!', mods);
    assert(res.action == 0); 
    assert(res.to_string() == "!");
    
    assert_typing(engine, "hello !", "hello !");

    // 2. English Auto-Restore
    assert_typing(engine, "test ", "test ");
    assert_typing(engine, "status ", "status ");

    std::cout << "test_engine_production_features PASSED" << std::endl;
}

void test_engine_shortcuts() {
    Engine engine;
    engine.add_shortcut("vn", "Việt Nam");

    assert_typing(engine, "vn", "vn");
    assert_typing(engine, "vn ", "Việt Nam ");
    assert_typing(engine, "Vn ", "Việt Nam ");
    assert_typing(engine, "VN ", "VIỆT NAM ");

    std::cout << "test_engine_shortcuts PASSED" << std::endl;
}

void test_engine_stuck_word_bug() {
    Engine engine;
    Modifiers mods;
    std::u32string screen;
    auto type = [&](const std::string& keys) {
        for (char c : keys) {
            auto res = engine.process_key((char32_t)c, mods);
            if (res.backspace == 0 && (c == 8 || c == 127)) {
                if (!screen.empty()) screen.pop_back();
            } else {
                for (int i = 0; i < (int)res.backspace && !screen.empty(); i++) screen.pop_back();
                for (int i = 0; i < (int)res.count; i++) screen.push_back(res.chars[i]);
            }
        }
    };

    // Scenario: "cho tôi "
    type("cho tooi ");
    // Expected: "cho" + " " + "t" + "ô" + "i" + " "
    assert(unicode::to_utf8(screen) == "cho t\xC3\xB4i ");

    // 1. Backspace through " " (Recovery)
    type("\b");
    assert(unicode::to_utf8(screen) == "cho tôi");

    // 2. Hit "s" -> Should NOT delete the space before "tôi"
    type("s"); 
    // "cho tối" = "cho" + " " + "t" + "ố" + "i"
    assert(unicode::to_utf8(screen) == "cho tối");
    
    std::cout << "test_engine_stuck_word_bug PASSED" << std::endl;
}

void test_engine_backspace_chaining() {
    Engine engine;
    Modifiers mods;
    std::u32string screen;
    auto type = [&](const std::string& keys) {
        for (char c : keys) {
            auto res = engine.process_key((char32_t)c, mods);
            // Simulate TUI pass-through for backspace
            if (res.backspace == 0 && (c == 8 || c == 127)) {
                if (!screen.empty()) screen.pop_back();
            } else {
                for (int i = 0; i < (int)res.backspace && !screen.empty(); i++) screen.pop_back();
                for (int i = 0; i < (int)res.count; i++) screen.push_back(res.chars[i]);
            }
        }
    };

    // Scenario: "ha cho "
    type("ha cho ");
    assert(unicode::to_utf8(screen) == "ha cho ");

    // 1. Backspace through " " (Recovery)
    type("\b");
    assert(unicode::to_utf8(screen) == "ha cho");

    // 2. Backspace through "cho"
    type("\b\b\b"); 
    assert(unicode::to_utf8(screen) == "ha ");

    // 3. Backspace through " " (Pass-through because LB was cleared by 'c')
    type("\b");
    assert(unicode::to_utf8(screen) == "ha");
    
    // 4. Backspace through "ha"
    type("\b\b");
    assert(unicode::to_utf8(screen) == "");
    
    std::cout << "test_engine_backspace_chaining PASSED" << std::endl;
}

void test_engine_linguistic_regression() {
    Engine engine;
    Modifiers mods;
    
    // 1. Normalization & Character Drift (nên -> nến)
    engine.process_key('n', mods);
    engine.process_key('e', mods);
    engine.process_key('e', mods);
    EngineResult res = engine.process_key('n', mods);
    assert(res.to_string() == "nên");
    assert(res.count == 3);

    res = engine.process_key('s', mods);
    assert(res.to_string() == "nến");
    assert(res.count == 3);
    assert(res.backspace == 3); 

    // 2. Tone Placement (đươ -> đượ)
    engine = Engine(); // Reset
    engine.process_key('d', mods);
    engine.process_key('d', mods); // TWO d's for đ
    engine.process_key('u', mods);
    engine.process_key('o', mods);
    engine.process_key('w', mods);
    res = engine.process_key('j', mods);
    
    // đượ should have mark on 'ơ' (U+1EE3)
    // Characters: đ (U+0111), ư (U+01B0), ợ (U+1EE3)
    std::u32string u32 = unicode::to_utf32(res.to_string());
    assert(u32.size() == 3);
    assert(u32[0] == 0x0111); // đ
    assert(u32[1] == 0x01B0); // ư
    assert(u32[2] == 0x1EE3); // ợ
    
    // 3. Final completion (đượ -> được)
    res = engine.process_key('c', mods);
    assert(res.to_string() == "được");
    assert(res.count == 4);
    assert(res.backspace == 3);
    
    printf("test_engine_linguistic_regression PASSED\n");
}
