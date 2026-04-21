#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace lotus_engine {

class Linguistics {
   public:
    /**
     * @brief Checks if a word is explicitly in the English whitelist.
     */
    static bool is_on_whitelist(const std::string& word);

    /**
     * @brief Checks if a word is definitely English (whitelist or unambiguous clusters).
     * Used for early transformation bypass.
     */
    static bool is_definite_english(const std::string& word);

    /**
     * @brief Checks if a word is likely English based on linguistic patterns.
     * Combines clusters and illegal finals.
     */
    static bool is_likely_english(const std::string& word);

    /**
     * @brief Checks for non-Vietnamese consonant clusters (sh, wh, br, str, etc.)
     */
    static bool contains_english_cluster(const std::string& word);

    /**
     * @brief Checks if a word ends with a consonant that is impossible in Vietnamese.
     */
    static bool has_impossible_final(const std::string& word);
};

}  // namespace lotus_engine
