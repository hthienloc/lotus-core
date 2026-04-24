/**
 * @file engine.cpp
 * @brief Core input method engine implementation.
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

namespace lotus_core {

// ============================================================================
// [ Internal Helpers ]
// ============================================================================

/**
 * @brief Checks if a character marks the end of a sentence.
 * @param c UTF-32 character.
 * @return True if it's a sentence-ending punctuation or newline.
 */
static bool is_sentence_ending(char32_t c) {
    return c == '.' || c == '!' || c == '?' || c == '\n' || c == '\r';
}

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
 * Initializes the engine with default Telex method and all smart features disabled.
 */
Engine::Engine()
    : last_modifier_key(0),
      last_boundary_key(0),
      at_sentence_start(true),
      method(InputMethod::TELEX),
      double_space_to_period(false),
      auto_capitalize(false) {}

/**
 * @brief Registers a new text shortcut.
 * @param trigger The short key sequence.
 * @param replacement The full string to expand into.
 */
void Engine::add_shortcut(const std::string& trigger, const std::string& replacement) {
    shortcut_manager.add_shortcut(trigger, replacement);
}

/**
 * @brief Resets the internal engine state (buffer, markers, committed text).
 * Does NOT clear the word history.
 */
void Engine::reset() {
    buffer.clear();
    last_modifier_key = 0;
    last_boundary_key = 0;
    last_committed_text.clear();
}

/**
 * @brief Completely clears the engine state, including word history.
 * Used when the cursor moves to a completely new, unrelated position.
 */
void Engine::clear_all() {
    reset();
    word_history.clear();
}

/**
 * @brief Reconstructs the engine state from a block of surrounding text.
 *
 * Used for synchronization when the cursor moves or text is modified externally.
 * @param text The surrounding context string.
 */
void Engine::rebuild_from_text(const std::string& text) {
    reset();
    at_sentence_start = true;
    word_history.clear();
    if (text.empty())
        return;

    std::u32string full_text = unicode::to_utf32(text);
    if (!full_text.empty()) {
        char32_t last_c = full_text.back();
        if (is_sentence_ending(last_c))
            at_sentence_start = true;
        else if (last_c != ' ' && last_c != '\t')
            at_sentence_start = false;
        else {
            for (int i = (int)full_text.size() - 1; i >= 0; --i) {
                if (is_sentence_ending(full_text[i])) {
                    at_sentence_start = true;
                    break;
                }
                if (full_text[i] != ' ' && full_text[i] != '\t') {
                    at_sentence_start = false;
                    break;
                }
            }
        }
    }

    std::vector<std::string> words;
    std::string current_word;
    for (char32_t c : unicode::to_utf32(text)) {
        bool is_boundary =
            (c == ' ' || c == '\r' || c == '\n' || (c < 128 && (ispunct((int)c) || c == '\t')));
        if (is_boundary) {
            if (!current_word.empty()) {
                words.push_back(current_word);
                current_word.clear();
            }
            words.push_back(unicode::to_utf8(std::u32string(1, c)));
        } else {
            current_word += unicode::to_utf8(std::u32string(1, c));
        }
    }
    if (!current_word.empty()) {
        words.push_back(current_word);
    }

    if (words.empty())
        return;

    std::string last_word = words.back();
    words.pop_back();

    for (const auto& w : words) {
        word_history.push(unicode::to_utf32(w));
        std::u32string w32 = unicode::to_utf32(w);
        if (!w32.empty()) {
            char32_t last_c = w32.back();
            bool is_boundary = (last_c == ' ' || last_c == '\r' || last_c == '\n' ||
                                (last_c < 128 && (ispunct((int)last_c) || last_c == '\t')));
            if (is_boundary) {
                last_boundary_key = last_c;
            } else {
                last_boundary_key = 0;
            }
        }
    }

    Syllable s = SyllableParser::parse(unicode::to_utf32(last_word));
    std::vector<char32_t> keys = s.to_keys(method);
    buffer.clear();
    for (char32_t k : keys)
        buffer.push_back(k);
    last_committed_text = unicode::to_utf32(last_word);

    std::u32string last32 = unicode::to_utf32(last_word);
    if (!last32.empty()) {
        char32_t last_c = last32.back();
        bool is_boundary = (last_c == ' ' || last_c == '\r' || last_c == '\n' ||
                            (last_c < 128 && (ispunct((int)last_c) || last_c == '\t')));
        if (is_boundary) {
            last_boundary_key = last_c;
            word_history.push(last32);
            buffer.clear();
            last_committed_text.clear();
        } else {
            last_boundary_key = 0;
        }
    }
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
    apply_std_uo(key);

    EngineResult res{};

    if (handle_navigation(key, mods, res))
        return res;
    if (handle_backspace(key, mods, res))
        return res;
    if (handle_smart_typing(key, mods, res) && res.action != 0)
        return res;

    if (key != 0 && buffer.empty()) {
        last_committed_text.clear();
    }

    if (handle_boundary(key, res))
        return res;
    if (handle_modifier_escape(key, res))
        return res;

    if (key != 0) {
        buffer.push_back(key);
        at_sentence_start = false;
    }

    std::string raw_word = unicode::to_utf8(buffer);
    return apply_im_pipeline(key, raw_word);
}

/**
 * @brief Applies the IM transformations, parsing, linguistic validation, and state updating.
 *
 * Coordinates the full pipeline from raw keys to a valid Vietnamese syllable.
 *
 * @param key The current key pressed.
 * @param raw_word The current raw buffer content as UTF-8.
 * @return EngineResult the final result of the transformation pipeline.
 */
EngineResult Engine::apply_im_pipeline(char32_t key, std::string& raw_word) {
    std::string current_str = raw_word;
    Tone tone_state = Tone::NONE;
    bool key_consumed = (key == 0);

    if (key != 0)
        last_boundary_key = 0;

    if (method == InputMethod::TELEX) {
        apply_telex_modifiers(current_str, key, key_consumed, tone_state);
    } else {
        apply_vni_modifiers(current_str, key, key_consumed, tone_state);
    }

    LOTUS_LOG_DEBUG("[Pipeline] After IM: " + current_str +
                    " (Tone: " + std::to_string(static_cast<int>(tone_state)) +
                    ", Consumed: " + (key_consumed ? "Y" : "N") + ")");

    if (!key_consumed)
        last_modifier_key = 0;

    current_str = unicode::normalize_nfc(current_str);
    Syllable s = SyllableParser::parse(unicode::to_utf32(current_str), this->allow_non_standard_initials);
    if (tone_state != Tone::NONE)
        s.tone = tone_state;

    LOTUS_LOG_DEBUG("[Pipeline] Parsed: " + s.to_string(tone_style));

    // THE GATE (Post-IM): Initial Consonant Validation
    // This is the first line of defense against English words.
    // In Vietnamese, everything before the first vowel MUST be a valid initial consonant sequence.
    bool has_valid_initial = true;
    if (!current_str.empty()) {
        std::u32string u32_curr = unicode::to_utf32(current_str);
        size_t first_vowel = std::u32string::npos;
        for (size_t i = 0; i < u32_curr.size(); ++i) {
            if (SyllableParser::is_vowel(u32_curr[i])) {
                first_vowel = i;
                break;
            }
        }
        std::u32string prefix =
            (first_vowel == std::u32string::npos) ? u32_curr : u32_curr.substr(0, first_vowel);
        if (!prefix.empty() && !Validator::is_valid_initial(unicode::to_lower(prefix), this->allow_non_standard_initials)) {
            has_valid_initial = false;
        }
    }

    // If the transformation resulted in an invalid prefix, favor the raw English input.
    if (auto_restore && !has_valid_initial && !key_consumed) {
        LOTUS_LOG_DEBUG("[Pipeline] Restore: Invalid initial prefix");
        return make_transformation_result(buffer);
    }

    // Second Gate: Structural Vietnamese Validity
    // If the resulting syllable violates Vietnamese spelling rules AND looks like English,
    // we restore the raw buffer contents.
    bool is_valid_vn = Validator::is_valid(s, nullptr, this->allow_non_standard_initials);
    bool is_eng = Linguistics::is_likely_english(raw_word);

    LOTUS_LOG_DEBUG("[Pipeline] Gates: ValidVN=" + std::string(is_valid_vn ? "Y" : "N") +
                    ", LikelyEng=" + std::string(is_eng ? "Y" : "N"));

    if (auto_restore && !is_valid_vn && !key_consumed && is_eng) {
        LOTUS_LOG_DEBUG("[Pipeline] Restore: Invalid structure & likely English");
        return make_transformation_result(buffer);
    }

    std::string final_v_word = s.to_string(tone_style);

    // We explicitly DO NOT normalize the buffer here.
    // The Engine class uses `buffer` to store the EXACT, raw sequence of keystrokes 
    // typed by the user without normalization.
    // This ensures `auto_restore` restores exactly what was typed and allows 
    // `Linguistics::is_likely_english` to accurately evaluate the original key sequence.

    LOTUS_LOG_DEBUG("[Pipeline] Final: " + final_v_word);
    return make_transformation_result(unicode::to_utf32(final_v_word));
}

/**
 * @brief Applies standard UO key mappings.
 */
void Engine::apply_std_uo(char32_t& key) {
    if (!std_uo)
        return;
    if (key == '[')
        key = U'ư';
    else if (key == ']')
        key = U'ơ';
    else if (key == '{')
        key = U'Ư';
    else if (key == '}')
        key = U'Ơ';
}

/**
 * @brief Handles navigation and escape keys.
 */
bool Engine::handle_navigation(char32_t key, const Modifiers& mods, EngineResult& result) {
    (void)result;
    bool is_nav_key = key == constants::KEY_UP || key == constants::KEY_DOWN ||
                      key == constants::KEY_LEFT || key == constants::KEY_RIGHT ||
                      key == constants::KEY_HOME || key == constants::KEY_END ||
                      key == constants::KEY_PAGE_UP || key == constants::KEY_PAGE_DOWN ||
                      key == constants::KEY_TAB || key == constants::KEY_ESC;
                      
    bool is_ctrl_nav = mods.ctrl && (key == constants::KEY_LEFT || key == constants::KEY_RIGHT || key == constants::KEY_HOME || key == constants::KEY_END);

    if (is_nav_key || is_ctrl_nav) {
        clear_all();
        return true;
    }
    return false;
}

/**
 * @brief Handles triple-tap escape logic (e.g., typing 'aaa' results in 'aa').
 */
bool Engine::handle_modifier_escape(char32_t key, EngineResult& result) {
    if (key != 0 && key == last_modifier_key && !buffer.empty()) {
        char32_t lk = unicode::to_lower(key);
        bool revertible =
            (constants::TELEX_MARKERS.find(static_cast<char>(lk)) != std::string_view::npos);
        if (revertible) {
            buffer.push_back(key);
            last_modifier_key = 0;
            at_sentence_start = false;
            last_boundary_key = 0;
            std::u32string out = buffer;
            if (!out.empty())
                out.pop_back();
            result = make_transformation_result(out);
            return true;
        }
    }
    return false;
}

/**
 * @brief Handles backspace key logic.
 *
 * If the buffer is not empty, it attempts to reconstruct the previous syllable state.
 * If the buffer is empty, it attempts to recover the last committed word from history.
 *
 * @param key The key pressed (8 or 127).
 * @param mods Keyboard modifiers.
 * @param result OUT: The engine result to populate.
 * @return True if the key was a backspace and was handled.
 */
bool Engine::handle_backspace(char32_t key, const Modifiers& mods, EngineResult& result) {
    if (key != 8 && key != 127)
        return false;
    if (!buffer.empty()) {
        std::string word = unicode::to_utf8(last_committed_text);
        Syllable s = SyllableParser::parse(unicode::to_utf32(word));
        
        bool is_english_fallback = auto_restore && is_english_word(unicode::to_utf8(buffer));

        if (backspace_style == BackspaceStyle::KEYSTROKE || is_english_fallback) {
            buffer.pop_back();
        } else if (Validator::is_valid(s)) {
            s.remove_last_char();
            std::vector<char32_t> keys = s.to_keys(method);
            buffer.clear();
            for (char32_t k : keys)
                buffer.push_back(k);
        } else {
            buffer.pop_back();
        }
        if (buffer.empty()) {
            result = make_transformation_result(U"");
            reclaim_from_history(method);
            return true;
        }
        result = process_key(0, mods);
        return true;
    } else {
        if (reclaim_from_history(method)) {
            return handle_backspace(key, mods, result);
        }
    }
    result = EngineResult{};
    return true;
}

/**
 * @brief Attempts to pop the last item from history and load it into the active buffer.
 *
 * If the item is a boundary, it reclaim the word before it as well.
 *
 * @param method The current input method for canonicalization.
 * @return True if something was reclaimed.
 */
bool Engine::reclaim_from_history(InputMethod method) {
    auto recovered = word_history.pop();
    if (recovered.empty())
        return false;

    char32_t rc = recovered[0];
    bool is_boundary = (recovered.size() == 1 && (rc == ' ' || rc == '\t' || rc == '\r' ||
                                                  rc == '\n' || (rc < 128 && ispunct((int)rc))));

    if (is_boundary) {
        // If we reclaimed a boundary, we just set it as the active state
        buffer = recovered;
        last_committed_text = recovered;
        last_boundary_key = rc;
    } else {
        // Context Validation: If the reclaimed word does not look like a valid structure
        // after being recovered, it might be due to a desync. If it's a completely invalid
        // combination that neither parses as English nor valid VN, we might optionally discard it.
        // However, for consistency, we always reclaim the literal string as raw keys.
        
        // Re-parse the word string into canonical keys
        Syllable s = SyllableParser::parse(recovered);
        std::vector<char32_t> keys = s.to_keys(method);
        buffer.assign(keys.begin(), keys.end());
        last_committed_text = recovered;
        LOTUS_LOG_DEBUG("[Backspace] Reclaimed word: '" + unicode::to_utf8(recovered) + "'");
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
    bool is_boundary = (key == ' ' || key == '\r' || key == '\n' ||
                        (key < 128 && (ispunct((int)key) || key == '\t')));
    if (!is_boundary)
        return false;

    std::string raw_word = unicode::to_utf8(buffer);
    if (is_english_word(raw_word)) {
        std::u32string output = buffer;
        output.push_back(key);
        result = make_transformation_result(output);
        result.action = 2;
        reset();
        last_boundary_key = key;
        return true;
    }

    // Check if the boundary is a non-word breaking symbol that should clear context.
    // Standard sentence punctuation like .,!?;: and quotes/brackets might be safe to keep in history
    // but math symbols and special characters like @, #, $, +, = etc. should invalidate history.
    bool is_safe_boundary = (key == ' ' || key == '\r' || key == '\n' || key == '\t' ||
                             key == '.' || key == ',' || key == '!' || key == '?' ||
                             key == ';' || key == ':' || key == '"' || key == '\'' ||
                             key == '(' || key == ')' || key == '[' || key == ']' ||
                             key == '{' || key == '}' || key == '-' || key == '_');

    if (!is_safe_boundary) {
        word_history.clear();
    } else {
        // Push the transformed word and THEN the boundary to history for exact sync
        if (!buffer.empty()) {
            std::string current_str = unicode::to_utf8(buffer);
            Tone tone_state = Tone::NONE;
            bool consumed = false;

            if (method == InputMethod::TELEX) {
                apply_telex_modifiers(current_str, 0, consumed, tone_state);
            } else {
                apply_vni_modifiers(current_str, 0, consumed, tone_state);
            }

            current_str = unicode::normalize_nfc(current_str);
            Syllable s = SyllableParser::parse(unicode::to_utf32(current_str));
            if (tone_state != Tone::NONE)
                s.tone = tone_state;
            word_history.push(unicode::to_utf32(s.to_string(tone_style)));
        }
        word_history.push({key});
    }

    if (handle_shortcuts(key, result))
        return true;

    result.action = 0;
    result.count = 1;
    result.chars[0] = key;
    result.backspace = 0;
    reset();
    last_boundary_key = key;
    if (is_sentence_ending(key))
        at_sentence_start = true;
    else if (key != ' ' && key != '\t')
        at_sentence_start = false;
    return true;
}

/**
 * @brief Checks and expands text shortcuts.
 *
 * @param key The boundary key that triggered expansion.
 * @param result OUT: The result containing the expanded text.
 * @return True if a shortcut was matched and expanded.
 */
bool Engine::handle_shortcuts(char32_t key, EngineResult& result) {
    if (shortcut_manager.handle(key, buffer, result, macro_mode)) {
        reset();
        last_boundary_key = key;
        last_committed_text.clear();
        for (int i = 0; i < result.count; i++) last_committed_text.push_back(result.chars[i]);
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
    bool handled = SmartTyping::handle(key, double_space_to_period, auto_capitalize, last_boundary_key, at_sentence_start, buffer, result, last_committed_text);
    if (handled) {
        last_boundary_key = ' ';
        at_sentence_start = true;
    }
    return handled;
}

/**
 * @brief Applies Telex-specific rules and transformations to the current buffer.
 *
 * Implements a single-pass transformation pipeline including flexible consonants,
 * flexible vowels, hooks, and tone marks.
 *
 * @param current_str IN/OUT: The string to modify based on Telex rules.
 * @param key The current key pressed.
 * @param key_consumed OUT: Set to true if the key triggered a transformation.
 * @param tone_state OUT: The identified tone for the current word.
 */
void Engine::apply_telex_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                                   Tone& tone_state) {
    const std::u32string& u32 = buffer;
    if (u32.empty())
        return;

    const std::string raw_str = unicode::to_utf8(u32);
    if (Linguistics::is_definite_english(raw_str)) {
        current_str = raw_str;
        return;
    }

    if (unicode::to_lower(u32[0]) == 'w' && free_w != FreeWOption::ALWAYS) {
        current_str = raw_str;
        return;
    }

    char32_t lk = unicode::to_lower(key);
    std::vector<bool> to_strip(u32.size(), false);

    // Single pass to gather all candidate indices for transformations.
    std::map<char32_t, std::vector<size_t>> indices;
    std::vector<size_t> tone_indices;
    size_t ir_len = Validator::find_longest_initial(u32, 0);

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t l = unicode::to_lower(u32[i]);
        if (l == 'd' || l == 'a' || l == 'e' || l == 'o' || l == 'u' || l == 'w') {
            indices[l].push_back(i);
        }

        if (i > 0 && i >= ir_len) {
            if (l == 's' || l == 'f' || l == 'r' || l == 'x' || l == 'j' || l == 'z' || l == '0') {
                tone_indices.push_back(i);
            }
        }
    }

    std::u32string u32_copy = u32;

    // Stage 1: Flexible Consonants (dd -> đ)
    if ((!key_consumed || key == 0) && indices['d'].size() >= 2) {
        size_t first = indices['d'][0], last = indices['d'].back();
        u32_copy[first] = (u32[first] == 'D') ? U'Đ' : U'đ';
        to_strip[last] = true;
        if (lk == 'd' && key != 0) {
            last_modifier_key = key;
            key_consumed = true;
        }
    }

    // Stage 2: Flexible Vowels (aa -> â, ee -> ê, oo -> ô)
    if (!key_consumed || key == 0) {
        auto try_flex = [&](char32_t base, char32_t target_l, char32_t target_u) {
            auto& idxs = indices[base];
            if (idxs.size() >= 2) {
                size_t first = idxs[0], last = idxs.back();
                u32_copy[first] = (u32[first] == unicode::to_upper(base)) ? target_u : target_l;
                to_strip[last] = true;
                if (lk == base && key != 0) {
                    last_modifier_key = key;
                    key_consumed = true;
                }
                return true;
            }
            return false;
        };
        try_flex('a', U'â', U'Â');
        try_flex('e', U'ê', U'Ê');
        try_flex('o', U'ô', U'Ô');
    }

    // Stage 3: Combined Hooks (uo, uaw, aw, ow, uw)
    if (!indices['w'].empty()) {
        size_t u_pos = indices['u'].empty() ? std::u32string::npos : indices['u'][0];
        size_t o_pos = indices['o'].empty() ? std::u32string::npos : indices['o'][0];
        size_t a_pos = indices['a'].empty() ? std::u32string::npos : indices['a'][0];
        bool hooked = false;

        if (u_pos != std::u32string::npos && o_pos != std::u32string::npos) {
            u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
            u32_copy[o_pos] = (u32[o_pos] == 'O') ? U'Ơ' : U'ơ';
            hooked = true;
        } else if (u_pos != std::u32string::npos && a_pos != std::u32string::npos) {
            u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
            hooked = true;
        } else {
            if (u_pos != std::u32string::npos) {
                u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
                hooked = true;
            }
            if (o_pos != std::u32string::npos) {
                if (a_pos == std::u32string::npos) {
                    u32_copy[o_pos] = (u32[o_pos] == 'O') ? U'Ơ' : U'ơ';
                    hooked = true;
                }
            }
            if (a_pos != std::u32string::npos) {
                u32_copy[a_pos] = (u32[a_pos] == 'A') ? U'Ă' : U'ă';
                hooked = true;
            }
        }

        if (hooked) {
            for (size_t idx : indices['w'])
                if (idx > 0)
                    to_strip[idx] = true;
            if (lk == 'w' && key != 0) {
                last_modifier_key = key;
                key_consumed = true;
            }
        }
    }

    // Stage 4: Standalone W -> ư
    if (!key_consumed && lk == 'w' && free_w != FreeWOption::OFF) {
        bool can_transform = (free_w == FreeWOption::ALWAYS) || (u32.size() > 1);
        if (can_transform) {
            bool has_real_v = false;
            for (auto c : u32)
                if (SyllableParser::is_vowel(c) && unicode::to_lower(c) != 'w')
                    has_real_v = true;
            if (!has_real_v && !indices['w'].empty()) {
                size_t first_w = indices['w'][0];
                u32_copy[first_w] = (u32[first_w] == 'W') ? U'Ư' : U'ư';
                key_consumed = true;
                last_modifier_key = key;
            }
        }
    }

    // Stage 5: Tone Marks
    if (!tone_indices.empty()) {
        std::vector<bool> is_literal_marker(u32.size(), false);
        std::vector<size_t> potential_active_indices;

        // Pass 1: Identify all escape pairs (xx -> x)
        for (size_t i = 0; i < tone_indices.size(); ++i) {
            size_t idx = tone_indices[i];
            char32_t current_key = unicode::to_lower(u32[idx]);

            if (i + 1 < tone_indices.size() &&
                current_key == unicode::to_lower(u32[tone_indices[i + 1]])) {
                // Escape pair!
                is_literal_marker[idx] = true;         // First one is literal
                to_strip[tone_indices[i + 1]] = true;  // Second one is stripped
                i++;
            } else {
                potential_active_indices.push_back(idx);
            }
        }

        // Pass 2: Apply Linguistic Gating to remaining potential markers
        std::vector<size_t> active_tone_indices;
        for (size_t idx : potential_active_indices) {
            char32_t current_key = unicode::to_lower(u32[idx]);

            // Build base string including literalized markers
            std::u32string base_u32;
            for (size_t j = 0; j < idx; ++j) {
                if (!to_strip[j] &&
                    (is_literal_marker[j] || std::find(tone_indices.begin(), tone_indices.end(),
                                                       j) == tone_indices.end())) {
                    base_u32 += u32_copy[j];
                }
            }

            bool is_literal = false;
            if (!base_u32.empty() && current_key != 'z' && current_key != '0') {
                char32_t last_base = unicode::to_lower(base_u32.back());
                // If word ends in impossible final, marker is literal
                if (last_base == 's' || last_base == 'r' || last_base == 'x' || last_base == 'f' ||
                    last_base == 'j' || last_base == 'w') {
                    is_literal = true;
                }

                // LINGUISTIC GATING: Tone markers MUST follow a vowel in Vietnamese.
                // If the base word has no vowels, this marker must be literal.
                bool has_vowel = false;
                for (char32_t cp : base_u32) {
                    if (SyllableParser::is_vowel(cp)) {
                        has_vowel = true;
                        break;
                    }
                }
                if (!has_vowel)
                    is_literal = true;
            }

            if (!is_literal) {
                active_tone_indices.push_back(idx);
            }
        }

        // Pass 3: Final tone application
        tone_state = Tone::NONE;
        for (size_t idx : active_tone_indices) {
            char32_t marker = unicode::to_lower(u32[idx]);

            // Map Telex keys to Tone enum
            if (marker == 's')
                tone_state = Tone::ACUTE;
            else if (marker == 'f')
                tone_state = Tone::GRAVE;
            else if (marker == 'r')
                tone_state = Tone::HOOK;
            else if (marker == 'x')
                tone_state = Tone::TILDE;
            else if (marker == 'j')
                tone_state = Tone::DOT;
            else if (marker == 'z' || marker == '0')
                tone_state = Tone::NONE;

            to_strip[idx] = true;
        }
    }

    // Stage 6: Final Execution
    std::u32string final_u32;
    for (size_t i = 0; i < u32.size(); ++i)
        if (!to_strip[i])
            final_u32 += u32_copy[i];
    current_str = unicode::to_utf8(final_u32);
}

/**
 * @brief Applies VNI-specific rules and transformations to the current buffer.
 *
 * Implements a high-performance single-pass scanner to identify tones and markers.
 *
 * @param current_str IN/OUT: The string to modify based on VNI rules.
 * @param key The current key pressed.
 * @param key_consumed OUT: Set to true if the key triggered a transformation.
 * @param tone_state OUT: The identified tone for the current word.
 */
void Engine::apply_vni_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                                 Tone& tone_state) {
    const std::u32string& u32 = buffer;
    if (u32.empty())
        return;

    const std::string raw_str = unicode::to_utf8(u32);
    if (Linguistics::is_definite_english(raw_str))
        return;

    std::u32string u32_copy = u32;
    std::vector<bool> to_strip(u32.size(), false);

    bool has_mod = false;
    char32_t lk = unicode::to_lower(key);

    bool has_6 = false;
    bool has_7 = false;
    bool has_8 = false;
    bool has_9 = false;
    bool has_a = false;

    // Single pass to gather digits and flags
    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t k = u32[i];
        if (k == 'a' || k == 'A' || k == U'ă' || k == U'Ă' || k == U'â' || k == U'Â') has_a = true;

        if (k >= '0' && k <= '9') {
            has_mod = true;
            to_strip[i] = true;

            // Tier 3: Tones
            if (k >= '1' && k <= '5') tone_state = static_cast<Tone>(k - '0');
            else if (k == '0') tone_state = Tone::NONE;
            else if (k == '6') has_6 = true;
            else if (k == '7') has_7 = true;
            else if (k == '8') has_8 = true;
            else if (k == '9') has_9 = true;

            if (k == lk && !key_consumed) {
                last_modifier_key = key;
                key_consumed = true;
            }
        }
    }

    if (!has_mod) return;

    // One-pass character transformation in tier priority
    for (size_t i = 0; i < u32_copy.size(); ++i) {
        char32_t c = u32_copy[i];

        // Tier 1: Base/Hooks
        if (has_9) {
            if (c == 'd') c = U'đ';
            else if (c == 'D') c = U'Đ';
        }
        if (has_8) {
            if (c == 'a') c = U'ă';
            else if (c == 'A') c = U'Ă';
        }
        if (has_7) {
            if (c == 'u') c = U'ư';
            else if (c == 'U') c = U'Ư';
            if (!has_a) {
                if (c == 'o') c = U'ơ';
                else if (c == 'O') c = U'Ơ';
            }
        }

        // Tier 2: Circumflex
        if (has_6) {
            if (c == 'a' || c == U'ă') c = U'â';
            else if (c == 'A' || c == U'Ă') c = U'Â';
            else if (c == 'e') c = U'ê';
            else if (c == 'E') c = U'Ê';
            else if (c == 'o' || c == U'ơ') c = U'ô';
            else if (c == 'O' || c == U'Ơ') c = U'Ô';
        }

        u32_copy[i] = c;
    }

    // Assembly
    std::u32string final_u32;
    for (size_t i = 0; i < u32.size(); ++i) {
        if (!to_strip[i]) {
            final_u32 += u32_copy[i];
        }
    }

    current_str = unicode::to_utf8(final_u32);
}

/**
 * @brief Helper to wrap a transformed string into an EngineResult.
 *
 * @param final_u32 The final transformed character sequence.
 * @return EngineResult indicating a replacement action.
 */
EngineResult Engine::make_transformation_result(const std::u32string& final_u32) {
    EngineResult result{};
    result.action = 1;
    result.backspace = (uint8_t)last_committed_text.size();
    result.count = (uint8_t)std::min((size_t)32, final_u32.size());
    for (int i = 0; i < result.count; i++)
        result.chars[i] = final_u32[i];

    LOTUS_LOG_DEBUG("[Pipeline] Result: BS=" + std::to_string((int)result.backspace) +
                    ", Count=" + std::to_string((int)result.count) + ", PrevText='" +
                    unicode::to_utf8(last_committed_text) + "'");

    last_committed_text = final_u32;
    return result;
}

/**
 * @brief Determines if the current buffer likely represents an English word.
 *
 * @param word The raw key sequence.
 * @return True if the word should be preserved as English.
 */
bool Engine::is_english_word(const std::string& word) const {
    if (Linguistics::is_on_whitelist(word))
        return true;
    std::string transformed = word;
    Tone tone = Tone::NONE;
    bool consumed = false;
    if (method == InputMethod::TELEX) {
        const_cast<Engine*>(this)->apply_telex_modifiers(transformed, 0, consumed, tone);
    } else {
        const_cast<Engine*>(this)->apply_vni_modifiers(transformed, 0, consumed, tone);
    }
    Syllable s = SyllableParser::parse(unicode::to_utf32(transformed));
    if (tone != Tone::NONE)
        s.tone = tone;
    if (Validator::is_valid(s))
        return false;
    return Linguistics::is_likely_english(word);
}

}  // namespace lotus_core
