/**
 * @file test_capi_integration.cpp
 * @brief Integration tests for the Lotus C-API.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_engine/capi.h"
#include "lotus_engine/unicode.h"

#include <cassert>
#include <iostream>

using namespace lotus_engine;

// ============================================================================
// [ Helpers ]
// ============================================================================

/**
 * @brief Utility to assert that a sequence of keys results in expected Vietnamese output.
 * @param engine The C-API engine handle.
 * @param input Raw key input sequence.
 * @param expected Expected UTF-8 output.
 */
void assert_capi_typing(lotus_engine_t* engine, const std::string& input,
                        const std::string& expected) {
    lotus_engine_reset(engine);
    lotus_modifiers_t mods = {false, false};
    lotus_result_t res;
    for (char c : input) {
        res = lotus_engine_process_key(engine, (char32_t)c, mods);
    }
    std::u32string res_u32;
    for (int i = 0; i < res.count; i++)
        res_u32.push_back(res.chars[i]);
    std::string got = unicode::to_utf8(res_u32);
    if (got != expected) {
        printf("  \033[1;31m[FAIL]\033[0m C-API: typing '%s' expected '%s' got '%s'\n", input.c_str(),
               expected.c_str(), got.c_str());
        assert(false);
    }
}

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Verifies engine behavior through the C-API including Telex/VNI and tone styles.
 */
void test_capi_integration() {
    lotus_engine_t* engine = lotus_engine_create();
    assert(engine != nullptr);

    // 1. Basic Telex Input (hòa vs hoà)
    lotus_engine_set_tone_style(engine, LOTUS_TONE_OLD);
    assert_capi_typing(engine, "hof", "hò");
    assert_capi_typing(engine, "hoaf", "hòa");

    lotus_engine_set_tone_style(engine, LOTUS_TONE_NEW);
    assert_capi_typing(engine, "hoaf", "hoà");

    // 2. VNI Input
    lotus_engine_set_method(engine, LOTUS_METHOD_VNI);
    assert_capi_typing(engine, "vie65t", "việt");

    lotus_engine_destroy(engine);
    std::cout << "  \033[1;32m[PASS]\033[0m C-API integration suite" << std::endl;
}
