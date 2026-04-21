#pragma once

#include "lotus_engine/types.h"
#include <string>
#include <vector>
#include <functional>
#include <map>

namespace lotus_engine {

/**
 * @brief Engine trung tâm điều phối việc gõ tiếng Việt.
 */
enum class InputMethod {
    TELEX,
    VNI
};

class Engine {
public:
    Engine();
    
    void set_method(InputMethod method) { this->method = method; }
    InputMethod get_method() const { return method; }

    void set_tone_style(ToneStyle style) { this->tone_style = style; }
    ToneStyle get_tone_style() const { return tone_style; }

    /**
     * @brief Xử lý một phím gõ mới.
     * @param key Mã Unicode (UTF-32) của phím gõ.
     * @param mods Các modifier key (Shift, CapsLock...).
     * @return EngineResult Kết quả điều khiển cho frontend.
     */
    EngineResult process_key(char32_t key, const Modifiers& mods);

    /**
     * @brief Xóa sạch buffer và reset trạng thái engine.
     */
    void reset();

    /**
     * @brief Thêm một phím tắt gõ nhanh.
     * @param trigger Chuỗi kích hoạt (vd: "vn")
     * @param replacement Chuỗi thay thế (vd: "Việt Nam")
     */
    void add_shortcut(const std::string& trigger, const std::string& replacement);

private:
    std::u32string buffer;
    char32_t last_modifier_key = 0;
    std::u32string last_committed_text; 
    std::map<std::string, std::string> shortcuts;
    WordHistory word_history;
    char32_t last_boundary_key = 0;
    InputMethod method;
    ToneStyle tone_style = ToneStyle::NEW;
    
    // Internal English whitelist (simplified)
    bool is_english_word(const std::string& word) const {
        static const std::vector<std::string> whitelist = {
            "test", "expect", "date", "status", "nurses", "bass", "issue", "message"
        };
        for (const auto& w : whitelist) if (w == word) return true;
        return false;
    }
};

} // namespace lotus_engine
