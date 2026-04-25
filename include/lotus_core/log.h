#pragma once

#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace lotus_core {

/**
 * @brief Log levels for the Lotus Core.
 */
enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

/**
 * @brief Global logging callback type.
 * @param level The log level.
 * @param stage The pipeline stage name or component.
 * @param time_us The execution time in microseconds (for tracing).
 * @param message The log message content.
 */
using LogCallback =
    std::function<void(LogLevel level, const std::string& stage, double time_us, const std::string& message)>;

/**
 * @brief The maximum log level to emit. Messages below this level are ignored.
 */
extern LogLevel g_max_log_level;

/**
 * @brief Fast-path boolean to check if a log callback is currently registered.
 */
extern bool g_has_log_callback;

/**
 * @brief Set the global log callback.
 */
void set_log_callback(LogCallback callback);

/**
 * @brief Set the maximum log level.
 * @param level The new maximum log level.
 */
void set_max_log_level(LogLevel level);

/**
 * @brief Emits a log message to the registered callback.
 */
void emit_log(LogLevel level, const std::string& message, const std::string& stage = "",
              double time_us = 0.0);

/**
 * @brief Exports current tracing buffer to a JSON file.
 * @param filepath The output path.
 */
void export_tracing(const std::string& filepath);

/**
 * @brief RAII helper for measuring execution time of a scope.
 */
struct TraceScope {
    std::string name;
    std::chrono::steady_clock::time_point start;
    TraceScope(std::string n) : name(std::move(n)), start(std::chrono::steady_clock::now()) {}
    ~TraceScope();
};

/**
 * @brief Formats a log message with a standardized aligned component tag.
 * @param component The component name (e.g., "PARSER").
 * @param message The log message.
 * @return The formatted log string.
 */
std::string format_log_message(const std::string& component, const std::string& message);

}  // namespace lotus_core

// Internal macros for easily emitting logs
#define LOTUS_TRACE_SCOPE(name) lotus_core::TraceScope _lotus_trace_##__LINE__(name)

#define LOTUS_LOG_DEBUG(msg) \
    do { if (lotus_core::g_has_log_callback) lotus_core::emit_log(lotus_core::LogLevel::DEBUG, msg); } while(0)
#define LOTUS_LOG_INFO(msg) \
    do { if (lotus_core::g_has_log_callback) lotus_core::emit_log(lotus_core::LogLevel::INFO, msg); } while(0)
#define LOTUS_LOG_WARN(msg) \
    do { if (lotus_core::g_has_log_callback) lotus_core::emit_log(lotus_core::LogLevel::WARN, msg); } while(0)
#define LOTUS_LOG_ERROR(msg) \
    do { if (lotus_core::g_has_log_callback) lotus_core::emit_log(lotus_core::LogLevel::ERROR, msg); } while(0)
