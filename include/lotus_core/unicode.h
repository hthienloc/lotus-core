#pragma once

#include "lotus_core/common.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <array>
#include <optional>
#include <string>
#include <unordered_map>
#include <string_view>
#include <vector>

#include "lotus_core/phonology_data.h"

namespace lotus_core {
namespace unicode {

/**
 * @brief Convert a UTF-8 string to a UTF-32 vector.
 */
inline std::u32string to_utf32(std::string_view utf8) {
    std::u32string u32;
    u32.reserve(utf8.size());
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

inline std::string to_utf8(std::u32string_view u32) {
    std::string res;
    res.reserve(u32.size() * 3); // Conservative estimate
    for (auto cp : u32) {
        if (cp < 0x80) {
            res += static_cast<char>(cp);
        } else if (cp < 0x800) {
            res += static_cast<char>(0xC0 | (cp >> 6));
            res += static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            res += static_cast<char>(0xE0 | (cp >> 12));
            res += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            res += static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            res += static_cast<char>(0xF0 | (cp >> 18));
            res += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            res += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            res += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }
    return res;
}

/**
 * @brief Case transformation for Vietnamese characters.
 */
inline char32_t to_lower(char32_t cp) {
    if (cp >= 'A' && cp <= 'Z')
        return cp + ('a' - 'A');
    auto it = phonology::LOWER_MAP.find(cp);
    return it != phonology::LOWER_MAP.end() ? it->second : cp;
}

inline char32_t to_upper(char32_t cp) {
    if (cp >= 'a' && cp <= 'z')
        return cp - ('a' - 'A');
    auto it = phonology::UPPER_MAP.find(cp);
    return it != phonology::UPPER_MAP.end() ? it->second : cp;
}

inline std::string to_lower(std::string_view input) {
    std::u32string u32 = to_utf32(input);
    for (auto& cp : u32)
        cp = to_lower(cp);
    return to_utf8(u32);
}

inline std::u32string to_lower(std::u32string_view input) {
    std::u32string res(input);
    for (auto& cp : res)
        cp = to_lower(cp);
    return res;
}

/**
 * @brief Checks if a character is alphabetical (ASCII or Vietnamese letters).
 */
inline bool is_alpha(char32_t cp) {
    if ((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z'))
        return true;
    return phonology::LOWER_MAP.count(cp) > 0 || phonology::UPPER_MAP.count(cp) > 0;
}

inline std::string to_upper(std::string_view input) {
    std::u32string u32 = to_utf32(input);
    for (auto& cp : u32)
        cp = to_upper(cp);
    return to_utf8(u32);
}

/**
 * @brief Returns the visual width of a UTF-8 string (number of codepoints).
 */
inline size_t display_width(std::string_view utf8) {
    size_t count = 0;
    for (size_t i = 0; i < utf8.size(); ) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        if (c < 0x80) {
            i += 1;
        } else if (c < 0xE0) {
            i += 2;
        } else if (c < 0xF0) {
            i += 3;
        } else {
            i += 4;
        }
        count++;
    }
    return count;
}

/**
 * @brief Strips Vietnamese specific tone marks while keeping centering diacritics (ê, ô, etc).
 */
inline char32_t strip_tone(char32_t cp) {
    auto it = phonology::STRIP_TONE_MAP.find(cp);
    return it != phonology::STRIP_TONE_MAP.end() ? it->second : cp;
}

/**
 * @brief Returns the Tone of a precomposed Vietnamese character.
 */
inline Tone get_tone(char32_t cp) {
    auto it = phonology::TONE_MAP.find(cp);
    return it != phonology::TONE_MAP.end() ? it->second : Tone::NONE;
}

/**
 * @brief Normalize Vietnamese text to NFC (Precomposed) form.
 */
inline std::u32string normalize_nfc(std::u32string_view u32) {
    std::u32string res;
    res.reserve(u32.size());

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t current = u32[i];
        while (i + 1 < u32.size()) {
            char32_t next = u32[i + 1];
            if (next >= 0x0300 && next <= 0x036F) {
                auto it = phonology::COMPOSITION_MAP.find({current, next});
                if (it != phonology::COMPOSITION_MAP.end()) {
                    current = it->second;
                    i++;
                    continue;
                }
            }
            break;
        }
        res.push_back(current);
    }
    return res;
}

inline std::string normalize_nfc(std::string_view input) {
    return to_utf8(normalize_nfc(to_utf32(input)));
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
}  // namespace lotus_core
