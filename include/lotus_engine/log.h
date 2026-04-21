#pragma once

#include <string>
#include <iostream>
#include <functional>

namespace lotus_engine {

/**
 * @brief Log levels for the Lotus Engine.
 */
enum class LogLevel {
    DEBUG = 0,
    INFO  = 1,
    ERROR = 2
};

/**
 * @brief Global logging callback type.
 * @param level The log level.
 * @param message The log message.
 */
using LogCallback = std::function<void(LogLevel level, const std::string& message)>;

/**
 * @brief Set the global log callback. If null, logs will not be emitted (except ERROR to stderr by default if desired, though usually we defer entirely to the callback).
 */
void set_log_callback(LogCallback callback);

/**
 * @brief Internal function to emit a log message.
 */
void emit_log(LogLevel level, const std::string& message);

} // namespace lotus_engine

// Internal macros for easily emitting logs
#define LOTUS_LOG_DEBUG(msg) do { \
    lotus_engine::emit_log(lotus_engine::LogLevel::DEBUG, msg); \
} while(0)

#define LOTUS_LOG_INFO(msg) do { \
    lotus_engine::emit_log(lotus_engine::LogLevel::INFO, msg); \
} while(0)

#define LOTUS_LOG_ERROR(msg) do { \
    lotus_engine::emit_log(lotus_engine::LogLevel::ERROR, msg); \
} while(0)
