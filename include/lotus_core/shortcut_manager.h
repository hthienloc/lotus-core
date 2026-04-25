#pragma once

#include "lotus_core/types.h"
#include "lotus_core/common.h"
#include <string>
#include <map>

namespace lotus_core {

/**
 * @brief Manages macro expansions and text shortcuts.
 * 
 * ShortcutManager provides a mechanism to map shorthand triggers to full replacement texts.
 * Linguistically, this is crucial for Vietnamese typing because many common words or phrases
 * are long and repetitive (e.g., "ko" -> "không", "dc" -> "được"). By allowing users to define
 * custom shortcuts, the engine significantly speeds up typing and reduces the number of keystrokes.
 * 
 * The expansion only happens when a "trigger key" (such as space, punctuation, or newline)
 * is pressed, ensuring that shortcuts don't prematurely expand while the user is still in the
 * middle of typing a word. The manager also supports different macro modes to adapt to the
 * user's capitalization of the trigger.
 */
class ShortcutManager {
public:
    /**
     * @brief Registers a new shortcut mapping.
     * 
     * @param trigger The shorthand string that triggers the replacement (e.g., "ko").
     * @param replacement The full string that should replace the trigger (e.g., "không").
     */
    void add_shortcut(const std::string& trigger, const std::string& replacement);

    /**
     * @brief Clears all registered shortcuts.
     */
    void clear();

    /**
     * @brief Attempts to expand a shortcut based on the current buffer and input key.
     * 
     * This function checks if the current `buffer` matches any registered shortcut trigger
     * and if the input `key` acts as a valid trigger boundary (e.g., space or punctuation).
     * The `MacroMode` dictates how capitalization of the trigger affects the replacement.
     * 
     * @param key The character currently being inputted.
     * @param buffer The sequence of characters typed prior to the key.
     * @param result Populated with the transformation result (backspaces and output characters) if a shortcut is expanded.
     * @param mode The macro expansion mode (ADAPTIVE, EXACT, or FIXED) determining capitalization logic.
     * @return true if a shortcut was successfully expanded, false otherwise.
     */
    bool handle(char32_t key, const std::u32string& buffer, EngineResult& result, MacroMode mode = MacroMode::ADAPTIVE);

private:
    /**
     * @brief Maps lowercase trigger strings to their corresponding replacement strings.
     * Used primarily for ADAPTIVE and FIXED macro modes.
     */
    std::map<std::string, std::string> shortcuts;
    
    /**
     * @brief Maps exact, case-sensitive trigger strings to their corresponding replacement strings.
     * Used specifically for the EXACT macro mode to strictly match user input.
     */
    std::map<std::string, std::string> exact_shortcuts;
    
    /**
     * @brief Determines if a character acts as a valid boundary to trigger a shortcut expansion.
     * 
     * Linguistically, shortcuts should only expand when the user has finished typing the word.
     * Therefore, boundaries are typically whitespaces (space, tab, newline) or punctuation marks
     * (comma, period, exclamation mark, etc.).
     * 
     * @param key The input character to test.
     * @return true if the character is a boundary trigger, false otherwise.
     */
    bool is_trigger_key(char32_t key) const;

    /**
     * @brief Helper function to construct an EngineResult for a successful shortcut expansion.
     * 
     * @param final_u32 The full expanded replacement string combined with the trigger key.
     * @param prev_size The length of the trigger string in the buffer, dictating how many backspaces are needed.
     * @return An EngineResult structure instructing the editor on what text to replace and insert.
     */
    EngineResult _make_transformation_result(const std::u32string& final_u32, size_t prev_size) const;
};

} // namespace lotus_core
