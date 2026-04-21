#include "lotus_engine/log.h"

#include <mutex>

namespace lotus_engine {

static LogCallback g_log_callback = nullptr;
static std::mutex g_log_mutex;

void set_log_callback(LogCallback callback) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    g_log_callback = std::move(callback);
}

void emit_log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    if (g_log_callback) {
        g_log_callback(level, message);
    }
}

}  // namespace lotus_engine
