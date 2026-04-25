#pragma once

#include "lotus_core/common.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace lotus_core {

/**
 * @brief Represents a standard Vietnamese syllable structure: (C1)(G)V(C2) + T.
 *
 * Components:
 * - C1: Initial Consonant (Phụ âm đầu)
 * - G: Glide (Âm đệm)
 * - V: Vowel Nucleus (Hạt nhân nguyên âm)
 * - C2: Final Coda (Phụ âm cuối)
 * - T: Tone (Dấu thanh)
 */
struct Syllable {
    std::u32string initial;         ///< Initial Consonant (e.g., b, ch, ngh)
    std::optional<char32_t> glide;  ///< Glide (e.g., o, u)
    std::u32string vowel;           ///< Vowel Nucleus (e.g., a, ă, ê, iê)
    std::u32string final_c;         ///< Final Coda (e.g., n, ng, ch, i, y)
    Tone tone = Tone::NONE;         ///< Tone mark

    /**
     * @brief Converts the syllable to a UTF-8 string.
     * @param style The tone placement style to use.
     * @return std::string UTF-8 representation of the syllable.
     */
    std::string to_string(ToneStyle style = ToneStyle::NEW) const;

    /**
     * @brief Checks if the syllable is empty (no components set).
     */
    bool is_empty() const {
        return initial.empty() && !glide.has_value() && vowel.empty() && final_c.empty();
    }

    /**
     * @brief Removes the last visual character unit from the syllable.
     *
     * Handles complex Vietnamese character promote/demote logic.
     * Example: xó (x-o-s) -> xo (x-o), tuyến (t-u-y-e-e-n-s) -> tuyế.
     */
    void remove_last_char();

    /**
     * @brief Decomposes the current syllable back into raw input keys.
     * @param method The input method (TELEX/VNI) to use for decomposition.
     * @return std::vector<char32_t> A vector of UTF-32 key codes.
     */
    std::vector<char32_t> to_keys(InputMethod method) const;
};

/**
 * @brief Active keyboard modifier states.
 */
struct Modifiers {
    bool shift = false;      ///< Shift key state
    bool caps_lock = false;  ///< Caps Lock state
    bool ctrl = false;       ///< Ctrl key state
};

/**
 * @brief Actions returned by the engine indicating the type of transformation.
 */
enum class EngineAction : uint8_t {
    PASS = 0,      ///< No transformation, output character normally.
    TRANSFORM = 1, ///< Replace 'backspace' characters with 'count' new 'chars'.
    RESTORE = 2    ///< English detection restoration, revert to original keys.
};

/**
 * @brief Result structure returned after each key press, containing instructions for the frontend.
 * Designed to be FFI-compatible (C ABI).
 */
struct EngineResult {
    /**
     * @brief Output characters in UTF-32.
     * Fixed size for FFI compatibility. 'count' defines the number of valid entries.
     */
    uint32_t chars[32];

    /**
     * @brief Control action for the frontend.
     */
    EngineAction action;

    /**
     * @brief Number of characters to delete using backspace BEFORE inserting 'chars'.
     */
    uint8_t backspace;

    /**
     * @brief The number of valid characters in the 'chars' array.
     */
    uint8_t count;

    /**
     * @brief Helper to convert the result into a UTF-8 string.
     */
    std::string to_string() const;
};

}  // namespace lotus_core
