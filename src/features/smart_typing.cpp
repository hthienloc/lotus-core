#include "lotus_core/smart_typing.h"
#include "lotus_core/unicode.h"

namespace lotus_core {

bool SmartTyping::handle(char32_t& key, bool double_space_to_period, bool auto_capitalize, char32_t last_boundary_key, bool at_sentence_start, const std::u32string& buffer, EngineResult& result, std::u32string& last_committed_text) {
    // Check for double-space to period formatting
    // If the user typed a space, and the last boundary recorded was also a space,
    // we backspace the previous space and replace it with a period and a new space.
    if (double_space_to_period && key == ' ' && last_boundary_key == ' ') {
        result.action = EngineAction::TRANSFORM;       // Transformation action
        result.backspace = 1;    // Backspace the previously committed space
        result.count = 2;        // Insert two characters
        result.chars[0] = '.';   // Period
        result.chars[1] = ' ';   // Space
        last_committed_text = U". ";
        return true; // We handled the double space to period, no further engine processing needed for this keystroke
    }

    // Check for auto-capitalization at the start of a sentence
    // We only capitalize if the cursor is recognized to be at the start of a sentence,
    // there's a valid character input, and the composition buffer is empty (meaning
    // we're at the very beginning of a new word).
    if (auto_capitalize && at_sentence_start && key != 0 && buffer.empty()) {
        char32_t upper = unicode::to_upper(key);
        if (upper != key)
            key = upper; // Modify the input key in-place to its uppercase equivalent
    }
    return false; // Return false so the engine continues to process the (potentially modified) key
}

} // namespace lotus_core
