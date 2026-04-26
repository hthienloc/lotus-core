#pragma once

#include "lotus_core/types.h"
#include <string>
#include <optional>

namespace lotus_core {

/**
 * @brief Result of a buffer transformation.
 */
struct TransformationResult {
    std::u32string output;
    bool key_consumed;
    bool is_valid_vn;
    bool has_valid_initial;
    DiagnosticCode diagnostic = DiagnosticCode::SUCCESS;
};

/**
 * @brief Manages the active typing buffer and applies linguistic transformations.
 *
 * Encapsulates the raw sequence of typed keys and orchestrates the application
 * of TELEX or VNI rules, syllable parsing, and phonotactic validation.
 */
class CompositionBuffer {
public:
    CompositionBuffer() = default;

    /** @brief Appends a key to the raw buffer. */
    bool append_key(char32_t key, size_t commit_threshold);

    /**
     * @brief Transforms the raw buffer into a Vietnamese syllable string.
     * @param key The key that triggered the transformation.
     * @param method The input method (TELEX or VNI).
     * @param free_w The FreeWOption for TELEX.
     * @param style The tone placement style.
     * @param allow_non_standard Whether non-standard initials are allowed.
     * @return TransformationResult containing the output string and metadata.
     */
    TransformationResult transform(char32_t key, InputMethod method, FreeWOption free_w, ToneStyle style, bool allow_non_standard);

    /** @brief Clears the raw buffer and internal state. */
    void clear();

    /** @brief Returns the raw buffer content. */
    const std::u32string& get_raw() const { return buffer; }
    
    /** @brief Directly sets the raw buffer. Used when restoring from history. */
    void set_raw(const std::u32string& new_buffer);

    /** @brief Pops the last raw character. */
    void pop_back();

    /** @brief Handles manual tone escape (e.g., 'aaa' -> 'aa'). */
    std::optional<std::u32string> handle_manual_tone_escape(char32_t key);

    /** @brief Applies standard UO key mappings. */
    void handle_hook_key_shortcuts(char32_t& key, bool std_uo);

    /** @brief Evaluates if the given word is likely English based on rules. */
    bool is_likely_english(const std::string& word, InputMethod method, FreeWOption free_w, bool allow_non_standard) const;

private:
    std::u32string buffer;
    char32_t last_modifier_key = 0;

    void apply_telex_rules(std::string& current_str, char32_t key, bool& key_consumed, Tone& tone_state, FreeWOption free_w) const;
    void apply_vni_rules(std::string& current_str, char32_t key, bool& key_consumed, Tone& tone_state) const;
};

} // namespace lotus_core
