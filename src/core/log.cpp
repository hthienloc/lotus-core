/**
 * @file log.cpp
 * @brief Logging infrastructure for the Lotus Engine.
 */

#include "lotus_engine/log.h"

#include <mutex>

namespace lotus_engine {

// ============================================================================
// [ Internal State ]
// ============================================================================

LogLevel g_max_log_level = LogLevel::INFO;
bool g_has_log_callback = false;

static LogCallback g_log_callback = nullptr;
static std::mutex g_log_mutex;

// ============================================================================
// [ Logging Implementation ]
// ============================================================================

/**
 * @brief Registers a global callback for engine log events.
 * @param callback The function to invoke for log messages.
 */
void set_log_callback(LogCallback callback) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    g_log_callback = std::move(callback);
    g_has_log_callback = (g_log_callback != nullptr);
}

/**
 * @brief Set the maximum log level.
 * @param level The new maximum log level.
 */
void set_max_log_level(LogLevel level) {
    g_max_log_level = level;
}

/**
 * @brief Emits a log message to the registered callback.
 * @param level The severity level of the log.
 * @param message The log message content.
 */
void emit_log(LogLevel level, const std::string& message) {
    if (!g_has_log_callback || level < g_max_log_level) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_callback) {
        g_log_callback(level, message);
    }
}

}  // namespace lotus_engine
