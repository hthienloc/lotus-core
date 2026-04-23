/**
 * @file test_edge_cases.cpp
 * @brief Tests for complex Vietnamese syllables and edge cases.
 */

#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/log.h"

#include <cassert>
#include <iostream>
#include <string>
#include <cstdlib>

using namespace lotus_engine;

// ============================================================================
// [ Test Utilities ]
// ============================================================================

void type_into_edge(Engine& engine, std::u32string& screen, const std::string& keys,
               const Modifiers& mods = {}) {
    for (unsigned char c : keys) {
        auto res = engine.process_key(static_cast<char32_t>(c), mods);

        if (res.backspace == 0 && (c == 8 || c == 127)) {
            if (!screen.empty())
                screen.pop_back();
        } else {
            for (int i = 0; i < static_cast<int>(res.backspace) && !screen.empty(); i++)
                screen.pop_back();
            for (int i = 0; i < static_cast<int>(res.count); i++)
                screen.push_back(res.chars[i]);
        }
    }
}

void assert_typing_edge(Engine& engine, const std::string& keys, const std::string& expected) {
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
    type_into_edge(engine, screen, keys);
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
// [ Edge Case Tests ]
// ============================================================================

void test_complex_syllables() {
    Engine engine;
    engine.set_method(InputMethod::TELEX);
    engine.set_auto_restore(true);

    // khuỷu
    assert_typing_edge(engine, "khuyru", "khuỷu");
    assert_typing_edge(engine, "khuyur", "khuỷu");
    assert_typing_edge(engine, "khur", "khủ");
    assert_typing_edge(engine, "khuru", "khủu");
    assert_typing_edge(engine, "khuryu", "khuỷu");

    // ngoan
    assert_typing_edge(engine, "ngoan", "ngoan");
    assert_typing_edge(engine, "ngoans", "ngoán");
    assert_typing_edge(engine, "ngoanf", "ngoàn");

    // nghễnh
    assert_typing_edge(engine, "ngheexnh", "nghễnh");
    
    // Additional edge cases
    assert_typing_edge(engine, "nghien", "nghien");
    assert_typing_edge(engine, "nghieng", "nghieng");
    assert_typing_edge(engine, "nghieeng", "nghiêng");
    assert_typing_edge(engine, "nghieengs", "nghiếng");
    
    assert_typing_edge(engine, "quoc", "quoc");
    assert_typing_edge(engine, "quoocs", "quốc");
    
    assert_typing_edge(engine, "huowu", "hươu");
    assert_typing_edge(engine, "huowus", "hướu");

    std::cout << "  [PASS] Complex syllables edge cases (khuỷu, ngoan, nghễnh, etc.)" << std::endl;
}

void test_edge_cases_run() {
    test_complex_syllables();
}
