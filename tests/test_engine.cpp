#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace lotus_engine;

void type_into(Engine& engine, std::u32string& screen, const std::string& keys,
               const Modifiers& mods = {}) {
    for (unsigned char c : keys) {
        auto res = engine.process_key((char32_t)c, mods);
        // Simulate TUI backspace behavior
        if (res.backspace == 0 && (c == 8 || c == 127)) {
            if (!screen.empty())
                screen.pop_back();
        } else {
            for (int i = 0; i < (int)res.backspace && !screen.empty(); i++)
                screen.pop_back();
            for (int i = 0; i < (int)res.count; i++)
                screen.push_back(res.chars[i]);
        }
    }
}

void assert_typing(Engine& engine, const std::string& keys, const std::string& expected) {
    engine.reset();
    std::u32string screen;
    type_into(engine, screen, keys);
    if (unicode::to_utf8(screen) != expected) {
        std::cerr << "Assertion failed: Input '" << keys << "' expected '" << expected << "', got '"
                  << unicode::to_utf8(screen) << "' (hex: ";
        for (unsigned char c : unicode::to_utf8(screen))
            std::cerr << std::hex << (int)c << " ";
        std::cerr << std::dec << ")" << std::endl;
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
    std::u32string screen;
    type_into(engine, screen, "cho tooi ");
    assert(unicode::to_utf8(screen) == "cho t\xC3\xB4i ");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "cho tôi");
    type_into(engine, screen, "s");
    assert(unicode::to_utf8(screen) == "cho tối");
    std::cout << "test_engine_stuck_word_bug PASSED" << std::endl;
}

void test_engine_backspace_chaining() {
    Engine engine;
    std::u32string screen;

    type_into(engine, screen, "ha cho ");
    assert(unicode::to_utf8(screen) == "ha cho ");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "ha cho");
    type_into(engine, screen, "\b\b\b");
    assert(unicode::to_utf8(screen) == "ha ");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "ha");
    type_into(engine, screen, "\b\b");
    assert(unicode::to_utf8(screen) == "");

    // 5. Syllable-aware backspace
    screen.clear();
    engine.reset();
    type_into(engine, screen, "xoas");
    assert(unicode::to_utf8(screen) == "xoá");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "xó");

    screen.clear();
    engine.reset();
    type_into(engine, screen, "tuyeens");
    assert(unicode::to_utf8(screen) == "tuyến");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "tuyế");

    screen.clear();
    engine.reset();
    type_into(engine, screen, "dduowngf");
    if (unicode::to_utf8(screen) != "đường") {
        std::cerr << "FAIL dduowngf: got '" << unicode::to_utf8(screen) << "' (hex: ";
        for (unsigned char c : unicode::to_utf8(screen))
            std::cerr << std::hex << (int)c << " ";
        std::cerr << ")" << std::endl;
        assert(false);
    }
    type_into(engine, screen, "\b");
    if (unicode::to_utf8(screen) != "đườn") {
        std::cerr << "FAIL dduowngf-back: got '" << unicode::to_utf8(screen) << "' (hex: ";
        for (unsigned char c : unicode::to_utf8(screen))
            std::cerr << std::hex << (int)c << " ";
        std::cerr << ")" << std::endl;
        assert(false);
    }

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
    engine = Engine();  // Reset
    engine.process_key('d', mods);
    engine.process_key('d', mods);  // TWO d's for đ
    engine.process_key('u', mods);
    engine.process_key('o', mods);
    engine.process_key('w', mods);
    res = engine.process_key('j', mods);

    // đượ should have mark on 'ơ' (U+1EE3)
    // Characters: đ (U+0111), ư (U+01B0), ợ (U+1EE3)
    std::u32string u32 = unicode::to_utf32(res.to_string());
    assert(u32.size() == 3);
    assert(u32[0] == 0x0111);  // đ
    assert(u32[1] == 0x01B0);  // ư
    assert(u32[2] == 0x1EE3);  // ợ

    // 3. Final completion (đượ -> được)
    res = engine.process_key('c', mods);
    assert(res.to_string() == "được");
    assert(res.count == 4);
    assert(res.backspace == 3);

    // 4. Reported Bug: nois -> nói (Previously resulted in noí)
    assert_typing(engine, "nois", "nói");
    assert_typing(engine, "tuis", "túi");
    assert_typing(engine, "muas", "múa");
    assert_typing(engine, "mua", "mua");
    assert_typing(engine, "muoons", "muốn");

    // 5. Late Hook Modifier (Free w)
    assert_typing(engine, "rangw", "răng");
    assert_typing(engine, "rangwf", "rằng");
    assert_typing(engine, "traw", "tră");
    assert_typing(engine, "uow", "ươ");
    assert_typing(engine, "uo", "uo");
    assert_typing(engine, "uow", "ươ");

    // 6. Special 'gi' and 'qu' handling
    assert_typing(engine, "gif", "gì");
    assert_typing(engine, "gin", "gin");
    assert_typing(engine, "gia", "gia");
    assert_typing(engine, "giar", "giả");
    assert_typing(engine, "quaf", "quà");
    assert_typing(engine, "qu", "qu");

    printf("test_engine_linguistic_regression PASSED\n");
}
