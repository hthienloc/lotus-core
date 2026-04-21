#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "lotus_engine/common.h"

namespace lotus_engine {

/**
 * @brief Cấu trúc mô phỏng một âm tiết tiếng Việt chuẩn: (C1)(G)V(C2) + T.
 */
struct Syllable {
    std::string initial;        // Phụ âm đầu (C1): b, ch, ngh...
    std::optional<char> glide;  // Âm đệm (G): o, u
    std::string vowel;          // Hạt nhân nguyên âm (V): a, ă, ê, iê...
    std::string final_c;        // Phụ âm cuối (C2): n, ng, ch, i, y...
    Tone tone = Tone::NONE;     // Dấu thanh (T)

    /**
     * @brief Chuyển đổi âm tiết về dạng chuỗi UTF-8.
     * @param style Kiểu bỏ dấu (Mới/Cũ).
     */
    std::string to_string(ToneStyle style = ToneStyle::NEW) const;

    /**
     * @brief Kiểm tra xem âm tiết có trống hay không.
     */
    bool is_empty() const {
        return initial.empty() && !glide.has_value() && vowel.empty() && final_c.empty();
    }

    /**
     * @brief Xoá 1 đơn vị ký tự hiển thị cuối cùng của âm tiết.
     * Ví dụ: xoá (x-o-a-s) -> xó (x-o-s), tuyến (t-u-y-e-e-n-s) -> tuyế.
     */
    void remove_last_char();

    /**
     * @brief Chuyển đổi trạng thái âm tiết hiện tại thành chuỗi phím gõ (Telex/VNI).
     */
    std::vector<char32_t> to_keys(InputMethod method) const;
};

/**
 * @brief Các modifiers điều khiển việc gõ (Shift, CapsLock...).
 */
struct Modifiers {
    bool shift = false;
    bool caps_lock = false;
    bool ctrl = false;
};

/**
 * @brief Ring buffer for word history (last committed words).
 */
struct WordHistory {
    static constexpr size_t CAPACITY = 10;
    std::u32string data[CAPACITY];
    size_t head = 0;
    size_t size = 0;

    void push(const std::u32string& word) {
        data[head] = word;
        head = (head + 1) % CAPACITY;
        if (size < CAPACITY)
            size++;
    }

    std::u32string pop() {
        if (size == 0)
            return U"";
        head = (head + CAPACITY - 1) % CAPACITY;
        size--;
        return data[head];
    }

    void clear() {
        head = 0;
        size = 0;
    }
};

/**
 * @brief Kết quả trả về sau mỗi phím gõ (FFI-compatible).
 */
struct EngineResult {
    /**
     * @brief Output characters in UTF-32.
     * Fixed size for FFI compatibility. 'count' defines valid entries.
     */
    uint32_t chars[32];

    /**
     * @brief Operation for frontend: 0=Pass, 1=Send (Insert/Replace), 2=Restore (English Fix)
     */
    uint8_t action;

    /**
     * @brief Number of characters to delete BEFORE inserting 'chars'.
     */
    uint8_t backspace;

    /**
     * @brief Number of valid characters in 'chars' array.
     */
    uint8_t count;

    /**
     * @brief Helper to convert result to UTF-8 string.
     */
    std::string to_string() const;
};

}  // namespace lotus_engine
