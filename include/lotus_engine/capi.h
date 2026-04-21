#ifndef LOTUS_ENGINE_CAPI_H
#define LOTUS_ENGINE_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Opaque handle to a Vietnamese Engine instance.
 */
typedef struct lotus_engine_t lotus_engine_t;

/**
 * @brief Input methods.
 */
typedef enum { LOTUS_METHOD_TELEX = 0, LOTUS_METHOD_VNI = 1 } lotus_method_t;

/**
 * @brief Tone placement styles.
 */
typedef enum {
    LOTUS_TONE_OLD = 0,  // hòa
    LOTUS_TONE_NEW = 1   // hoà (Default)
} lotus_tone_style_t;

/**
 * @brief Free-W options (Telex).
 */
typedef enum {
    LOTUS_FREE_W_OFF = 0,        // w -> w
    LOTUS_FREE_W_NON_START = 1,  // w -> ư (unless at start)
    LOTUS_FREE_W_ALWAYS = 2      // w -> ư everywhere
} lotus_free_w_t;

/**
 * @brief Log levels for internal diagnostic messages.
 */
typedef enum {
    LOTUS_LOG_LEVEL_DEBUG = 0,
    LOTUS_LOG_LEVEL_INFO = 1,
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
    uint8_t action;      // 0: pass-through, 1: transformation
    uint8_t backspace;   // Number of characters to delete
    uint8_t count;       // Number of new characters in 'chars'
    uint32_t chars[32];  // Buffer of UTF-32 characters
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
 * @return A pointer to the newly created lotus_engine_t instance.
 *         Must be destroyed with lotus_engine_destroy.
 */
lotus_engine_t* lotus_engine_create();

/**
 * @brief Destroy an engine instance.
 * @param engine The engine instance to destroy.
 */
void lotus_engine_destroy(lotus_engine_t* engine);

/**
 * @brief Process a key press and return a transformation result.
 * @param engine The engine instance.
 * @param key The UTF-32 key code to process.
 * @param mods Active modifier keys (Shift, Caps Lock).
 * @return A lotus_result_t structure containing the required UI actions.
 */
lotus_result_t lotus_engine_process_key(lotus_engine_t* engine, uint32_t key,
                                        lotus_modifiers_t mods);

/**
 * @brief Reset engine state (clears the composition buffer).
 * @param engine The engine instance.
 */
void lotus_engine_reset(lotus_engine_t* engine);

/**
 * @brief Configure the input method (Telex or VNI).
 * @param engine The engine instance.
 * @param method The input method to use.
 */
void lotus_engine_set_method(lotus_engine_t* engine, lotus_method_t method);

/**
 * @brief Configure the tone placement style.
 * @param engine The engine instance.
 * @param style The tone style (Old or New).
 */
void lotus_engine_set_tone_style(lotus_engine_t* engine, lotus_tone_style_t style);

/**
 * @brief Configure the Free-W option (Telex).
 * @param engine The engine instance.
 * @param option The Free-W option (Off, Non-start, Always).
 */
void lotus_engine_set_free_w(lotus_engine_t* engine, lotus_free_w_t option);

/**
 * @brief Configure the manual hook keys option ([,] -> ư,ơ).
 * @param engine The engine instance.
 * @param enabled True to enable [ and ] as Vietnamese hooks.
 */
void lotus_engine_set_std_uo(lotus_engine_t* engine, bool enabled);

/**
 * @brief Add a custom shortcut for string expansion.
 * @param engine The engine instance.
 * @param trigger The shortcut trigger string.
 * @param replacement The expansion string.
 */
void lotus_engine_add_shortcut(lotus_engine_t* engine, const char* trigger,
                               const char* replacement);

/**
 * @brief Set the global logging callback to capture diagnostic info from the engine.
 * @param callback Function pointer to the handler, or NULL to disable logging.
 */
void lotus_engine_set_log_callback(lotus_log_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif  // LOTUS_ENGINE_CAPI_H
