/**
 * @file test_tone_style.cpp
 * @brief Unit tests for Vietnamese tone placement styles (Old vs. New).
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_core/engine.h"
#include "lotus_core/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_core;

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Tests tone placement difference between OLD (hòa) and NEW (hoà) styles.
 */
void test_tone_style_placement() {
    Engine engine;

    // Test NEW Style (Nucleus-centric: hoà) - Default
    engine.set_method(InputMethod::TELEX);
    engine.set_tone_style(ToneStyle::NEW);

    (void)engine.process_key('h', {});
    (void)engine.process_key('o', {});
    (void)engine.process_key('a', {});
    auto res4 = engine.process_key('f', {});

    // Expected: hoà
    std::u32string out_u32_new(res4.chars, res4.chars + res4.count);
    std::string out_new = unicode::to_utf8(out_u32_new);
    // h + o + à = hoà. à is U+00E0.
    assert(out_new == "hoà");

    engine.reset();

    // Test OLD Style (Glide-centric: hòa)
    engine.set_tone_style(ToneStyle::OLD);
    engine.process_key('h', {});
    engine.process_key('o', {});
    engine.process_key('a', {});
    auto res_old = engine.process_key('f', {});

    // Expected: hòa
    std::u32string out_u32_old(res_old.chars, res_old.chars + res_old.count);
    std::string out_old = unicode::to_utf8(out_u32_old);
    assert(out_old == "hòa");

    std::cout << "  \033[1;32m[PASS]\033[0m Tone placement styles (Old vs. New)" << std::endl;
}

/**
 * @brief Tests tone placement on complex diphthongs and triphthongs.
 */
void test_complex_diphthongs() {
    Engine engine;
    engine.set_method(InputMethod::TELEX);
    engine.set_tone_style(ToneStyle::NEW);

    auto type = [&](const std::string& keys) -> std::string {
        engine.reset();
        EngineResult res;
        for (char c : keys) {
            res = engine.process_key(c, {});
        }
        return unicode::to_utf8(std::u32string(res.chars, res.chars + res.count));
    };

    // triphthongs (iêu, uôi, ươi, ươu)
    assert(type("kieeur") == "kiểu");
    assert(type("chuoois") == "chuối");
    assert(type("ruowuj") == "rượu");
    assert(type("huowu") == "hươu");
    assert(type("nguowif") == "người");

    std::cout << "  \033[1;32m[PASS]\033[0m Complex diphthongs and triphthongs" << std::endl;
}
