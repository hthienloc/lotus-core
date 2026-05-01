/**
 * @file engine.cpp
 * @brief Core input config.method engine implementation.
 *
 * Orchestrates the processing of keyboard input, applying linguistic rules,
 * managing word history, and handling advanced features like shortcuts and auto-restore.
 */

#include "lotus_core/engine.h"

#include "lotus_core/constants.h"
#include "lotus_core/linguistics.h"
#include "lotus_core/log.h"
#include "lotus_core/parser.h"
#include "lotus_core/unicode.h"
#include "lotus_core/validator.h"
#include "lotus_core/smart_typing.h"

#include <algorithm>
#include <cctype>
#include <cstring>

using namespace lotus_core;

using namespace constants;

namespace lotus_core {

// ============================================================================
// [ EngineResult Implementation ]
// ============================================================================

/**
 * @brief Converts the engine result structure to a UTF-8 string.
 * @return UTF-8 string representation of the transformed characters.
 */
std::string EngineResult::to_string() const {
    std::u32string u32;
    for (uint8_t i = 0; i < count; ++i)
        u32 += chars[i];
    return unicode::to_utf8(u32);
}

// ============================================================================
// [ Engine Implementation ]
// ============================================================================

/**
 * @brief Default constructor for Engine.
 *
 * Initializes the engine with default Telex config.method and all smart features disabled.
 */
Engine::Engine() : config(), state() {}

/**
 * @brief Registers a new text shortcut.
 * @param trigger The short key sequence.
 * @param replacement The full string to expand into.
 */
void Engine::add_shortcut(const std::string& trigger, const std::string& replacement) {
    shortcut_manager.add_shortcut(trigger, replacement);
}

/**
 * @brief Clears all registered text expansion shortcuts.
 */
void Engine::clear_shortcuts() {
    shortcut_manager.clear();
}

/**
 * @brief Resets the internal engine state (composition_buffer.get_raw(), markers, committed text).
 * Does NOT clear the word history.
 */
void Engine::reset() {
    composition_buffer.clear();
    
    state.last_boundary_key = 0;
    state.last_committed_text.clear();
}

/**
 * @brief Completely clears the engine state, including word history.
 * Used when the cursor moves to a completely new, unrelated position.
 */
void Engine::clear_all() {
    reset();
    context_tracker.clear();
}

/**
 * @brief Reconstructs the engine state from a block of surrounding text.
 *
 * Used for synchronization when the cursor moves or text is modified externally.
 * @param text The surrounding context string.
 */
void Engine::rebuild_from_text(const std::string& text) {
    reset();
    context_tracker.clear();
    
    ReconstructResult result = context_tracker.reconstruct(text, config.method);
    context_tracker.set_at_sentence_start(result.at_sentence_start);
    
    for (const auto& w : result.history_words) {
        context_tracker.push_word(w);
    }
    
    composition_buffer.set_raw(result.active_buffer);
    state.last_committed_text = result.last_committed_text;
    state.last_boundary_key = result.last_boundary_key;
}

/**
 * @brief Commits the current syllable buffer to history, followed by the boundary key.
 * @param boundary_key The boundary character that triggered the commit.
 */
void Engine::commit_syllable_to_history(char32_t boundary_key) {
    if (!composition_buffer.get_raw().empty()) {
        auto transform_res = composition_buffer.transform(0, config.method, config.free_w, config.tone_style, config.allow_non_standard_initials, config.tone_less, config.mark_less);
        context_tracker.push_word(std::u32string(transform_res.output.view()));
    }
    context_tracker.push_boundary(boundary_key);
}

/**
 * @brief Processes a single key press and returns the transformation result.
 *
 * This is the main entry point for keyboard input. It handles backspacing,
 * boundary detection, smart typing features, and initiates IM-specific
 * modifier application.
 *
 * @param original_key The UTF-32 key code.
 * @param mods Keyboard modifiers (Shift, CapsLock).
 * @return EngineResult containing the action to perform on the host application.
 */
EngineResult Engine::process_key(char32_t original_key, const Modifiers& mods) {
    char32_t key = original_key;
    composition_buffer.handle_hook_key_shortcuts(key, config.std_uo);

    EngineResult res{};
    
    // --- UX Safety: Auto-commit when buffer is nearly full (127 chars) ---
    // This ensures no data loss even if the user types extremely long sequences.
    if (composition_buffer.get_raw().size() >= StaticString::MAX_LEN_CONST - 1 &&
        InputDispatcher::categorize(key, mods) == InputCategory::CHARACTER) {
        
        auto transform_res = composition_buffer.transform(0, config.method, config.free_w, config.tone_style, config.allow_non_standard_initials, config.tone_less, config.mark_less);
        res.action = EngineAction::TRANSFORM;
        res.backspace = 0;
        res.count = transform_res.output.size();
        for (size_t i = 0; i < res.count; ++i) res.chars[i] = transform_res.output[i];
        
        // Reset state for the next word
        commit_syllable_to_history(0);
        composition_buffer.clear();
        state.last_committed_text = transform_res.output;
        
        return res;
    }

    InputCategory category = InputDispatcher::categorize(key, mods);

    switch (category) {
        case InputCategory::NAVIGATION:
            handle_navigation(key, mods, res);
            return res;

        case InputCategory::BACKSPACE:
            handle_backspace(key, mods, res);
            return res;

        case InputCategory::BOUNDARY:
            if (handle_smart_typing(key, mods, res) && res.action != EngineAction::PASS)
                return res;
            if (key != 0 && composition_buffer.get_raw().empty()) {
                state.last_committed_text.clear();
            }
            if (handle_boundary(key, res))
                return res;
            break;

        case InputCategory::CHARACTER:
            if (handle_smart_typing(key, mods, res) && res.action != EngineAction::PASS)
                return res;
            if (key != 0 && composition_buffer.get_raw().empty()) {
                state.last_committed_text.clear();
            }
            if (auto esc_opt = composition_buffer.handle_manual_tone_escape(key); esc_opt.has_value()) {
                context_tracker.set_at_sentence_start(false);
                state.last_boundary_key = 0;
                return build_result(esc_opt.value());
            }
            if (key != 0) {
                bool forced_flush = composition_buffer.append_key(key, config.commit_threshold);
                if (forced_flush) {
                    auto transform_res = composition_buffer.transform(0, config.method, config.free_w, config.tone_style, config.allow_non_standard_initials, config.tone_less, config.mark_less);
                    EngineResult br = build_result(transform_res.output.view());
                    br.diagnostic = transform_res.diagnostic;
                    br.action = EngineAction::PASS;
                    state.last_committed_text.clear();
                    context_tracker.set_at_sentence_start(false);
                    composition_buffer.clear();
                    return br;
                }
                context_tracker.set_at_sentence_start(false);
            }
            break;
    }


    auto transform_res = composition_buffer.transform(key, config.method, config.free_w, config.tone_style, config.allow_non_standard_initials, config.tone_less, config.mark_less);
    
    if (config.auto_restore && !transform_res.has_valid_initial && !transform_res.key_consumed) {
        LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "Restore: Invalid initial prefix"));
        EngineResult br = build_result(composition_buffer.get_raw().view());
        br.diagnostic = DiagnosticCode::ENGLISH_RESTORED;
        return br;
    }

    bool is_eng = is_likely_english(unicode::to_utf8(composition_buffer.get_raw().view()));
    if (config.auto_restore && is_eng && (!transform_res.is_valid_vn || (!transform_res.key_consumed && key != 'z' && key != 'Z'))) {
        LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "Restore: English word logic"));
        EngineResult br = build_result(composition_buffer.get_raw().view());
        br.diagnostic = DiagnosticCode::ENGLISH_RESTORED;
        return br;
    }

    LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "Final: " + unicode::to_utf8(transform_res.output.view())));
    EngineResult br = build_result(transform_res.output.view());
    br.diagnostic = transform_res.diagnostic;
    return br;
}





/**
 * @brief Handles navigation and escape keys.
 */
bool Engine::handle_navigation(char32_t key, const Modifiers& mods, EngineResult& result) {
    (void)result;
    bool is_nav_key = std::find(NAV_KEYS.begin(), NAV_KEYS.end(), key) != NAV_KEYS.end();
    bool is_ctrl_nav = mods.ctrl && std::find(CTRL_NAV_KEYS.begin(), CTRL_NAV_KEYS.end(), key) != CTRL_NAV_KEYS.end();

    if (is_nav_key || is_ctrl_nav) {
        clear_all();
        return true;
    }
    return false;
}



/**
 * @brief Handles backspace key logic.
 *
 * If the composition_buffer.get_raw() is not empty, it attempts to reconstruct the previous syllable state.
 * If the composition_buffer.get_raw() is empty, it attempts to recover the last committed word from history.
 *
 * @param key The key pressed (KEY_BACKSPACE or KEY_DELETE).
 * @param mods Keyboard modifiers.
 * @param result OUT: The engine result to populate.
 * @return True if the key was a backspace and was handled.
 */
bool Engine::handle_backspace(char32_t key, const Modifiers& mods, EngineResult& result) {
    if (key != KEY_BACKSPACE && key != KEY_DELETE)
        return false;
    if (!composition_buffer.get_raw().empty()) {
        std::string word = unicode::to_utf8(state.last_committed_text);
        Syllable s = SyllableParser::parse(unicode::to_utf32(word));
        
        bool is_english_fallback = config.auto_restore && is_likely_english(unicode::to_utf8(composition_buffer.get_raw().view()));

        if (config.backspace_style == BackspaceStyle::KEYSTROKE || is_english_fallback) {
            composition_buffer.pop_back();
        } else if (Validator::is_valid(s)) {
            s.remove_last_char();
            StaticString keys = s.to_keys(config.method);
            composition_buffer.clear();
            for (char32_t k : keys)
                composition_buffer.append_key(k, config.commit_threshold);
        } else {
            composition_buffer.pop_back();
        }
        if (composition_buffer.get_raw().empty()) {
            result = build_result(std::u32string_view(U""));
            reclaim_from_history(config.method);
            return true;
        }
        result = process_key(0, mods);
        return true;
    } else {
        if (reclaim_from_history(config.method)) {
            return handle_backspace(key, mods, result);
        }
    }
    result = EngineResult{};
    return true;
}

/**
 * @brief Attempts to pop the last item from history and load it into the active composition_buffer.get_raw().
 *
 * If the item is a boundary, it reclaim the word before it as well.
 *
 * @param method The current input method for canonicalization.
 * @return True if something was reclaimed.
 */
bool Engine::reclaim_from_history(InputMethod method) {
    auto recovered_opt = context_tracker.reclaim_last_word();
    if (!recovered_opt.has_value())
        return false;

    std::u32string recovered = recovered_opt.value();
    char32_t rc = recovered[0];
    bool is_boundary = (recovered.size() == 1 && InputDispatcher::is_word_boundary(rc));

    if (is_boundary) {
        // If we reclaimed a boundary, we just set it as the active state
        composition_buffer.set_raw(recovered);
        state.last_committed_text = recovered;
        state.last_boundary_key = rc;
    } else {
        // Context Validation: If the reclaimed word does not look like a valid structure
        // after being recovered, it might be due to a desync. If it's a completely invalid
        // combination that neither parses as English nor valid VN, we might optionally discard it.
        // However, for consistency, we always reclaim the literal string as raw keys.
        
        // Re-parse the word string into canonical keys
        Syllable s = SyllableParser::parse(recovered);
        StaticString keys = s.to_keys(method);
        composition_buffer.set_raw(keys.view());
        state.last_committed_text = recovered;
        LOTUS_LOG_DEBUG(format_log_message("BACKSPACE", "Reclaimed word: '" + unicode::to_utf8(recovered) + "'"));
    }
    return true;
}

/**
 * @brief Handles word boundaries (space, enter, punctuation).
 *
 * Triggers shortcut expansion and English word restoration.
 *
 * @param key The current key.
 * @param result OUT: The engine result to populate.
 * @return True if the key was a boundary and was handled.
 */
bool Engine::handle_boundary(char32_t key, EngineResult& result) {
    if (!InputDispatcher::is_word_boundary(key))
        return false;

    std::string raw_word = unicode::to_utf8(composition_buffer.get_raw().view());
    if (is_likely_english(raw_word)) {
        StaticString output = composition_buffer.get_raw();
        output.push_back(key);
        result = build_result(output.view());
        result.action = EngineAction::RESTORE;
        reset();
        state.last_boundary_key = key;
        return true;
    }

    // Check if the boundary is a non-word breaking symbol that should clear context.
    // Standard sentence punctuation like .,!?;: and quotes/brackets might be safe to keep in history
    // but math symbols and special characters like @, #, $, +, = etc. should invalidate history.
    bool is_safe_boundary = false;
    for (char32_t c : SAFE_BOUNDARY_KEYS) {
        if (c == key) {
            is_safe_boundary = true;
            break;
        }
    }

    if (!is_safe_boundary) {
        context_tracker.clear();
    } else {
        commit_syllable_to_history(key);
    }

    if (handle_shortcuts(key, result))
        return true;

    result.action = EngineAction::PASS;
    result.count = 1;
    result.chars[0] = key;
    result.backspace = 0;
    reset();
    state.last_boundary_key = key;
    if (ContextTracker::is_sentence_ending(key))
        context_tracker.set_at_sentence_start(true);
    else if (key != ' ' && key != '\t')
        context_tracker.set_at_sentence_start(false);
    return true;
}

/**
 * @brief Pops the last word from ContextTracker, uses Syllable::to_keys() to reverse it back to raw keys, and populates CompositionBuffer.
 * @return EngineResult indicating the transformation to restore the text to the UI.
 */
EngineResult Engine::reclaim_last_word() {
    auto recovered_opt = context_tracker.reclaim_last_word();
    if (!recovered_opt.has_value()) {
        return EngineResult{};
    }

    std::u32string recovered = recovered_opt.value();
    char32_t rc = recovered[0];
    bool is_boundary = (recovered.size() == 1 && InputDispatcher::is_word_boundary(rc));

    if (is_boundary) {
        composition_buffer.set_raw(recovered);
        state.last_committed_text = recovered;
        state.last_boundary_key = rc;
    } else {
        Syllable s = SyllableParser::parse(recovered);
        StaticString keys = s.to_keys(config.method);
        composition_buffer.set_raw(keys.view());
        state.last_committed_text = recovered;
    }

    EngineResult result{};
    result.action = EngineAction::TRANSFORM;
    result.backspace = 0; // We assume the caller handles UI backspacing or this is used in a specific API context
    result.count = state.last_committed_text.size();
    for (size_t i = 0; i < result.count && i < 128; i++) {
        result.chars[i] = state.last_committed_text[i];
    }
    return result;
}

/**
 * @brief Processes a sequence of characters.
 * @param utf8_str The UTF-8 encoded string to process.
 * @return EngineResult the final result of processing the string.
 */
EngineResult Engine::process_string(const std::string& utf8_str) {
    EngineResult result{};
    std::u32string u32_str = unicode::to_utf32(utf8_str);
    for (char32_t c : u32_str) {
        Modifiers mods; // Default modifiers
        result = process_key(c, mods);
    }
    return result;
}

/**
 * @brief Checks and expands text shortcuts.
 *
 * @param key The boundary key that triggered expansion.
 * @param result OUT: The result containing the expanded text.
 * @return True if a shortcut was matched and expanded.
 */
bool Engine::handle_shortcuts(char32_t key, EngineResult& result) {
    if (shortcut_manager.handle(key, composition_buffer.get_raw().view(), result, config.macro_mode)) {
        reset();
        state.last_boundary_key = key;
        state.last_committed_text.clear();
        for (int i = 0; i < result.count; i++) state.last_committed_text.push_back(result.chars[i]);
        return true;
    }
    return false;
}

/**
 * @brief Handles smart features like double-space to period and auto-capitalize.
 *
 * @param key IN/OUT: The current key.
 * @param mods Keyboard modifiers.
 * @param result OUT: The engine result to populate.
 * @return True if a smart feature was triggered and handled.
 */
bool Engine::handle_smart_typing(char32_t& key, const Modifiers& mods, EngineResult& result) {
    (void)mods;
    bool handled = SmartTyping::handle(key, config.double_space_to_period, config.auto_capitalize, state.last_boundary_key, context_tracker.is_at_sentence_start(), composition_buffer.get_raw().view(), result, state.last_committed_text);
    if (handled) {
        state.last_boundary_key = ' ';
        context_tracker.set_at_sentence_start(true);
    }
    return handled;
}





/**
 * @brief Helper to wrap a transformed string into an EngineResult.
 *
 * @param final_u32 The final transformed character sequence.
 * @return EngineResult indicating a replacement action.
 */
EngineResult Engine::build_result(std::u32string_view final_u32) {
    EngineResult result{};
    result.action = EngineAction::TRANSFORM;
    result.backspace = (uint8_t)state.last_committed_text.size();
    result.count = (uint8_t)std::min((size_t)32, final_u32.size());
    for (int i = 0; i < result.count; i++)
        result.chars[i] = final_u32[i];

    LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "Result: BS=" + std::to_string((int)result.backspace) +
                    ", Count=" + std::to_string((int)result.count) + ", PrevText='" +
                    unicode::to_utf8(state.last_committed_text) + "'"));

    state.last_committed_text = std::u32string(final_u32);
    return result;
}

/**
 * @brief Determines if the current composition_buffer.get_raw() likely represents an English word.
 *
 * @param word The raw key sequence.
 * @return True if the word should be preserved as English.
 */
bool Engine::is_likely_english(const std::string& word) const {
    if (config.spell_check && !dictionary.empty()) {
        std::string lower = word;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (dictionary.find(lower) == dictionary.end()) {
            return true;
        }
    }
    bool is_valid_vn = composition_buffer.is_likely_english(unicode::to_utf32_static(word).view(), config.method, config.free_w, config.allow_non_standard_initials, config.tone_less, config.mark_less);
    return context_tracker.is_likely_english(word, is_valid_vn);
}

}  // namespace lotus_core
