#pragma once

#include "lotus_engine/types.h"
#include "lotus_engine/linguistics.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace lotus_engine {

class Engine {
   public:
    Engine();

    void set_method(InputMethod method) { this->method = method; }
    InputMethod get_method() const { return method; }

    void set_tone_style(ToneStyle style) { this->tone_style = style; }
    ToneStyle get_tone_style() const { return tone_style; }

    void set_free_w(FreeWOption option) { this->free_w = option; }
    FreeWOption get_free_w() const { return free_w; }

    void set_std_uo(bool enabled) { this->std_uo = enabled; }
    bool get_std_uo() const { return std_uo; }

    void set_auto_restore(bool enabled) { this->auto_restore = enabled; }
    bool get_auto_restore() const { return auto_restore; }

    void set_double_space_to_period(bool enabled) { this->double_space_to_period = enabled; }
    bool get_double_space_to_period() const { return double_space_to_period; }

    void set_auto_capitalize(bool enabled) { this->auto_capitalize = enabled; }
    bool get_auto_capitalize() const { return auto_capitalize; }

    void set_at_sentence_start(bool enabled) { this->at_sentence_start = enabled; }
    bool get_at_sentence_start() const { return at_sentence_start; }

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

    /**
     * @brief Tái cấu trúc trạng thái engine từ một đoạn text có sẵn.
     * @param text Đoạn text nguồn (vd: "xin chào")
     */
    void rebuild_from_text(const std::string& text);

   private:
    // Helper methods for refactoring
    void apply_telex_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                               Tone& tone_state);
    void apply_vni_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                             Tone& tone_state);
    EngineResult make_transformation_result(const std::u32string& final_u32);

    std::u32string buffer;
    char32_t last_modifier_key = 0;
    std::u32string last_committed_text;
    std::map<std::string, std::string> shortcuts;
    WordHistory word_history;
    char32_t last_boundary_key = 0;
    bool at_sentence_start = true;
    InputMethod method;
    ToneStyle tone_style = ToneStyle::NEW;
    FreeWOption free_w = FreeWOption::NON_START;
    bool std_uo = false;
    bool auto_restore = true;
    bool double_space_to_period = false;
    bool auto_capitalize = false;

    // Internal English detection using rule-based linguistics
    bool is_english_word(const std::string& word) const;
};

}  // namespace lotus_engine
