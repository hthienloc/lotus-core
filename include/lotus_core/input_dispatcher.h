#pragma once

#include "lotus_core/types.h"

namespace lotus_core {

/**
 * @brief Categorization of input keystrokes.
 */
enum class InputCategory : uint8_t {
    NAVIGATION, ///< Navigation keys (e.g., arrow keys, home, end)
    BACKSPACE,  ///< Deletion keys (Backspace, Delete)
    BOUNDARY,   ///< Word boundaries (Space, Enter, punctuation)
    CHARACTER   ///< Regular text characters
};

/**
 * @brief Utility class for classifying and routing keystrokes.
 * 
 * Separates the classification and routing logic from the main Engine orchestrator.
 */
class InputDispatcher {
public:
    /**
     * @brief Checks if a key combined with modifiers is a navigation command.
     * @param key The UTF-32 key code.
     * @param mods Active keyboard modifiers.
     * @return True if it is a navigation key sequence.
     */
    static bool is_nav_key(char32_t key, const Modifiers& mods);

    /**
     * @brief Checks if a character is a general word boundary.
     * @param c UTF-32 character.
     * @return True if it acts as a boundary.
     */
    static bool is_word_boundary(char32_t c);

    /**
     * @brief Categorizes a keypress into an InputCategory.
     * @param key The UTF-32 key code.
     * @param mods Active keyboard modifiers.
     * @return The determined InputCategory.
     */
    static InputCategory categorize(char32_t key, const Modifiers& mods);
};

} // namespace lotus_core
