#pragma once

#include "lotus_engine/linguistics.h"
#include "lotus_engine/types.h"
#include "lotus_engine/shortcut_manager.h"

#include <functional>
#include <string>
#include <vector>

namespace lotus_engine {

/**
 * @brief The core Vietnamese input method engine.
 *
 * Maintains composition state (buffer, tone, modifiers) and transforms
 * raw keystroke sequences into Vietnamese text according to TELEX or VNI
 * encoding rules. Returns @ref EngineResult values that instruct the frontend
 * on how many characters to delete and what to insert.
 */
class Engine {
   public:
    Engine();

    /** @brief Sets the active input method (TELEX or VNI). */
    void set_method(InputMethod method) { this->method = method; }
    /** @brief Returns the current input method. */
    InputMethod get_method() const { return method; }

    /** @brief Sets the tone placement style (Old or New). */
    void set_tone_style(ToneStyle style) { this->tone_style = style; }
    /** @brief Returns the current tone placement style. */
    ToneStyle get_tone_style() const { return tone_style; }

    /** @brief Sets the standalone 'w' handling option (TELEX). */
    void set_free_w(FreeWOption option) { this->free_w = option; }
    /** @brief Returns the current Free-W option. */
    FreeWOption get_free_w() const { return free_w; }

    /** @brief Enables or disables standard uo -> ươ transformation. */
    void set_std_uo(bool enabled) { this->std_uo = enabled; }
    /** @brief Returns whether the std_uo transformation is enabled. */
    bool get_std_uo() const { return std_uo; }

    /** @brief Enables or disables automatic English word restoration. */
    void set_auto_restore(bool enabled) { this->auto_restore = enabled; }
    /** @brief Returns whether auto-restore is enabled. */
    bool get_auto_restore() const { return auto_restore; }

    /** @brief Enables or disables double-space to period conversion. */
    void set_double_space_to_period(bool enabled) { this->double_space_to_period = enabled; }
    /** @brief Returns whether double-space to period is enabled. */
    bool get_double_space_to_period() const { return double_space_to_period; }

    /** @brief Enables or disables automatic capitalization after sentence boundaries. */
    void set_auto_capitalize(bool enabled) { this->auto_capitalize = enabled; }
    /** @brief Returns whether auto-capitalize is enabled. */
    bool get_auto_capitalize() const { return auto_capitalize; }

    /** @brief Sets whether the next input is at the start of a new sentence. */
    void set_at_sentence_start(bool enabled) { this->at_sentence_start = enabled; }
    /** @brief Returns whether the engine is currently at a sentence start. */
    bool get_at_sentence_start() const { return at_sentence_start; }

    /** @brief Enables or disables non-standard initials (z, w, j, f). */
    void set_allow_non_standard_initials(bool enabled) { this->allow_non_standard_initials = enabled; }
    /** @brief Returns whether non-standard initials are allowed. */
    bool get_allow_non_standard_initials() const { return allow_non_standard_initials; }

    /** @brief Sets the active macro expansion mode. */
    void set_macro_mode(MacroMode mode) { this->macro_mode = mode; }
    /** @brief Returns the current macro expansion mode. */
    MacroMode get_macro_mode() const { return macro_mode; }

    /** @brief Sets the active backspace style. */
    void set_backspace_style(BackspaceStyle style) { this->backspace_style = style; }
    /** @brief Returns the current backspace style. */
    BackspaceStyle get_backspace_style() const { return backspace_style; }

    /**
     * @brief Processes a single keypress and returns the required UI action.
     * @param key The UTF-32 codepoint of the pressed key.
     * @param mods The active modifier keys (Shift, CapsLock).
     * @return EngineResult The action to perform on the composition buffer.
     */
    EngineResult process_key(char32_t key, const Modifiers& mods);

    /**
     * @brief Clears the composition buffer and resets engine state.
     */
    void reset();

    /**
     * @brief Completely clears the engine state, including word history.
     */
    void clear_all();

    /**
     * @brief Registers a text expansion shortcut.
     * @param trigger The typed string that activates the shortcut (e.g., "vn").
     * @param replacement The expanded output string (e.g., "Việt Nam").
     */
    void add_shortcut(const std::string& trigger, const std::string& replacement);

    /**
     * @brief Reconstructs engine state from an existing committed text string.
     *
     * Used when the surrounding text context changes (e.g., after a cursor move).
     * @param text The existing word text to restore from (e.g., "xin chào").
     */
    void rebuild_from_text(const std::string& text);

   private:
    // Internal TELEX modifier application (single-pass, UTF-32 based).
    void apply_telex_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                               Tone& tone_state);
    // Internal VNI modifier application.
    void apply_vni_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                             Tone& tone_state);

    // Decomposed process_key helpers
    bool handle_backspace(char32_t key, const Modifiers& mods, EngineResult& result);
    bool handle_boundary(char32_t key, EngineResult& result);
    bool handle_shortcuts(char32_t key, EngineResult& result);
    bool handle_smart_typing(char32_t& key, const Modifiers& mods, EngineResult& result);
    bool handle_navigation(char32_t key, EngineResult& result);
    bool handle_modifier_escape(char32_t key, EngineResult& result);
    void apply_std_uo(char32_t& key);
    EngineResult apply_im_pipeline(char32_t key, std::string& raw_word);
    bool reclaim_from_history(InputMethod method);

    EngineResult make_transformation_result(const std::u32string& final_u32);

    std::u32string buffer;               ///< The current raw composition buffer (UTF-32).
    char32_t last_modifier_key = 0;      ///< The last consumed modifier key (for escape detection).
    std::u32string last_committed_text;  ///< The last committed text (for backspace recovery).
    ShortcutManager shortcut_manager;    ///< Registered text expansion shortcuts.
    WordHistory word_history;                      ///< Ring buffer of recently committed words.
    char32_t last_boundary_key = 0;         ///< The key that triggered the last word boundary.
    bool at_sentence_start = true;          ///< Whether we are at the start of a sentence.
    InputMethod method;                     ///< The active input method.
    ToneStyle tone_style = ToneStyle::NEW;  ///< The active tone style.
    FreeWOption free_w = FreeWOption::NON_START;  ///< The standalone 'w' option.
    bool std_uo = false;                          ///< Whether standard uo transformation is active.
    bool auto_restore = true;                     ///< Whether English auto-restore is enabled.
    bool double_space_to_period = false;          ///< Whether double-space converts to period.
    bool auto_capitalize = false;  ///< Whether auto-capitalize after sentences is enabled.
    MacroMode macro_mode = MacroMode::ADAPTIVE;   ///< The active macro expansion mode.
    bool allow_non_standard_initials = false;     ///< Whether non-standard initials (z, w, j, f) are allowed.
    BackspaceStyle backspace_style = BackspaceStyle::SURGICAL; ///< The active backspace style.

    // Rule-based English detection using phonotactic linguistics.
    bool is_english_word(const std::string& word) const;
};

}  // namespace lotus_engine
