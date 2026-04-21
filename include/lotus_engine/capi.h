#ifndef LOTUS_ENGINE_CAPI_H
#define LOTUS_ENGINE_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Opaque handle to a Vietnamese Engine instance.
 */
typedef struct lotus_engine_t lotus_engine_t;

/**
 * @brief Input methods.
 */
typedef enum {
    LOTUS_METHOD_TELEX = 0,
    LOTUS_METHOD_VNI   = 1
} lotus_method_t;

/**
 * @brief Tone placement styles.
 */
typedef enum {
    LOTUS_TONE_OLD = 0, // hòa
    LOTUS_TONE_NEW = 1  // hoà (Default)
} lotus_tone_style_t;

/**
 * @brief Log levels for internal diagnostic messages.
 */
typedef enum {
    LOTUS_LOG_LEVEL_DEBUG = 0,
    LOTUS_LOG_LEVEL_INFO  = 1,
    LOTUS_LOG_LEVEL_ERROR = 2
} lotus_log_level_t;

/**
 * @brief Callback function type for receiving log messages.
 * @param level Sensitivity level of the log message.
 * @param message The logging string (null-terminated).
 */
typedef void (*lotus_log_callback_t)(lotus_log_level_t level, const char* message);

/**
 * @brief Result structure for a key processing action.
 */
typedef struct {
    uint8_t action;     // 0: pass-through, 1: transformation
    uint8_t backspace;  // Number of characters to delete
    uint8_t count;      // Number of new characters in 'chars'
    uint32_t chars[32]; // Buffer of UTF-32 characters
} lotus_result_t;

/**
 * @brief Modifier keys bitmask.
 */
typedef struct {
    bool shift;
    bool caps_lock;
} lotus_modifiers_t;

/**
 * @brief Create a new engine instance.
 */
lotus_engine_t* lotus_engine_create();

/**
 * @brief Destroy an engine instance.
 */
void lotus_engine_destroy(lotus_engine_t* engine);

/**
 * @brief Process a key press.
 */
lotus_result_t lotus_engine_process_key(lotus_engine_t* engine, uint32_t key, lotus_modifiers_t mods);

/**
 * @brief Reset engine state.
 */
void lotus_engine_reset(lotus_engine_t* engine);

/**
 * @brief Configure input method.
 */
void lotus_engine_set_method(lotus_engine_t* engine, lotus_method_t method);

/**
 * @brief Configure tone placement style.
 */
void lotus_engine_set_tone_style(lotus_engine_t* engine, lotus_tone_style_t style);

/**
 * @brief Add a shortcut.
 */
void lotus_engine_add_shortcut(lotus_engine_t* engine, const char* trigger, const char* replacement);

/**
 * @brief Set the global logging callback to capture diagnostic info from the engine.
 * @param callback Function pointer to the handler, or NULL to disable logging.
 */
void lotus_engine_set_log_callback(lotus_log_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif // LOTUS_ENGINE_CAPI_H
