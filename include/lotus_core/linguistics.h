#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace lotus_core {

class Linguistics {
   public:
    /**
     * @brief Checks if a word is explicitly in the English whitelist.
     */
    static bool is_on_whitelist(std::string_view word);

    /**
     * @brief Checks if a word is definitely English (whitelist or unambiguous clusters).
     * Used for early transformation bypass.
     */
    static bool is_definite_english(std::string_view word);

    /**
     * @brief Checks if a word is likely English based on linguistic patterns.
     * Combines clusters and illegal finals.
     */
    static bool is_likely_english(std::string_view word);

   private:
    /**
     * @brief Checks if the word contains 'x' in positions unlikely for Vietnamese.
     */
    static bool has_english_x_pattern(std::string_view lower);

    /**
     * @brief Checks for definite English clusters at the start.
     */
    static bool has_english_start_cluster(std::string_view lower);

    /**
     * @brief Checks for English 'y' consonant patterns.
     */
    static bool has_english_y_pattern(std::string_view lower);

    /**
     * @brief Checks for non-Vietnamese consonant clusters (sh, wh, br, str, etc.)
     */
    static bool contains_english_cluster(std::string_view lower);

    /**
     * @brief Checks if a word ends with a consonant that is impossible in Vietnamese.
     */
    static bool has_impossible_final(std::string_view lower);

    /**
     * @brief Checks if a word ends with a common English suffix (like 'ine', 'one', 'ase').
     */
    static bool has_english_suffix(std::string_view lower);
};

}  // namespace lotus_core
