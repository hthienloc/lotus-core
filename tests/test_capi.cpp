/**
 * @file test_capi.cpp
 * @brief Unit tests for the Lotus C-API.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_engine/capi.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// [ Test Cases ]
// ============================================================================

/**
 * @brief Tests basic C-API engine creation and key processing.
 */
void test_capi_basic() {
    lotus_engine_t* engine = lotus_engine_create();
    assert(engine != NULL);

    lotus_modifiers_t mods = {false, false};

    // Type 'h'
    lotus_result_t res = lotus_engine_process_key(engine, 'h', mods);
    // The engine may return replacement (1) with 0 backspaces for the first char
    assert(res.action == 1 || res.action == 0);
    assert(res.backspace == 0);
    assert(res.count == 1);
    assert(res.chars[0] == 'h');

    // Type 'a'
    res = lotus_engine_process_key(engine, 'a', mods);
    // Replacement of "h" with "ha"
    assert(res.action == 1);
    assert(res.backspace == 1);
    assert(res.count == 2);
    assert(res.chars[0] == 'h');
    assert(res.chars[1] == 'a');

    // Type 's' (Telex sharp)
    res = lotus_engine_process_key(engine, 's', mods);
    assert(res.action == 1);
    assert(res.backspace == 2);
    assert(res.count == 2);
    // h + á (0x00E1)
    assert(res.chars[0] == 'h');
    assert(res.chars[1] == 0x00E1);

    lotus_engine_destroy(engine);
    printf("  \033[1;32m[PASS]\033[0m C-API basic operations\n");
}

/**
 * @brief Tests tone style configuration through the C-API.
 */
void test_capi_tone_style() {
    lotus_engine_t* engine = lotus_engine_create();
    lotus_modifiers_t mods = {false, false};

    // NEW style (hoà) - Default
    lotus_engine_process_key(engine, 'h', mods);
    lotus_engine_process_key(engine, 'o', mods);
    lotus_engine_process_key(engine, 'a', mods);
    lotus_result_t res = lotus_engine_process_key(engine, 'f', mods);
    assert(res.chars[2] == 0x00E0);  // à

    lotus_engine_reset(engine);

    // OLD style (hòa)
    lotus_engine_set_tone_style(engine, LOTUS_TONE_OLD);
    lotus_engine_process_key(engine, 'h', mods);
    lotus_engine_process_key(engine, 'o', mods);
    lotus_engine_process_key(engine, 'a', mods);
    res = lotus_engine_process_key(engine, 'f', mods);
    // Mark on glide 'o'
    assert(res.chars[1] == 0x00F2);  // ò
    assert(res.chars[2] == 'a');

    lotus_engine_destroy(engine);
    printf("  \033[1;32m[PASS]\033[0m C-API tone style configuration\n");
}

/**
 * @brief Entry point for the C-API unit test suite.
 */
void test_capi_run_all() {
    test_capi_basic();
    test_capi_tone_style();
}
