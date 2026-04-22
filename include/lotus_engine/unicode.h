#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "lotus_engine/common.h"

namespace lotus_engine {
namespace unicode {

/**
 * @brief Convert a UTF-8 string to a UTF-32 vector.
 */
inline std::u32string to_utf32(const std::string& utf8) {
    std::u32string u32;
    for (size_t i = 0; i < utf8.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        char32_t val = 0;
        if (c < 0x80) {
            val = c;
        } else if (c < 0xE0) {
            val = (c & 0x1F) << 6;
            val |= (utf8[++i] & 0x3F);
        } else if (c < 0xF0) {
            val = (c & 0x0F) << 12;
            val |= (utf8[++i] & 0x3F) << 6;
            val |= (utf8[++i] & 0x3F);
        } else {
            val = (c & 0x07) << 18;
            val |= (utf8[++i] & 0x3F) << 12;
            val |= (utf8[++i] & 0x3F) << 6;
            val |= (utf8[++i] & 0x3F);
        }
        u32.push_back(val);
    }
    return u32;
}

/**
 * @brief Convert a UTF-32 string/char to UTF-8.
 */
inline std::string to_utf8(char32_t cp) {
    std::string utf8;
    if (cp < 0x80) {
        utf8 += static_cast<char>(cp);
    } else if (cp < 0x800) {
        utf8 += static_cast<char>(0xC0 | (cp >> 6));
        utf8 += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        utf8 += static_cast<char>(0xE0 | (cp >> 12));
        utf8 += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        utf8 += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        utf8 += static_cast<char>(0xF0 | (cp >> 18));
        utf8 += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        utf8 += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        utf8 += static_cast<char>(0x80 | (cp & 0x3F));
    }
    return utf8;
}

inline std::string to_utf8(const std::u32string& u32) {
    std::string res;
    for (auto cp : u32)
        res += to_utf8(cp);
    return res;
}

/**
 * @brief Case transformation for Vietnamese characters.
 */
inline char32_t to_lower(char32_t cp) {
    if (cp >= 'A' && cp <= 'Z')
        return cp + ('a' - 'A');
    switch (cp) {
        case U'À':
            return U'à';
        case U'Á':
            return U'á';
        case U'Â':
            return U'â';
        case U'Ã':
            return U'ã';
        case U'È':
            return U'è';
        case U'É':
            return U'é';
        case U'Ê':
            return U'ê';
        case U'Ì':
            return U'ì';
        case U'Í':
            return U'í';
        case U'Ò':
            return U'ò';
        case U'Ó':
            return U'ó';
        case U'Ô':
            return U'ô';
        case U'Õ':
            return U'õ';
        case U'Ù':
            return U'ù';
        case U'Ú':
            return U'ú';
        case U'Ý':
            return U'ý';
        case U'Ă':
            return U'ă';
        case U'Đ':
            return U'đ';
        case U'Ĩ':
            return U'ĩ';
        case U'Ũ':
            return U'ũ';
        case U'Ơ':
            return U'ơ';
        case U'Ư':
            return U'ư';
        case U'Ạ':
            return U'ạ';
        case U'Ả':
            return U'ả';
        case U'Ấ':
            return U'ấ';
        case U'Ầ':
            return U'ầ';
        case U'Ẩ':
            return U'ẩ';
        case U'Ẫ':
            return U'ẫ';
        case U'Ậ':
            return U'ậ';
        case U'Ắ':
            return U'ắ';
        case U'Ằ':
            return U'ằ';
        case U'Ẳ':
            return U'ẳ';
        case U'Ẵ':
            return U'ẵ';
        case U'Ặ':
            return U'ặ';
        case U'Ẹ':
            return U'ẹ';
        case U'Ẻ':
            return U'ẻ';
        case U'Ẽ':
            return U'ẽ';
        case U'Ế':
            return U'ế';
        case U'Ề':
            return U'ề';
        case U'Ể':
            return U'ể';
        case U'Ễ':
            return U'ễ';
        case U'Ệ':
            return U'ệ';
        case U'Ỉ':
            return U'ỉ';
        case U'Ị':
            return U'ị';
        case U'Ọ':
            return U'ọ';
        case U'Ỏ':
            return U'ỏ';
        case U'Ố':
            return U'ố';
        case U'Ồ':
            return U'ồ';
        case U'Ổ':
            return U'ổ';
        case U'Ỗ':
            return U'ỗ';
        case U'Ộ':
            return U'ộ';
        case U'Ớ':
            return U'ớ';
        case U'Ờ':
            return U'ờ';
        case U'Ở':
            return U'ở';
        case U'Ỡ':
            return U'ỡ';
        case U'Ợ':
            return U'ợ';
        case U'Ụ':
            return U'ụ';
        case U'Ủ':
            return U'ủ';
        case U'Ứ':
            return U'ứ';
        case U'Ừ':
            return U'ừ';
        case U'Ử':
            return U'ử';
        case U'Ữ':
            return U'ữ';
        case U'Ự':
            return U'ự';
        case U'Ỳ':
            return U'ỳ';
        case U'Ỵ':
            return U'ỵ';
        case U'Ỷ':
            return U'ỷ';
        case U'Ỹ':
            return U'ỹ';
        default:
            return cp;
    }
}

inline char32_t to_upper(char32_t cp) {
    if (cp >= 'a' && cp <= 'z')
        return cp - ('a' - 'A');
    switch (cp) {
        case U'à':
            return U'À';
        case U'á':
            return U'Á';
        case U'â':
            return U'Â';
        case U'ã':
            return U'Ã';
        case U'è':
            return U'È';
        case U'é':
            return U'É';
        case U'ê':
            return U'Ê';
        case U'ì':
            return U'Ì';
        case U'í':
            return U'Í';
        case U'ò':
            return U'Ò';
        case U'ó':
            return U'Ó';
        case U'ô':
            return U'Ô';
        case U'õ':
            return U'Õ';
        case U'ù':
            return U'Ù';
        case U'ú':
            return U'Ú';
        case U'ý':
            return U'Ý';
        case U'ă':
            return U'Ă';
        case U'đ':
            return U'Đ';
        case U'ĩ':
            return U'Ĩ';
        case U'ũ':
            return U'Ũ';
        case U'ơ':
            return U'Ơ';
        case U'ư':
            return U'Ư';
        case U'ạ':
            return U'Ạ';
        case U'ả':
            return U'Ả';
        case U'ấ':
            return U'Ấ';
        case U'ầ':
            return U'Ầ';
        case U'ẩ':
            return U'Ẩ';
        case U'ẫ':
            return U'Ẫ';
        case U'ậ':
            return U'Ậ';
        case U'ắ':
            return U'Ắ';
        case U'ằ':
            return U'Ằ';
        case U'ẳ':
            return U'Ẳ';
        case U'ẵ':
            return U'Ẵ';
        case U'ặ':
            return U'Ặ';
        case U'ẹ':
            return U'Ẹ';
        case U'ẻ':
            return U'Ẻ';
        case U'ẽ':
            return U'Ẽ';
        case U'ế':
            return U'Ế';
        case U'ề':
            return U'Ề';
        case U'ể':
            return U'Ể';
        case U'ễ':
            return U'Ễ';
        case U'ệ':
            return U'Ệ';
        case U'ỉ':
            return U'Ỉ';
        case U'ị':
            return U'Ị';
        case U'ọ':
            return U'Ọ';
        case U'ỏ':
            return U'Ỏ';
        case U'ố':
            return U'Ố';
        case U'ồ':
            return U'Ồ';
        case U'ổ':
            return U'Ổ';
        case U'ỗ':
            return U'Ỗ';
        case U'ộ':
            return U'Ộ';
        case U'ớ':
            return U'Ớ';
        case U'ờ':
            return U'Ờ';
        case U'ở':
            return U'Ở';
        case U'ỡ':
            return U'Ỡ';
        case U'ợ':
            return U'Ợ';
        case U'ụ':
            return U'Ụ';
        case U'ủ':
            return U'Ủ';
        case U'ứ':
            return U'Ứ';
        case U'ừ':
            return U'Ừ';
        case U'ử':
            return U'Ử';
        case U'ữ':
            return U'Ữ';
        case U'ự':
            return U'Ự';
        case U'ỳ':
            return U'Ỳ';
        case U'ỵ':
            return U'Ỵ';
        case U'ỷ':
            return U'Ỷ';
        case U'ỹ':
            return U'Ỹ';
        default:
            return cp;
    }
}

inline std::string to_lower(const std::string& input) {
    std::u32string u32 = to_utf32(input);
    for (auto& cp : u32)
        cp = to_lower(cp);
    return to_utf8(u32);
}

inline std::u32string to_lower(const std::u32string& input) {
    std::u32string res = input;
    for (auto& cp : res)
        cp = to_lower(cp);
    return res;
}

inline std::string to_upper(const std::string& input) {
    std::u32string u32 = to_utf32(input);
    for (auto& cp : u32)
        cp = to_upper(cp);
    return to_utf8(u32);
}

/**
 * @brief Returns the visual width of a UTF-8 string (number of codepoints).
 */
inline size_t display_width(const std::string& utf8) {
    return to_utf32(utf8).size();
}

/**
 * @brief Strips Vietnamese specific tone marks while keeping centering diacritics (ê, ô, etc).
 */
inline char32_t strip_tone(char32_t cp) {
    switch (cp) {
        case U'à':
        case U'á':
        case U'ã':
        case U'ạ':
        case U'ả':
            return U'a';
        case U'ằ':
        case U'ắ':
        case U'ẵ':
        case U'ặ':
        case U'ẳ':
            return U'ă';
        case U'ầ':
        case U'ấ':
        case U'ẫ':
        case U'ậ':
        case U'ẩ':
            return U'â';
        case U'è':
        case U'é':
        case U'ẽ':
        case U'ẹ':
        case U'ẻ':
            return U'e';
        case U'ề':
        case U'ế':
        case U'ễ':
        case U'ệ':
        case U'ể':
            return U'ê';
        case U'ì':
        case U'í':
        case U'ĩ':
        case U'ị':
        case U'ỉ':
            return U'i';
        case U'ò':
        case U'ó':
        case U'õ':
        case U'ọ':
        case U'ỏ':
            return U'o';
        case U'ồ':
        case U'ố':
        case U'ỗ':
        case U'ộ':
        case U'ổ':
            return U'ô';
        case U'ờ':
        case U'ớ':
        case U'ỡ':
        case U'ợ':
        case U'ở':
            return U'ơ';
        case U'ù':
        case U'ú':
        case U'ũ':
        case U'ụ':
        case U'ủ':
            return U'u';
        case U'ừ':
        case U'ứ':
        case U'ữ':
        case U'ự':
        case U'ử':
            return U'ư';
        case U'ỳ':
        case U'ý':
        case U'ỹ':
        case U'ỵ':
        case U'ỷ':
            return U'y';
        default:
            return cp;
    }
}

/**
 * @brief Returns the Tone of a precomposed Vietnamese character.
 */
inline Tone get_tone(char32_t cp) {
    switch (cp) {
        case U'á': case U'ắ': case U'ấ': case U'é': case U'ế': case U'í': 
        case U'ó': case U'ố': case U'ớ': case U'ú': case U'ứ': case U'ý':
            return Tone::ACUTE;
        case U'à': case U'ằ': case U'ầ': case U'è': case U'ề': case U'ì':
        case U'ò': case U'ồ': case U'ờ': case U'ù': case U'ừ': case U'ỳ':
            return Tone::GRAVE;
        case U'ả': case U'ẳ': case U'ẩ': case U'ẻ': case U'ể': case U'ỉ':
        case U'ỏ': case U'ổ': case U'ở': case U'ủ': case U'ử': case U'ỷ':
            return Tone::HOOK;
        case U'ã': case U'ẵ': case U'ẫ': case U'ẽ': case U'ễ': case U'ĩ':
        case U'õ': case U'ỗ': case U'ỡ': case U'ũ': case U'ữ': case U'ỹ':
            return Tone::TILDE;
        case U'ạ': case U'ặ': case U'ậ': case U'ẹ': case U'ệ': case U'ị':
        case U'ọ': case U'ộ': case U'ợ': case U'ụ': case U'ự': case U'ỵ':
            return Tone::DOT;
        default:
            return Tone::NONE;
    }
}

/**
 * @brief Normalize Vietnamese text to NFC (Precomposed) form.
 */
inline std::string normalize_nfc(const std::string& input) {
    std::u32string u32 = to_utf32(input);
    std::u32string res;

    auto get_nfc = [](char32_t base, char32_t mark) -> char32_t {
        if (mark == 0x0301) {  // Sắc
            switch (base) {
                case 'a':
                    return U'á';
                case 0x0103:
                    return U'ắ';
                case 0x00E2:
                    return U'ấ';  // a, ă, â
                case 'e':
                    return U'é';
                case 0x00EA:
                    return U'ế';  // e, ê
                case 'i':
                    return U'í';
                case 'o':
                    return U'ó';
                case 0x00F4:
                    return U'ố';
                case 0x01A1:
                    return U'ớ';  // o, ô, ơ
                case 'u':
                    return U'ú';
                case 0x01B0:
                    return U'ứ';  // u, ư
                case 'y':
                    return U'ý';
            }
        } else if (mark == 0x0300) {  // Huyền
            switch (base) {
                case 'a':
                    return U'à';
                case 0x0103:
                    return U'ằ';
                case 0x00E2:
                    return U'ầ';
                case 'e':
                    return U'è';
                case 0x00EA:
                    return U'ề';
                case 'i':
                    return U'ì';
                case 'o':
                    return U'ò';
                case 0x00F4:
                    return U'ồ';
                case 0x01A1:
                    return U'ờ';
                case 'u':
                    return U'ù';
                case 0x01B0:
                    return U'ừ';
                case 'y':
                    return U'ỳ';
            }
        } else if (mark == 0x0309) {  // Hỏi
            switch (base) {
                case 'a':
                    return 0x1EA3;
                case 0x0103:
                    return 0x1EB3;
                case 0x00E2:
                    return 0x1EA9;
                case 'e':
                    return 0x1EBB;
                case 0x00EA:
                    return 0x1EC3;
                case 'i':
                    return 0x1EC9;
                case 'o':
                    return 0x1ECF;
                case 0x00F4:
                    return 0x1ED5;
                case 0x01A1:
                    return 0x1EDF;
                case 'u':
                    return 0x1EE7;
                case 0x01B0:
                    return 0x1EED;
                case 'y':
                    return 0x1EF7;
            }
        } else if (mark == 0x0303) {  // Ngã
            switch (base) {
                case 'a':
                    return U'ã';
                case 0x0103:
                    return U'ẵ';
                case 0x00E2:
                    return U'ẫ';
                case 'e':
                    return U'ẽ';
                case 0x00EA:
                    return U'ễ';
                case 'i':
                    return U'ĩ';
                case 'o':
                    return U'õ';
                case 0x00F4:
                    return U'ỗ';
                case 0x01A1:
                    return U'ỡ';
                case 'u':
                    return U'ũ';
                case 0x01B0:
                    return U'ữ';
                case 'y':
                    return U'ỹ';
            }
        } else if (mark == 0x0323) {  // Nặng
            switch (base) {
                case 'a':
                    return U'ạ';
                case 0x0103:
                    return U'ặ';
                case 0x00E2:
                    return U'ậ';
                case 'e':
                    return U'ẹ';
                case 0x00EA:
                    return U'ệ';
                case 'i':
                    return U'ị';
                case 'o':
                    return U'ọ';
                case 0x00F4:
                    return U'ộ';
                case 0x01A1:
                    return U'ợ';
                case 'u':
                    return U'ụ';
                case 0x01B0:
                    return U'ự';
                case 'y':
                    return U'ỵ';
            }
        } else if (mark == 0x0302) {  // Circumflex (^)
            switch (base) {
                case 'a':
                    return U'â';
                case 'e':
                    return U'ê';
                case 'o':
                    return U'ô';
                case 'A':
                    return U'Â';
                case 'E':
                    return U'Ê';
                case 'O':
                    return U'Ô';
            }
        } else if (mark == 0x0306) {  // Breve (()
            switch (base) {
                case 'a':
                    return U'ă';
                case 'A':
                    return U'Ă';
            }
        } else if (mark == 0x031B) {  // Horn (+)
            switch (base) {
                case 'o':
                    return U'ơ';
                case 'u':
                    return U'ư';
                case 'O':
                    return U'Ơ';
                case 'U':
                    return U'Ư';
            }
        }
        return 0;
    };

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t current = u32[i];
        while (i + 1 < u32.size()) {
            char32_t next = u32[i + 1];
            if (next >= 0x0300 && next <= 0x036F) {
                char32_t combined = get_nfc(current, next);
                if (combined != 0) {
                    current = combined;
                    i++;
                    continue;
                }
            }
            break;
        }
        res.push_back(current);
    }
    return to_utf8(res);
}

/**
 * @brief Simple string replacement utility.
 */
inline void replace_all(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

}  // namespace unicode
}  // namespace lotus_engine
