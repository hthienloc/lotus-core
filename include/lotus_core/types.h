#pragma once

#include "lotus_core/common.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace lotus_core {

/**
 * @brief A fixed-size zero-allocation string for hot-loop operations.
 */
class StaticString {
public:
    static constexpr size_t MAX_LEN = 128;
    
    StaticString() : len(0) {}
    StaticString(const std::u32string& str) : len(0) {
        for (char32_t c : str) {
            if (len < MAX_LEN) data[len++] = c;
        }
    }
    StaticString& operator=(std::u32string_view view) {
        len = 0;
        for (char32_t c : view) {
            if (len < MAX_LEN) data[len++] = c;
        }
        return *this;
    }

    StaticString& operator+=(char32_t c) { push_back(c); return *this; }
    StaticString substr(size_t pos, size_t count = std::u32string::npos) const {
        StaticString res;
        if (pos >= len) return res;
        size_t end = (count == std::u32string::npos) ? len : std::min(len, pos + count);
        for (size_t i = pos; i < end; ++i) res.push_back(data[i]);
        return res;
    }

    
    StaticString(std::u32string_view view) : len(0) {
        for (char32_t c : view) {
            if (len < MAX_LEN) data[len++] = c;
        }
    }

    void push_back(char32_t c) {
        if (len < MAX_LEN) data[len++] = c;
    }
    
    void pop_back() {
        if (len > 0) len--;
    }
    
    void clear() {
        len = 0;
    }
    
    bool empty() const {
        return len == 0;
    }
    
    size_t size() const {
        return len;
    }
    
    char32_t& operator[](size_t pos) { return data[pos]; }
    const char32_t& operator[](size_t pos) const { return data[pos]; }
    
    char32_t front() const { return data[0]; }
    char32_t back() const { return data[len - 1]; }
    
    auto begin() { return data.begin(); }
    auto end() { return data.begin() + len; }
    auto begin() const { return data.begin(); }
    auto end() const { return data.begin() + len; }
    
    std::u32string_view view() const {
        return std::u32string_view(data.data(), len);
    }
    
    std::u32string to_u32string() const {
        return std::u32string(data.data(), len);
    }
    
    bool operator==(const StaticString& other) const {
        return view() == other.view();
    }
    
    bool operator==(std::u32string_view other) const {
        return view() == other;
    }

private:
    std::array<char32_t, MAX_LEN> data;
    size_t len;
};

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
    StaticString initial;         ///< Initial Consonant (e.g., b, ch, ngh)
    std::optional<char32_t> glide;  ///< Glide (e.g., o, u)
    StaticString vowel;           ///< Vowel Nucleus (e.g., a, ă, ê, iê)
    StaticString final_c;         ///< Final Coda (e.g., n, ng, ch, i, y)
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
     * @return StaticString A StaticString of UTF-32 key codes.
     */
    StaticString to_keys(InputMethod method) const;
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
 * @brief Linguistic diagnostic codes for engine evaluation and debugging.
 */
enum class DiagnosticCode : uint8_t {
    SUCCESS = 0,
    INVALID_INITIAL,
    INVALID_GLIDE,
    INVALID_NUCLEUS,
    INVALID_CODA,
    TONE_PLACEMENT_ERROR,
    ENGLISH_RESTORED,
    MACRO_EXPANDED,
    INTERNAL_ERROR
};

/**
 * @brief Helper to convert a DiagnosticCode to a descriptive string.
 */
inline std::string to_string(DiagnosticCode code) {
    switch (code) {
        case DiagnosticCode::SUCCESS: return "Success";
        case DiagnosticCode::INVALID_INITIAL: return "Error: Invalid initial consonant";
        case DiagnosticCode::INVALID_GLIDE: return "Error: Invalid glide combination";
        case DiagnosticCode::INVALID_NUCLEUS: return "Error: Invalid vowel nucleus";
        case DiagnosticCode::INVALID_CODA: return "Error: Invalid final consonant (coda)";
        case DiagnosticCode::TONE_PLACEMENT_ERROR: return "Error: Invalid tone placement";
        case DiagnosticCode::ENGLISH_RESTORED: return "Info: Restored English fallback";
        case DiagnosticCode::MACRO_EXPANDED: return "Info: Macro expanded";
        case DiagnosticCode::INTERNAL_ERROR: return "Error: Internal engine error";
        default: return "Unknown diagnostic code";
    }
}

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
    uint32_t chars[128];

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
     * @brief Diagnostic code indicating linguistic validation status.
     */
    DiagnosticCode diagnostic = DiagnosticCode::SUCCESS;

    /**
     * @brief Helper to convert the result into a UTF-8 string.
     */
    std::string to_string() const;
};

}  // namespace lotus_core
