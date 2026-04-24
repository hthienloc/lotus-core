/**
 * @file test_main.cpp
 * @brief Main test runner for the Lotus engine test suite.
 * @author Huỳnh Thiện Lộc
 */

#include <iostream>

// ============================================================================
// [ External Test Declarations ]
// ============================================================================

// --- Core ---
/** @brief Tests basic syllable-to-string conversion without complex tones. */
void test_syllable_to_string_basic();
/** @brief Verifies the 'is_empty' logic for various syllable states. */
void test_syllable_is_empty();
/** @brief Tests out-of-order vowel typing (Bamboo-style reordering). */
void test_vowel_reordering();
/** @brief Tests basic syllable validation including consonant and vowel checks. */
void test_validator_basic();
/** @brief Tests complex syllable validation including tones and glides. */
void test_validator_complex();
/** @brief Tests basic syllable parsing including initial, vowel, and final components. */
void test_parser_basic();
/** @brief Tests parsing of special cases like 'qu-' and 'gi-'. */
void test_parser_special();
/** @brief Exhaustive validation of complex Vietnamese rhymes and syllable structures. */
void test_rhymes_exhaustive();
/** @brief Verifies correct parsing of syllables into their constituent parts. */
void test_syllable_parts();
/** @brief Validates strict Vietnamese orthography and spelling rules. */
void test_orthography_rules();
/** @brief Tests tone placement difference between OLD (hòa) and NEW (hoà) styles. */
void test_tone_style_placement();
/** @brief Tests tone placement on complex diphthongs and triphthongs. */
void test_complex_diphthongs();

// --- Engine ---
void test_engine_telex_basic();
void test_engine_expect_bug();
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
void test_engine_khuyru_regression();
void test_engine_telex_escapes();
void test_engine_english_gating();
void test_engine_punctuation_backspace();
void test_engine_reproduction_user();

// --- C-API ---
void test_capi_run_all();
void test_capi_integration();

// --- Edge Cases ---
void test_edge_cases_run();

// --- Features ---
void test_features();

// ============================================================================
// [ Main Runner ]
// ============================================================================

/**
 * @brief Main entry point for the test suite.
 * Executes all registered test suites and reports overall status.
 */
int main() {
    std::cout << "Running Vietnamese Engine Tests..." << std::endl;

    // --- Core ---
    std::cout << "\n\033[1;34m[ Core Tests ]\033[0m\n";
    test_syllable_to_string_basic();
    test_syllable_is_empty();
    test_vowel_reordering();
    test_validator_basic();
    test_validator_complex();
    test_parser_basic();
    test_parser_special();
    test_rhymes_exhaustive();
    test_syllable_parts();
    test_orthography_rules();
    test_tone_style_placement();
    test_complex_diphthongs();

    // --- Engine ---
    std::cout << "\n\033[1;34m[ Engine Tests ]\033[0m\n";
    test_engine_telex_basic();
    test_engine_expect_bug();
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
    test_engine_khuyru_regression();
    test_engine_telex_escapes();
    test_engine_english_gating();
    test_engine_punctuation_backspace();
    test_engine_reproduction_user();

    // --- C-API ---
    std::cout << "\n\033[1;34m[ C-API Tests ]\033[0m\n";
    test_capi_run_all();
    test_capi_integration();

    // --- Edge Cases ---
    std::cout << "\n\033[1;34m[ Edge Case Tests ]\033[0m\n";
    test_edge_cases_run();

    // --- Features ---
    std::cout << "\n\033[1;34m[ Feature Tests ]\033[0m\n";
    test_features();

    std::cout << "\n\033[1;32mAll tests PASSED!\033[0m" << std::endl;
    return 0;
}
