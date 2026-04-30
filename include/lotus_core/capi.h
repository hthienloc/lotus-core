#ifndef LOTUS_CORE_CAPI_H
#define LOTUS_CORE_CAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Opaque handle to a Vietnamese Engine instance.
 */
typedef struct lotus_core_t lotus_core_t;

/**
 * @brief Input methods.
 */
typedef enum {
    LOTUS_METHOD_TELEX = 0,
    LOTUS_METHOD_VNI = 1,
    LOTUS_METHOD_TELEX_VNI = 2,
    LOTUS_METHOD_VIQR = 3,
    LOTUS_METHOD_TELEX_VNI_VIQR = 4
} lotus_method_t;

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
 * @brief Professional macro expansion modes for shortcuts.
 */
typedef enum {
    LOTUS_MACRO_OFF = 0,
    LOTUS_MACRO_EXACT = 1,
    LOTUS_MACRO_FIXED = 2,
    LOTUS_MACRO_ADAPTIVE = 3
} lotus_macro_mode_t;

/**
 * @brief Backspace behavior styles.
 */
typedef enum {
    LOTUS_BACKSPACE_KEYSTROKE = 0,
    LOTUS_BACKSPACE_SURGICAL = 1
} lotus_backspace_style_t;

/**
 * @brief Log levels for internal diagnostic messages.
 */
typedef enum {
    LOTUS_LOG_LEVEL_DEBUG = 0,
    LOTUS_LOG_LEVEL_INFO = 1,
    LOTUS_LOG_LEVEL_WARN = 2,
    LOTUS_LOG_LEVEL_ERROR = 3
} lotus_log_level_t;

/**
 * @brief Callback function type for receiving log and tracing messages.
 * @param level Sensitivity level of the log message.
 * @param stage The pipeline stage name.
 * @param time_us The execution duration in microseconds.
 * @param message The logging string (null-terminated).
 */
typedef void (*lotus_log_callback_t)(lotus_log_level_t level, const char* stage, double time_us, const char* message);

/**
 * @brief Linguistic diagnostic codes for engine evaluation and debugging.
 */
typedef enum {
    LOTUS_DIAGNOSTIC_SUCCESS = 0,
    LOTUS_DIAGNOSTIC_INVALID_INITIAL = 1,
    LOTUS_DIAGNOSTIC_INVALID_GLIDE = 2,
    LOTUS_DIAGNOSTIC_INVALID_NUCLEUS = 3,
    LOTUS_DIAGNOSTIC_INVALID_CODA = 4,
    LOTUS_DIAGNOSTIC_TONE_PLACEMENT_ERROR = 5,
    LOTUS_DIAGNOSTIC_ENGLISH_RESTORED = 6,
    LOTUS_DIAGNOSTIC_MACRO_EXPANDED = 7,
    LOTUS_DIAGNOSTIC_INTERNAL_ERROR = 8
} lotus_diagnostic_code_t;

/**
 * @brief Result structure for a key processing action.
 */
typedef struct {
    uint8_t action;      // 0: pass-through, 1: transformation
    uint8_t backspace;   // Number of characters to delete
    uint8_t count;       // Number of new characters in 'chars'
    uint32_t chars[128];  // Buffer of UTF-32 characters
    uint8_t diagnostic;  // Diagnostic code indicating linguistic validation status (lotus_diagnostic_code_t)
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
lotus_core_t* lotus_core_create();

/**
 * @brief Destroy an engine instance.
 */
void lotus_core_destroy(lotus_core_t* engine);

/**
 * @brief Process a key press and return a transformation result.
 */
lotus_result_t lotus_core_process_key(lotus_core_t* engine, uint32_t key,
                                        lotus_modifiers_t mods);

/**
 * @brief Reset engine state.
 * 
 * Implementers should call this function when the user's cursor moves non-linearly
 * (e.g., via mouse click or non-standard navigation) to prevent the engine from 
 * attempting to backspace into or modify text that is no longer at the active cursor position.
 */
void lotus_core_reset(lotus_core_t* engine);

/**
 * @brief Configure the input method.
 */
void lotus_core_set_method(lotus_core_t* engine, lotus_method_t method);

/**
 * @brief Configure the tone placement style.
 */
void lotus_core_set_tone_style(lotus_core_t* engine, lotus_tone_style_t style);

/**
 * @brief Configure the Free-W option.
 */
void lotus_core_set_free_w(lotus_core_t* engine, lotus_free_w_t option);

/**
 * @brief Configure the manual hook keys option.
 */
void lotus_core_set_std_uo(lotus_core_t* engine, bool enabled);

/**
 * @brief Add a custom shortcut for string expansion.
 */
void lotus_core_add_shortcut(lotus_core_t* engine, const char* trigger,
                               const char* replacement);

/**
 * @brief Set the global logging callback.
 */
void lotus_core_set_log_callback(lotus_log_callback_t callback);

/**
 * @brief Enables or disables automatic English word restoration.
 */
void lotus_core_set_auto_restore(lotus_core_t* engine, bool enabled);

/**
 * @brief Enables or disables allowing non-standard initial consonants (z, w, j, f).
 */
void lotus_core_set_allow_non_standard_initials(lotus_core_t* engine, bool enabled);

/**
 * @brief Enables or disables double-space to period conversion.
 */
void lotus_core_set_double_space_to_period(lotus_core_t* engine, bool enabled);

/**
 * @brief Enables or disables auto-capitalization after sentences.
 */
void lotus_core_set_auto_capitalize(lotus_core_t* engine, bool enabled);

/**
 * @brief Configures the macro expansion mode.
 */
void lotus_core_set_macro_mode(lotus_core_t* engine, lotus_macro_mode_t mode);

/**
 * @brief Configures the backspace style.
 */
void lotus_core_set_backspace_style(lotus_core_t* engine, lotus_backspace_style_t style);

/**
 * @brief Export current tracing buffer to a JSON file.
 * @param filepath The output path.
 */
void lotus_core_export_tracing(const char* filepath);

/**
 * @brief Opaque handle to a dictionary word list.
 */
typedef struct lotus_dict_t lotus_dict_t;

/**
 * @brief Create a dictionary from a newline-separated word list file.
 * @param filepath Path to the dictionary file (one word per line).
 * @return Dictionary handle or NULL on failure.
 */
lotus_dict_t* lotus_dict_create(const char* filepath);

/**
 * @brief Destroy a dictionary handle.
 */
void lotus_dict_destroy(lotus_dict_t* dict);

/**
 * @brief Check if a word exists in the dictionary.
 * @param dict Dictionary handle.
 * @param word UTF-8 word to check.
 * @return True if word is found.
 */
bool lotus_dict_contains(lotus_dict_t* dict, const char* word);

/**
 * @brief Set the dictionary for spell-check validation.
 * @param engine Engine handle.
 * @param dict Dictionary handle (NULL to disable spell-check).
 */
void lotus_core_set_dictionary(lotus_core_t* engine, lotus_dict_t* dict);

/**
 * @brief Enable or disable spell-check with dictionary.
 * @param engine Engine handle.
 * @param enabled Whether to use dictionary for validation.
 */
void lotus_core_set_spell_check(lotus_core_t* engine, bool enabled);

/**
 * @brief Output charset encodings.
 */
typedef enum {
    LOTUS_CHARSET_UNICODE = 0,
    LOTUS_CHARSET_TCVN3   = 1,
    LOTUS_CHARSET_VNI     = 2,
    LOTUS_CHARSET_VISCII  = 3
} lotus_charset_t;

/**
 * @brief Set the output charset encoding for commit text.
 * @param engine Engine handle.
 * @param charset Output charset encoding.
 */
void lotus_core_set_charset(lotus_core_t* engine, lotus_charset_t charset);

/**
 * @brief Encode a UTF-8 string to the target charset.
 * @param charset Target charset encoding.
 * @param input UTF-8 input string.
 * @param output Output buffer for encoded string.
 * @param output_size Size of the output buffer.
 * @return Number of bytes written (excluding null terminator).
 */
size_t lotus_charset_encode(lotus_charset_t charset, const char* input, char* output, size_t output_size);

/**
 * @brief Get the number of available input methods.
 * @return Count of input methods.
 */
int lotus_core_get_input_method_count();

/**
 * @brief Get the name of an input method by index.
 * @param index Zero-based index.
 * @return Input method name string (static, do not free).
 */
const char* lotus_core_get_input_method_name(int index);

/**
 * @brief Get the number of available charset encodings.
 * @return Count of charset encodings.
 */
int lotus_core_get_charset_count();

/**
 * @brief Get the name of a charset encoding by index.
 * @param index Zero-based index.
 * @return Charset name string (static, do not free).
 */
const char* lotus_core_get_charset_name(int index);

/**
 * @brief Rebuild engine state from existing committed text.
 * @param engine Engine handle.
 * @param text UTF-8 text to rebuild from (surrounding text).
 */
void lotus_core_rebuild_from_text(lotus_core_t* engine, const char* text);

/**
 * @brief Encode the result output to the configured charset.
 * @param engine Engine handle (for charset config).
 * @param result The result to encode.
 * @param output Output buffer for encoded string.
 * @param output_size Size of the output buffer.
 * @return Number of bytes written (excluding null terminator).
 */
size_t lotus_core_encode_result(lotus_core_t* engine, const lotus_result_t* result, char* output, size_t output_size);

#ifdef __cplusplus
}
#endif

#endif  // LOTUS_CORE_CAPI_H
