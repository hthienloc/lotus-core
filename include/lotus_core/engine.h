#pragma once

#include "lotus_core/linguistics.h"
#include "lotus_core/types.h"
#include "lotus_core/shortcut_manager.h"
#include "lotus_core/context_tracker.h"
#include "lotus_core/input_dispatcher.h"
#include "lotus_core/composition_buffer.h"

#include <functional>
#include <string>
#include <vector>

namespace lotus_core {

/**
 * @brief Configuration settings for the Engine.
 */
struct EngineConfig {
    InputMethod method = InputMethod::TELEX;
    ToneStyle tone_style = ToneStyle::NEW;
    FreeWOption free_w = FreeWOption::NON_START;
    bool std_uo = false;
    bool auto_restore = true;
    bool double_space_to_period = false;
    bool auto_capitalize = false;
    bool allow_non_standard_initials = false;
    size_t commit_threshold = 64;
    MacroMode macro_mode = MacroMode::ADAPTIVE;
    BackspaceStyle backspace_style = BackspaceStyle::SURGICAL;
};

/**
 * @brief Internal processing state of the Engine.
 */
struct EngineState {
            std::u32string last_committed_text;
    char32_t last_boundary_key = 0;
};

/**
 * @brief The core Vietnamese input method engine.
 *
 * Maintains composition state and transforms raw keystroke sequences
 * into Vietnamese text according to TELEX or VNI rules.
 */
class Engine {
   public:
    Engine();

    /** @brief Sets the active input method (TELEX or VNI). */
    void set_method(InputMethod method) { config.method = method; }
    /** @brief Returns the current input method. */
    InputMethod get_method() const { return config.method; }

    /** @brief Sets the tone placement style (Old or New). */
    void set_tone_style(ToneStyle style) { config.tone_style = style; }
    /** @brief Returns the current tone placement style. */
    ToneStyle get_tone_style() const { return config.tone_style; }

    /** @brief Sets the standalone 'w' handling option (TELEX). */
    void set_free_w(FreeWOption option) { config.free_w = option; }
    /** @brief Returns the current Free-W option. */
    FreeWOption get_free_w() const { return config.free_w; }

    /** @brief Enables or disables standard uo -> ươ transformation. */
    void set_std_uo(bool enabled) { config.std_uo = enabled; }
    /** @brief Returns whether the std_uo transformation is enabled. */
    bool get_std_uo() const { return config.std_uo; }

    /** @brief Enables or disables automatic English word restoration. */
    void set_auto_restore(bool enabled) { config.auto_restore = enabled; }
    /** @brief Returns whether auto-restore is enabled. */
    bool get_auto_restore() const { return config.auto_restore; }

    /** @brief Enables or disables double-space to period conversion. */
    void set_double_space_to_period(bool enabled) { config.double_space_to_period = enabled; }
    /** @brief Returns whether double-space to period is enabled. */
    bool get_double_space_to_period() const { return config.double_space_to_period; }

    /** @brief Enables or disables automatic capitalization after sentence boundaries. */
    void set_auto_capitalize(bool enabled) { config.auto_capitalize = enabled; }
    /** @brief Returns whether auto-capitalize is enabled. */
    bool get_auto_capitalize() const { return config.auto_capitalize; }

    /** @brief Sets whether the next input is at the start of a new sentence. */
    void set_at_sentence_start(bool enabled) { context_tracker.set_at_sentence_start(enabled); }
    /** @brief Returns whether the engine is currently at a sentence start. */
    bool get_at_sentence_start() const { return context_tracker.is_at_sentence_start(); }

    /** @brief Enables or disables non-standard initials (z, w, j, f). */
    void set_allow_non_standard_initials(bool enabled) { config.allow_non_standard_initials = enabled; }
    /** @brief Returns whether non-standard initials are allowed. */
    bool get_allow_non_standard_initials() const { return config.allow_non_standard_initials; }

    /** @brief Sets the active macro expansion mode. */
    void set_macro_mode(MacroMode mode) { config.macro_mode = mode; }
    /** @brief Returns the current macro expansion mode. */
    MacroMode get_macro_mode() const { return config.macro_mode; }

    /** @brief Sets the active backspace style. */
    void set_backspace_style(BackspaceStyle style) { config.backspace_style = style; }
    /** @brief Returns the current backspace style. */
    BackspaceStyle get_backspace_style() const { return config.backspace_style; }

    /**
     * @brief Batch configuration accessors.
     */
    const EngineConfig& get_config() const { return config; }
    void set_config(const EngineConfig& new_config) { config = new_config; }

    /**
     * @brief Processes a single keypress and returns the required UI action.
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
     */
    void add_shortcut(const std::string& trigger, const std::string& replacement);

    /**
     * @brief Reconstructs engine state from an existing committed text string.
     */
    void rebuild_from_text(const std::string& text);

   private:

    bool handle_backspace(char32_t key, const Modifiers& mods, EngineResult& result);
    bool handle_boundary(char32_t key, EngineResult& result);
    bool handle_shortcuts(char32_t key, EngineResult& result);
    bool handle_smart_typing(char32_t& key, const Modifiers& mods, EngineResult& result);
    bool handle_navigation(char32_t key, const Modifiers& mods, EngineResult& result);
    bool reclaim_from_history(InputMethod method);
    void commit_syllable_to_history(char32_t boundary_key);

    EngineResult build_result(std::u32string_view final_u32);

    ShortcutManager shortcut_manager;
    CompositionBuffer composition_buffer;
    ContextTracker context_tracker;
    EngineConfig config;
    EngineState state;

    bool is_likely_english(const std::string& word) const;
};

}  // namespace lotus_core
