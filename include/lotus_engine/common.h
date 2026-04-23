#pragma once

#include <cstdint>

namespace lotus_engine {

#define LOTUS_ENGINE_VERSION "1.0.0"

/**
 * @brief Vietnamese input method types.
 */
enum class InputMethod : uint8_t { TELEX, VNI };

/**
 * @brief Vietnamese tone marks.
 */
enum class Tone : uint8_t {
    NONE = 0,  // Ngang (No mark)
    ACUTE,     // Sắc (Rising)
    GRAVE,     // Huyền (Falling)
    HOOK,      // Hỏi (Dipping-rising)
    TILDE,     // Ngã (Glottalized rising)
    DOT        // Nặng (Low-dropping)
};

/**
 * @brief Vietnamese tone placement style (Old vs. New).
 */
enum class ToneStyle : uint8_t {
    OLD,  // e.g., hòa, họa
    NEW   // e.g., hoà, hoạ (Modern standard, default)
};

/**
 * @brief Options for handling standalone 'w' in TELEX.
 */
enum class FreeWOption : uint8_t {
    OFF,        // w -> w
    NON_START,  // w -> ư (unless it's the first character)
    ALWAYS      // w -> ư (everywhere)
};

/**
 * @brief Professional macro expansion modes for shortcuts.
 */
enum class MacroMode : uint8_t {
    OFF = 0,      // Disable shortcuts.
    EXACT = 1,    // Match case exactly (e.g., vn matches vn only).
    FIXED = 2,    // Match case-insensitively, return fixed string.
    ADAPTIVE = 3  // Match case-insensitively, return transformed case.
};

}  // namespace lotus_engine
