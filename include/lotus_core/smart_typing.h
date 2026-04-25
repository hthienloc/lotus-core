#pragma once

#include "lotus_core/types.h"
#include <string>

namespace lotus_core {

/**
 * @brief Manages smart typing features such as auto-capitalization and double-space to period.
 * 
 * SmartTyping improves the user experience by automating common text formatting tasks,
 * specifically mimicking modern mobile keyboard behaviors. These features help users
 * type faster and more ergonomically without manually switching cases or reaching for
 * punctuation keys.
 */
class SmartTyping {
public:
    /**
     * @brief Processes the current input key to apply smart typing behaviors.
     * 
     * This function evaluates the current input context against user-configured smart typing
     * settings. 
     * 
     * - Double-space to period: When a space is entered immediately following another space,
     *   it is interpreted as the end of a sentence. The first space is replaced with a period,
     *   followed by a single space (e.g., "word[space][space]" -> "word. ").
     * 
     * - Auto-capitalization: When starting a new sentence (after a period, question mark, or 
     *   exclamation point), the first alphabetic character typed is automatically capitalized,
     *   saving the user from using the Shift key.
     * 
     * @param key The current character being typed. May be modified (e.g., capitalized).
     * @param double_space_to_period User configuration to enable double-space to period.
     * @param auto_capitalize User configuration to enable automatic capitalization at the start of a sentence.
     * @param last_boundary_key The last boundary character recorded (useful for detecting consecutive spaces).
     * @param at_sentence_start Boolean flag indicating if the cursor is at the start of a new sentence.
     * @param buffer The current uncommitted string buffer.
     * @param result Populated with the transformation result (e.g., backspacing the first space and inserting ". ") if double-space to period is triggered.
     * @param last_committed_text Reference to the string of text just committed, updated if a transformation is applied.
     * @return true if a smart typing transformation (like double-space to period) fully consumed the event and requires immediate output, false if the key should continue being processed by the main engine (like auto-capitalized characters).
     */
    static bool handle(char32_t& key, bool double_space_to_period, bool auto_capitalize, char32_t last_boundary_key, bool at_sentence_start, const std::u32string& buffer, EngineResult& result, std::u32string& last_committed_text);
};

} // namespace lotus_core
