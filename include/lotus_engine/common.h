#pragma once

#include <cstdint>

namespace lotus_engine {

/**
 * @brief Engine trung tâm điều phối việc gõ tiếng Việt.
 */
enum class InputMethod : uint8_t { TELEX, VNI };

/**
 * @brief Dấu thanh trong tiếng Việt.
 */
enum class Tone : uint8_t {
    NONE = 0,  // Ngang
    ACUTE,     // Sắc
    GRAVE,     // Huyền
    HOOK,      // Hỏi
    TILDE,     // Ngã
    DOT        // Nặng
};

/**
 * @brief Kiểu bỏ dấu tiếng Việt (mới/cũ).
 */
enum class ToneStyle : uint8_t {
    OLD,  // hòa, họa (kiểu cũ)
    NEW   // hoà, hoạ (kiểu mới, mặc định)
};

/**
 * @brief Tùy chọn xử lý phím 'w' tự do (Telex).
 */
enum class FreeWOption : uint8_t {
    OFF,        // w -> w
    NON_START,  // w -> ư (nếu không phải ký tự đầu tiên)
    ALWAYS      // w -> ư (ở mọi nơi)
};

}  // namespace lotus_engine
