#pragma once

#include "lotus_engine/types.h"
#include <string>
#include <vector>

namespace lotus_engine {

class SyllableParser {
public:
    /**
     * @brief Phân tích một chuỗi tiếng Việt thô thành các thành phần âm tiết.
     * Thuật toán sử dụng Longest Match cho phụ âm đầu và cuối.
     */
    static Syllable parse(const std::string& raw);

private:
    static bool is_vowel(char32_t c);
    static std::u32string to_u32(const std::string& s);
    static std::string from_u32(const std::u32string& s);
};

} // namespace lotus_engine
