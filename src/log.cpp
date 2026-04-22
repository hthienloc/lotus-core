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
}

/**
 * @brief Emits a log message to the registered callback.
 * @param level The severity level of the log.
 * @param message The log message content.
 */
void emit_log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_callback) {
        g_log_callback(level, message);
    }
}

}  // namespace lotus_engine
