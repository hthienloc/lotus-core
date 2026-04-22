#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

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
    engine.set_at_sentence_start(true);
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

void test_engine_flexible_telex() {
    Engine engine;
    engine.set_method(InputMethod::TELEX);
    engine.set_auto_restore(true);

    // 1. Flexible Vowels (Non-linear composition)
    assert_typing(engine, "viecje", "việc");
    assert_typing(engine, "viecej", "việc");
    assert_typing(engine, "dduowjc", "được");
    assert_typing(engine, "dduowcj", "được");

    // 2. Flexible Consonants (Floating d)
    assert_typing(engine, "dosd", "đó");
    assert_typing(engine, "duwd", "đư");
    assert_typing(engine, "duowngfd", "đường");

    // 3. Late Restore Protection (Word boundary)
    assert_typing(engine, "viecje ", "việc ");
    assert_typing(engine, "dosd ", "đó ");

    // 4. English Stability (High-likelihood patterns)
    assert_typing(engine, "where", "where");
    assert_typing(engine, "status", "status");
    assert_typing(engine, "school", "school");

    std::cout << "test_engine_flexible_telex PASSED" << std::endl;
}

void test_engine_telex_vowels() {
    Engine engine;
    assert_typing(engine, "aa", "â");
    assert_typing(engine, "ee", "ê");
    assert_typing(engine, "oo", "ô");
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
    assert_typing(engine, "quas", "quá");
    assert_typing(engine, "quass", "quas");
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
    assert_typing(engine, "test ", "tét ");
    assert_typing(engine, "status ", "status ");
    assert_typing(engine, "for ", "for ");
    assert_typing(engine, "jump ", "jump ");
    assert_typing(engine, "win ", "win ");

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
    assert_typing(engine, "ha cho \b", "ha cho");
    assert_typing(engine, "ha cho \b\b\b\b", "ha ");
    assert_typing(engine, "ha cho \b\b\b\b\b", "ha");
    assert_typing(engine, "ha cho \b\b\b\b\b\b\b", "");
    assert_typing(engine, "dd\b", "");
    assert_typing(engine, "aa\b", "");
    assert_typing(engine, "duw\b", "d");
    assert_typing(engine, "dow\b", "d");
    assert_typing(engine, "tuas\b", "tú"); 
    assert_typing(engine, "huowngf\b", "hườn"); 
    assert_typing(engine, "xoas\b", "xó");
    assert_typing(engine, "tuyeens\b", "tuyế");
    assert_typing(engine, "dduowngf", "đường");
    assert_typing(engine, "dduowngf\b", "đườn");
    std::cout << "test_engine_backspace_chaining PASSED" << std::endl;
}

void test_engine_linguistic_regression() {
    Engine engine;
    assert_typing(engine, "neens", "nến");
    assert_typing(engine, "dduowj", "đượ");
    assert_typing(engine, "dduowjc", "được");
    assert_typing(engine, "nois", "nói");
    assert_typing(engine, "tuis", "túi");
    assert_typing(engine, "muas", "múa");
    assert_typing(engine, "mua", "mua");
    assert_typing(engine, "muoons", "muốn");
    assert_typing(engine, "rangw", "răng");
    assert_typing(engine, "traw", "tră");
    assert_typing(engine, "gif", "gì");
    assert_typing(engine, "gin", "gin");
    assert_typing(engine, "gia", "gia");
    assert_typing(engine, "giar", "giả");
    assert_typing(engine, "quaf", "quà");
    assert_typing(engine, "qu", "qu");
    std::cout << "test_engine_linguistic_regression PASSED" << std::endl;
}

void test_engine_rebuild_state() {
    Engine engine;
    engine.rebuild_from_text("hòa");
    std::u32string screen = unicode::to_utf32("hòa");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "hò");

    engine.rebuild_from_text("xin chào");
    screen = unicode::to_utf32("xin chào");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "xin chà");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "xin ch");
    
    type_into(engine, screen, "\b\b\b\b\b\b\b"); 
    assert(unicode::to_utf8(screen).length() <= 3); 
    
    engine.rebuild_from_text("xin");
    screen = unicode::to_utf32("xin");
    type_into(engine, screen, "a");
    assert(unicode::to_utf8(screen) == "xina");
    std::cout << "test_engine_rebuild_state PASSED" << std::endl;
}

void test_engine_telex_hooks() {
    Engine engine;
    assert_typing(engine, "uw", "ư");
    assert_typing(engine, "ow", "ơ");
    assert_typing(engine, "aw", "ă");
    assert_typing(engine, "uow", "ươ");
    assert_typing(engine, "nhungw", "nhưng");
    assert_typing(engine, "duongw", "dương");
    assert_typing(engine, "dduowngf", "đường");
    assert_typing(engine, "muaw", "mưa");
    assert_typing(engine, "uaw", "ưa");
    assert_typing(engine, "what", "what");
    assert_typing(engine, "where", "where");
    std::cout << "test_engine_telex_hooks PASSED" << std::endl;
}

void test_engine_telex_free_w() {
    Engine engine;
    engine.set_free_w(FreeWOption::ALWAYS);
    assert_typing(engine, "w", "ư");
    assert_typing(engine, "tw", "tư");
    engine.set_free_w(FreeWOption::NON_START);
    assert_typing(engine, "w", "w");
    assert_typing(engine, "tw", "tư");
    engine.set_free_w(FreeWOption::OFF);
    assert_typing(engine, "w", "w");
    assert_typing(engine, "tw", "tw");
    std::cout << "test_engine_telex_free_w PASSED" << std::endl;
}

void test_engine_manual_hook_keys() {
    Engine engine;
    engine.set_std_uo(true);
    assert_typing(engine, "[", "ư");
    assert_typing(engine, "]", "ơ");
    assert_typing(engine, "{", "Ư");
    assert_typing(engine, "}", "Ơ");
    assert_typing(engine, "[s", "ứ");
    assert_typing(engine, "v]ngf", "vờng");
    std::cout << "test_engine_manual_hook_keys PASSED" << std::endl;
}

void test_engine_smart_typing() {
    Engine engine;
    
    // 1. Double Space to Period (Disabled by default)
    assert_typing(engine, "space  ", "space  ");
    
    // 2. Enable Double Space (Auto-Caps: OFF)
    engine.set_double_space_to_period(true);
    engine.set_auto_capitalize(false);
    assert_typing(engine, "abc  e", "abc. e"); // a is lowercase, e is lowercase
    
    // 3. Enable Auto Capitalization (Double-Space: OFF)
    engine.set_double_space_to_period(false);
    engine.set_auto_capitalize(true);
    assert_typing(engine, "abc", "Abc");
    assert_typing(engine, "double  space", "Double  space"); // Only first word capitalized
    
    // 4. Combined features
    engine.reset();
    engine.set_at_sentence_start(true);
    engine.set_double_space_to_period(true);
    engine.set_auto_capitalize(true);
    
    std::u32string screen;
    type_into(engine, screen, "abc  e");
    assert(unicode::to_utf8(screen) == "Abc. E");

    std::cout << "test_engine_smart_typing PASSED" << std::endl;
}

