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
static std::vector<TraceEvent> g_trace_buffer;

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
    g_trace_buffer.push_back({level, stage, time_us, message});
    if (g_trace_buffer.size() > 10000) { // Limit buffer size
        g_trace_buffer.erase(g_trace_buffer.begin());
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
    for (size_t i = 0; i < g_trace_buffer.size(); ++i) {
        const auto& ev = g_trace_buffer[i];
        file << "  {\n";
        file << "    \"level\": " << static_cast<int>(ev.level) << ",\n";
        file << "    \"stage\": \"" << ev.stage << "\",\n";
        file << "    \"duration_us\": " << std::fixed << std::setprecision(3) << ev.time_us << ",\n";
        file << "    \"message\": \"" << ev.message << "\"\n";
        file << "  }" << (i == g_trace_buffer.size() - 1 ? "" : ",") << "\n";
    }
    file << "]\n";
}

TraceScope::~TraceScope() {
    auto end = std::chrono::steady_clock::now();
    double duration = std::chrono::duration<double, std::micro>(end - start).count();
    emit_log(LogLevel::DEBUG, "Scope finished", name, duration);
}

} // namespace lotus_core
