#include <iostream>

int main() {
    std::cout << "Running Vietnamese Engine Tests..." << std::endl;

    void test_syllable_to_string_basic();
    test_syllable_to_string_basic();

    void test_syllable_is_empty();
    test_syllable_is_empty();

    void test_validator_basic();
    test_validator_basic();

    void test_validator_complex();
    test_validator_complex();

    void test_parser_basic();
    test_parser_basic();

    void test_parser_special();
    test_parser_special();

    // Engine Tests (Telex)
    void test_engine_telex_basic();
    void test_engine_telex_vowels();
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
    void test_rhymes_exhaustive();
    void test_tone_style_placement();
    void test_complex_diphthongs();
    void test_capi_run_all();
    void test_capi_integration();
    void test_engine_linguistic_regression();
    void test_engine_rebuild_state();
    void test_engine_telex_free_w();
    void test_engine_manual_hook_keys();

    test_engine_telex_basic();
    test_engine_telex_vowels();
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
    test_rhymes_exhaustive();
    test_tone_style_placement();
    test_complex_diphthongs();
    test_capi_run_all();
    test_capi_integration();
    test_engine_linguistic_regression();
    test_engine_rebuild_state();
    test_engine_telex_free_w();
    test_engine_manual_hook_keys();

    std::cout << "All tests PASSED!" << std::endl;
    return 0;
}
