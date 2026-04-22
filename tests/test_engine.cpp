#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/log.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace lotus_engine;

// ============================================================================
// [ Test Utilities ]
// ============================================================================

/**
 * @brief Simulates typing a sequence of keys into the engine and updating a virtual screen.
 * @param engine The engine instance to use.
 * @param screen The virtual screen (u32string) representing the current editor state.
 * @param keys The raw key sequence (as UTF-8 string).
 * @param mods Keyboard modifiers (Shift, CapsLock).
 */
void type_into(Engine& engine, std::u32string& screen, const std::string& keys,
               const Modifiers& mods = {}) {
    for (unsigned char c : keys) {
        auto res = engine.process_key(static_cast<char32_t>(c), mods);

        // Simulate frontend backspace behavior (e.g., when the engine doesn't explicitly 
        // handle a raw backspace keypress but the user expects the editor to delete).
        if (res.backspace == 0 && (c == 8 || c == 127)) {
            if (!screen.empty())
                screen.pop_back();
        } else {
            // Apply engine-directed backspaces (transformation logic)
            for (int i = 0; i < static_cast<int>(res.backspace) && !screen.empty(); i++)
                screen.pop_back();
            // Insert transformed characters
            for (int i = 0; i < static_cast<int>(res.count); i++)
                screen.push_back(res.chars[i]);
        }
    }
}

/**
 * @brief Asserts that typing a key sequence results in the expected Vietnamese text.
 * @param engine The engine instance.
 * @param keys The raw key input sequence.
 * @param expected The expected UTF-8 output string.
 */
void assert_typing(Engine& engine, const std::string& keys, const std::string& expected) {
    std::string local_logs;
    const char* vndebug = std::getenv("LOTUSDEBUG");
    bool debug_enabled = (vndebug && std::string(vndebug) == "1");

    if (debug_enabled) {
        set_log_callback([&local_logs](LogLevel level, const std::string& msg) {
            local_logs += "[" + std::string(level == LogLevel::ERROR ? "ERR" : "DBG") + "] " + msg + "\n";
        });
    }

    engine.reset();
    engine.set_at_sentence_start(true);
    std::u32string screen;
    type_into(engine, screen, keys);
    std::string actual = unicode::to_utf8(screen);

    if (debug_enabled) {
        set_log_callback(nullptr);
    }

    if (actual != expected) {
        if (debug_enabled && !local_logs.empty()) {
            std::cerr << "\n--- Debug Logs for Failed Test ---\n" << local_logs << "----------------------------------\n";
        }
        std::cerr << "[FAIL] Input: '" << keys << "'\n"
                  << "       Expected: '" << expected << "'\n"
                  << "       Actual:   '" << actual << "'\n";
        assert(false);
    }
}

// ============================================================================
// [ 1. Basic Telex Rules ]
// ============================================================================

/**
 * @brief Basic character-by-character Telex transformations.
 */
void test_engine_telex_basic() {
    Engine engine;
    assert_typing(engine, "h", "h");
    assert_typing(engine, "ha", "ha");
    assert_typing(engine, "has", "há");
    std::cout << "  [PASS] Basic Telex transformations" << std::endl;
}

/**
 * @brief Telex vowel marker transformations (aa, ee, oo).
 */
void test_engine_telex_vowels() {
    Engine engine;
    assert_typing(engine, "aa", "â");
    assert_typing(engine, "ee", "ê");
    assert_typing(engine, "oo", "ô");
    assert_typing(engine, "Vieejc", "Việc");
    assert_typing(engine, "Vieej", "Việ");
    std::cout << "  [PASS] Telex vowel markers (â, ê, ô)" << std::endl;
}

/**
 * @brief Telex hook marker transformations (uw, ow, aw).
 */
void test_engine_telex_hooks() {
    Engine engine;
    assert_typing(engine, "uw", "ư");
    assert_typing(engine, "ow", "ơ");
    assert_typing(engine, "aw", "ă");
    assert_typing(engine, "uow", "ươ");
    assert_typing(engine, "nhungw", "nhưng");
    assert_typing(engine, "duongw", "dương");
    assert_typing(engine, "muaw", "mưa");
    assert_typing(engine, "what", "what"); // English stability
    std::cout << "  [PASS] Telex hook markers (ư, ơ, ă)" << std::endl;
}

/**
 * @brief Telex stroke marker transformation (dd).
 */
void test_engine_telex_stroke() {
    Engine engine;
    assert_typing(engine, "dd", "đ");
    std::cout << "  [PASS] Telex stroke marker (đ)" << std::endl;
}

// ============================================================================
// [ 2. Advanced Composition Logic ]
// ============================================================================

/**
 * @brief Tests non-linear composition (Flexible Telex).
 */
void test_engine_flexible_telex() {
    Engine engine;
    engine.set_method(InputMethod::TELEX);
    engine.set_auto_restore(true);

    // Vowel-first or marker-late typing
    assert_typing(engine, "viecje", "việc");
    assert_typing(engine, "viecej", "việc");
    assert_typing(engine, "dduowjc", "được");
    assert_typing(engine, "dduowcj", "được");

    // Floating 'd' for 'đ'
    assert_typing(engine, "dosd", "đó");
    assert_typing(engine, "duwd", "đư");
    assert_typing(engine, "duowngfd", "đường");

    // Late Restore Protection (Word boundary)
    assert_typing(engine, "viecje ", "việc ");
    assert_typing(engine, "dosd ", "đó ");

    std::cout << "  [PASS] Flexible composition (late markers, floating 'd')" << std::endl;
}

/**
 * @brief Sequential marker typing should cycle or revert to literal characters.
 */
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
    std::cout << "  [PASS] Telex revert/escape logic" << std::endl;
}

/**
 * @brief Verification of 'z' as a tone-canceling key.
 */
void test_engine_telex_recovery() {
    Engine engine;
    assert_typing(engine, "hasz", "ha");
    std::cout << "  [PASS] Tone canceling (z-recovery)" << std::endl;
}

// ============================================================================
// [ 3. Production & Workflow Features ]
// ============================================================================

/**
 * @brief English auto-restoration and backspace recovery.
 */
void test_engine_production_features() {
    Engine engine;
    Modifiers mods;

    // 1. Backspace Recovery: Should delete the trailing space and restore composition state.
    engine.reset();
    {
        std::u32string screen;
        type_into(engine, screen, "hello ");
        auto res = engine.process_key(8, mods); // Backspace
        assert(res.action == 1);
        assert(res.backspace == 1);
        assert(res.count == 0);
    }

    // 2. English Auto-Restore: Prevents Vietnamese transformations for likely English words.
    assert_typing(engine, "test ", "tét "); // Small words may still transform depending on rules
    assert_typing(engine, "status ", "status ");
    assert_typing(engine, "for ", "for ");
    assert_typing(engine, "jump ", "jump ");
    assert_typing(engine, "win ", "win ");

    std::cout << "  [PASS] Workflow features (Recovery, English Restoration)" << std::endl;
}

/**
 * @brief Text expansion and replacement.
 */
void test_engine_shortcuts() {
    Engine engine;
    engine.add_shortcut("vn", "Việt Nam");

    assert_typing(engine, "vn", "vn");
    assert_typing(engine, "vn ", "Việt Nam ");
    assert_typing(engine, "Vn ", "Việt Nam ");
    assert_typing(engine, "VN ", "VIỆT NAM ");

    std::cout << "  [PASS] Text expansion shortcuts" << std::endl;
}

/**
 * @brief Smart typing features: Auto-capitalization and double-space to period.
 */
void test_engine_smart_typing() {
    Engine engine;
    
    // 1. Double Space to Period
    engine.set_double_space_to_period(true);
    engine.set_auto_capitalize(false);
    assert_typing(engine, "abc  e", "abc. e");
    
    // 2. Auto Capitalization
    engine.set_double_space_to_period(false);
    engine.set_auto_capitalize(true);
    assert_typing(engine, "abc", "Abc");
    
    // 3. Combined
    engine.reset();
    engine.set_at_sentence_start(true);
    engine.set_double_space_to_period(true);
    engine.set_auto_capitalize(true);
    
    std::u32string screen;
    type_into(engine, screen, "abc  e");
    std::string result = unicode::to_utf8(screen);
    assert(result == "Abc. E");

    std::cout << "  [PASS] Smart typing (Auto-Caps, Double-Space)" << std::endl;
}

// ============================================================================
// [ 4. Interactive State & Edge Cases ]
// ============================================================================

/**
 * @brief Interactive backspacing through multiple words and complex syllables.
 */
void test_engine_backspace_chaining() {
    Engine engine;
    assert_typing(engine, "ha cho \b", "ha cho");
    assert_typing(engine, "ha cho \b\b\b\b", "ha ");
    assert_typing(engine, "ha cho \b\b\b\b\b", "ha");
    assert_typing(engine, "duw\b", "d");
    assert_typing(engine, "tuas\b", "tú"); 
    assert_typing(engine, "huowngf\b", "hườn"); 
    assert_typing(engine, "tuyeens\b", "tuyế");
    assert_typing(engine, "dduowngf\b", "đườn");
    std::cout << "  [PASS] Interactive backspace chaining" << std::endl;
}

/**
 * @brief Regressions for specific Vietnamese syllables and linguistic patterns.
 */
void test_engine_linguistic_regression() {
    Engine engine;
    assert_typing(engine, "neens", "nến");
    assert_typing(engine, "dduowjc", "được");
    assert_typing(engine, "nois", "nói");
    assert_typing(engine, "traw", "tră");
    assert_typing(engine, "gia", "gia");
    assert_typing(engine, "giar", "giả");
    assert_typing(engine, "quaf", "quà");
    std::cout << "  [PASS] Linguistic regression suite" << std::endl;
}

/**
 * @brief Regression for a bug where state gets stuck after a word boundary.
 */
void test_engine_stuck_word_bug() {
    Engine engine;
    std::u32string screen;
    type_into(engine, screen, "cho tooi ");
    assert(unicode::to_utf8(screen) == "cho t\xC3\xB4i ");
    type_into(engine, screen, "\b"); // Restore 'tôi'
    assert(unicode::to_utf8(screen) == "cho tôi");
    type_into(engine, screen, "s");
    assert(unicode::to_utf8(screen) == "cho tối");
    std::cout << "  [PASS] Stuck word boundary bug regression" << std::endl;
}

/**
 * @brief Testing state reconstruction from arbitrary surrounding text.
 */
void test_engine_rebuild_state() {
    Engine engine;
    engine.rebuild_from_text("hòa");
    std::u32string screen = unicode::to_utf32("hòa");
    type_into(engine, screen, "\b");
    assert(unicode::to_utf8(screen) == "hò");

    engine.rebuild_from_text("xin");
    screen = unicode::to_utf32("xin");
    type_into(engine, screen, "a");
    assert(unicode::to_utf8(screen) == "xina");
    std::cout << "  [PASS] State reconstruction (rebuild_from_text)" << std::endl;
}

/**
 * @brief Standalone 'w' key behavior (Free-W option).
 */
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
    std::cout << "  [PASS] Standalone 'w' key behavior" << std::endl;
}

/**
 * @brief Manual hook keys ([, ], {, }).
 */
void test_engine_manual_hook_keys() {
    Engine engine;
    engine.set_std_uo(true);
    assert_typing(engine, "[", "ư");
    assert_typing(engine, "]", "ơ");
    assert_typing(engine, "{", "Ư");
    assert_typing(engine, "}", "Ơ");
    std::cout << "  [PASS] Manual hook keys (brackets)" << std::endl;
}

/**
 * @brief Regression for 'khuỷu' syllable and variants of typing it.
 */
void test_engine_khuyru_regression() {
    Engine engine;
    engine.set_auto_restore(true);
    
    // Test direct typing
    assert_typing(engine, "khuyu", "khuyu");
    assert_typing(engine, "khuyur", "khuỷu");
    
    // Test mid-word tone typing (The 'khuyru' sequence)
    assert_typing(engine, "khuyru", "khuỷu");
    
    // Test different orders
    assert_typing(engine, "khur", "khủ");
    assert_typing(engine, "khuru", "khủu");
    assert_typing(engine, "khuryu", "khuỷu");

    std::cout << "  [PASS] 'khuỷu' regression (khuyru)" << std::endl;
}

/**
 * @brief Tests Telex tone marker escape logic (typing marker twice for literal).
 */
void test_engine_telex_escapes() {
    Engine engine;
    engine.set_auto_restore(true);
    
    assert_typing(engine, "as", "á");
    assert_typing(engine, "ass", "as");
    assert_typing(engine, "mix", "mĩ");
    assert_typing(engine, "mixx", "mix");
    assert_typing(engine, "mixxi", "mixi");
    assert_typing(engine, "hasss", "hass");
    assert_typing(engine, "curx", "cũ");
    assert_typing(engine, "huowngff", "hươngf");
    
    std::cout << "  [PASS] Telex tone escapes (mixxi, hasss)" << std::endl;
}

/**
 * @brief Tests the 'Invalid Initial Gate' which prevents Vietnamese transformations 
 * on words starting with non-Vietnamese consonant clusters (like 'qq', 'f', 'w').
 */
void test_engine_english_gating() {
    Engine engine;
    engine.set_auto_restore(true);
    
    // Invalid Vietnamese initials should remain raw
    assert_typing(engine, "qquas", "qquas");
    assert_typing(engine, "for", "for");
    assert_typing(engine, "what", "what");
    assert_typing(engine, "status", "status");
    assert_typing(engine, "cs", "cs");
    assert_typing(engine, "ngs", "ngs");
    
    std::cout << "  [PASS] Invalid Initial Gate (English protection)" << std::endl;
}
