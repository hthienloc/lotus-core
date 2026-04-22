/**
 * @file test_main.cpp
 * @brief Main test runner for the Lotus engine test suite.
 * @author Gemini CLI
 */

#include <iostream>

// ============================================================================
// [ External Test Declarations ]
// ============================================================================

// Core Logic Tests
void test_syllable_to_string_basic();
void test_syllable_is_empty();
void test_validator_basic();
void test_validator_complex();
void test_parser_basic();
void test_parser_special();
void test_rhymes_exhaustive();
void test_tone_style_placement();
void test_complex_diphthongs();

// Engine Tests
void test_engine_telex_basic();
void test_engine_telex_vowels();
void test_engine_telex_hooks();
void test_engine_telex_stroke();
void test_engine_telex_recovery();
void test_engine_telex_revert();
void test_engine_shortcuts();
void test_engine_production_features();
void test_engine_backspace_chaining();
void test_engine_vni_basic();
void test_engine_vni_vowels();
void test_engine_vni_combined();
void test_engine_stuck_word_bug();
void test_engine_linguistic_regression();
void test_engine_rebuild_state();
void test_engine_telex_free_w();
void test_engine_manual_hook_keys();
void test_engine_flexible_telex();
void test_engine_smart_typing();

// C-API Tests
void test_capi_run_all();
void test_capi_integration();

// ============================================================================
// [ Main Runner ]
// ============================================================================

/**
 * @brief Main entry point for the test suite.
 * Executes all registered test suites and reports overall status.
 */
int main() {
    std::cout << "Running Vietnamese Engine Tests..." << std::endl;

    // 1. Syllable & Validator Tests
    test_syllable_to_string_basic();
    test_syllable_is_empty();
    test_validator_basic();
    test_validator_complex();
    test_parser_basic();
    test_parser_special();
    test_rhymes_exhaustive();
    test_tone_style_placement();
    test_complex_diphthongs();

    // 2. Engine Tests (Telex & VNI)
    test_engine_telex_basic();
    test_engine_telex_vowels();
    test_engine_telex_hooks();
    test_engine_telex_stroke();
    test_engine_telex_recovery();
    test_engine_telex_revert();
    test_engine_shortcuts();
    test_engine_production_features();
    test_engine_backspace_chaining();
    test_engine_vni_basic();
    test_engine_vni_vowels();
    test_engine_vni_combined();
    test_engine_stuck_word_bug();
    test_engine_linguistic_regression();
    test_engine_rebuild_state();
    test_engine_telex_free_w();
    test_engine_manual_hook_keys();
    test_engine_flexible_telex();
    test_engine_smart_typing();

    // 3. C-API Tests
    test_capi_run_all();
    test_capi_integration();

    std::cout << "All tests PASSED!" << std::endl;
    return 0;
}
