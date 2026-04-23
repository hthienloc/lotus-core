/**
 * @file test_edge_cases.cpp
 * @brief Tests for complex Vietnamese syllables and direct validator edge cases.
 */

#include "lotus_engine/engine.h"
#include "lotus_engine/log.h"
#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/validator.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace lotus_engine;

// ============================================================================
// [ Test Utilities ]
// ============================================================================

/**
 * @brief Simulates typing a sequence of keys into the engine and updating a virtual screen.
 */
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

/**
 * @brief Asserts that typing a sequence of keys results in the expected output string.
 */
void assert_typing_edge(Engine& engine, const std::string& keys, const std::string& expected) {
    std::string local_logs;
    const char* vndebug = std::getenv("LOTUSDEBUG");
    bool debug_enabled = (vndebug && std::string(vndebug) == "1");

    if (debug_enabled) {
        set_log_callback([&local_logs](LogLevel level, const std::string& msg) {
            local_logs +=
                "[" + std::string(level == LogLevel::ERROR ? "ERR" : "DBG") + "] " + msg + "\n";
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
            std::cerr << "\n--- Debug Logs for Failed Test ---\n"
                      << local_logs << "----------------------------------\n";
        }
        std::cerr << "[FAIL] Input: '" << keys << "'\n"
                  << "       Expected: '" << expected << "'\n"
                  << "       Actual:   '" << actual << "'\n";
        assert(false);
    }
}

// ============================================================================
// [ 1. Direct Validator Tests ]
// ============================================================================

/**
 * @brief Tests internal validator rules directly (Unit tests for refactoring).
 */
void test_validator_edge_cases() {
    SyllableParser p;

    // Front vowel checks (strict and non-strict)
    assert(Validator::is_valid(p.parse(U"ke")));
    assert(Validator::is_valid(p.parse(U"kê")));
    assert(Validator::is_valid(p.parse(U"ki")));
    assert(Validator::is_valid(p.parse(U"ky")));
    assert(!Validator::is_valid(p.parse(U"ka")));

    assert(Validator::is_valid(p.parse(U"ca")));
    assert(!Validator::is_valid(p.parse(U"ce")));

    assert(Validator::is_valid(p.parse(U"ghe")));
    assert(!Validator::is_valid(p.parse(U"ghy")));

    // e vowel + ng coda restrictions
    assert(Validator::is_valid(p.parse(U"ang")));
    assert(!Validator::is_valid(p.parse(U"eng")));
    assert(!Validator::is_valid(p.parse(U"êng")));

    // valid ch/nh nucleus
    assert(Validator::is_valid(p.parse(U"ach")));
    assert(Validator::is_valid(p.parse(U"anh")));
    assert(Validator::is_valid(p.parse(U"ich")));
    assert(!Validator::is_valid(p.parse(U"och")));

    // Centering diphthongs requiring coda
    assert(Validator::is_valid(p.parse(U"iêng")));
    assert(Validator::is_valid(p.parse(U"uông")));
    assert(!Validator::is_valid(p.parse(U"iê")));

    // Centering diphthongs forbidding coda
    assert(Validator::is_valid(p.parse(U"ia")));
    assert(Validator::is_valid(p.parse(U"ua")));
    assert(!Validator::is_valid(p.parse(U"iang")));

    std::cout << "  [PASS] Validator direct edge cases" << std::endl;
}

// ============================================================================
// [ 2. Engine Level Tests ]
// ============================================================================

/**
 * @brief Tests engine behavior on complex syllables and known edge cases.
 */
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
    assert_typing_edge(engine, "nghieeng", "nghiêng");
    assert_typing_edge(engine, "huowu", "hươu");

    std::cout << "  [PASS] Engine complex syllables" << std::endl;
}

/**
 * @brief Main runner function for edge cases suite.
 */
void test_edge_cases_run() {
    test_validator_edge_cases();
    test_complex_syllables();
}
