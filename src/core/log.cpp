/**
 * @file log.cpp
 * @brief Logging and performance tracing infrastructure.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_core/log.h"
#include "lotus_core/unicode.h"
#include <mutex>
#include <vector>
#include <fstream>
#include <iomanip>

using namespace lotus_core;

namespace lotus_core {

struct TraceEvent {
    LogLevel level;
    std::string stage;
    double time_us;
    std::string message;
};

// ============================================================================
// [ Internal State ]
// ============================================================================

LogLevel g_max_log_level = LogLevel::INFO;
bool g_has_log_callback = false;

static LogCallback g_log_callback = nullptr;
static std::mutex g_log_mutex;
constexpr size_t MAX_TRACE_EVENTS = 10000;
static std::vector<TraceEvent> g_trace_buffer;
static size_t g_trace_head = 0;
static bool g_trace_buffer_full = false;

// ============================================================================
// [ Logging & Tracing Implementation ]
// ============================================================================

std::string format_log_message(const std::string& component, const std::string& message) {
    // We want the component tag (including brackets) to be 12 visual characters wide.
    // e.g. "[PARSER     ]" -> 12 characters.
    // So the component name itself plus brackets is width = 2 + display_width(component).
    
    std::string tag = "[" + component + "]";
    size_t width = unicode::display_width(tag);
    
    if (width < 13) { // 13 is target width of 12 for the tag + 1 space before message? No, requirement says "[PARSER     ] Initial: [ngh]" (where [PARSER     ] is exactly 12 width).
        // Let's make the tag exactly 12 visual characters wide.
        size_t pad_len = 12 - width;
        // insert padding before the closing bracket.
        tag.insert(tag.length() - 1, pad_len, ' ');
    }
    
    return tag + " " + message;
}

void set_log_callback(LogCallback callback) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    g_log_callback = std::move(callback);
    g_has_log_callback = (g_log_callback != nullptr);
}

void set_max_log_level(LogLevel level) {
    g_max_log_level = level;
}

void emit_log(LogLevel level, const std::string& message, const std::string& stage, double time_us) {
    if (level < g_max_log_level) return;

    std::lock_guard<std::mutex> lock(g_log_mutex);
    
    // Store in buffer for tracing export
    if (g_trace_buffer.empty()) {
        g_trace_buffer.resize(MAX_TRACE_EVENTS);
    }
    
    g_trace_buffer[g_trace_head] = {level, stage, time_us, message};
    g_trace_head = (g_trace_head + 1) % MAX_TRACE_EVENTS;
    if (g_trace_head == 0) {
        g_trace_buffer_full = true;
    }

    if (g_log_callback) {
        g_log_callback(level, stage, time_us, message);
    }
}

void export_tracing(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(g_log_mutex);
    std::ofstream file(filepath);
    if (!file.is_open()) return;

    file << "[\n";
    
    size_t count = g_trace_buffer_full ? MAX_TRACE_EVENTS : g_trace_head;
    for (size_t i = 0; i < count; ++i) {
        size_t idx = g_trace_buffer_full ? (g_trace_head + i) % MAX_TRACE_EVENTS : i;
        const auto& ev = g_trace_buffer[idx];
        file << "  {\n";
        file << "    \"level\": " << static_cast<int>(ev.level) << ",\n";
        file << "    \"stage\": \"" << ev.stage << "\",\n";
        file << "    \"duration_us\": " << std::fixed << std::setprecision(3) << ev.time_us << ",\n";
        file << "    \"message\": \"" << ev.message << "\"\n";
        file << "  }" << (i == count - 1 ? "" : ",") << "\n";
    }
    file << "]\n";
}

TraceScope::~TraceScope() {
    auto end = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::micro>(end - start).count();
    emit_log(LogLevel::DEBUG, "Scope finished", name, duration);
}

} // namespace lotus_core
