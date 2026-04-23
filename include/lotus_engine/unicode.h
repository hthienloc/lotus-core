#pragma once

#include "lotus_engine/common.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lotus_engine {
namespace unicode {

namespace detail {

inline const std::unordered_map<char32_t, char32_t> LOWER_MAP = {
    {U'À', U'à'}, {U'Á', U'á'}, {U'Â', U'â'}, {U'Ã', U'ã'}, {U'È', U'è'}, {U'É', U'é'},
    {U'Ê', U'ê'}, {U'Ì', U'ì'}, {U'Í', U'í'}, {U'Ò', U'ò'}, {U'Ó', U'ó'}, {U'Ô', U'ô'},
    {U'Õ', U'õ'}, {U'Ù', U'ù'}, {U'Ú', U'ú'}, {U'Ý', U'ý'}, {U'Ă', U'ă'}, {U'Đ', U'đ'},
    {U'Ĩ', U'ĩ'}, {U'Ũ', U'ũ'}, {U'Ơ', U'ơ'}, {U'Ư', U'ư'}, {U'Ạ', U'ạ'}, {U'Ả', U'ả'},
    {U'Ấ', U'ấ'}, {U'Ầ', U'ầ'}, {U'Ẩ', U'ẩ'}, {U'Ẫ', U'ẫ'}, {U'Ậ', U'ậ'}, {U'Ắ', U'ắ'},
    {U'Ằ', U'ằ'}, {U'Ẳ', U'ẳ'}, {U'Ẵ', U'ẵ'}, {U'Ặ', U'ặ'}, {U'Ẹ', U'ẹ'}, {U'Ẻ', U'ẻ'},
    {U'Ẽ', U'ẽ'}, {U'Ế', U'ế'}, {U'Ề', U'ề'}, {U'Ể', U'ể'}, {U'Ễ', U'ễ'}, {U'Ệ', U'ệ'},
    {U'Ỉ', U'ỉ'}, {U'Ị', U'ị'}, {U'Ọ', U'ọ'}, {U'Ỏ', U'ỏ'}, {U'Ố', U'ố'}, {U'Ồ', U'ồ'},
    {U'Ổ', U'ổ'}, {U'Ỗ', U'ỗ'}, {U'Ộ', U'ộ'}, {U'Ớ', U'ớ'}, {U'Ờ', U'ờ'}, {U'Ở', U'ở'},
    {U'Ỡ', U'ỡ'}, {U'Ợ', U'ợ'}, {U'Ụ', U'ụ'}, {U'Ủ', U'ủ'}, {U'Ứ', U'ứ'}, {U'Ừ', U'ừ'},
    {U'Ử', U'ử'}, {U'Ữ', U'ữ'}, {U'Ự', U'ự'}, {U'Ỳ', U'ỳ'}, {U'Ỵ', U'ỵ'}, {U'Ỷ', U'ỷ'},
    {U'Ỹ', U'ỹ'}};

inline const std::unordered_map<char32_t, char32_t> UPPER_MAP = []() {
    std::unordered_map<char32_t, char32_t> m;
    for (auto const& [upper, lower] : LOWER_MAP)
        m[lower] = upper;
    return m;
}();

inline const std::unordered_map<char32_t, char32_t> STRIP_TONE_MAP = {
    {U'à', U'a'}, {U'á', U'a'}, {U'ã', U'a'}, {U'ạ', U'a'}, {U'ả', U'a'}, {U'ằ', U'ă'},
    {U'ắ', U'ă'}, {U'ẵ', U'ă'}, {U'ặ', U'ă'}, {U'ẳ', U'ă'}, {U'ầ', U'â'}, {U'ấ', U'â'},
    {U'ẫ', U'â'}, {U'ậ', U'â'}, {U'ẩ', U'â'}, {U'è', U'e'}, {U'é', U'e'}, {U'ẽ', U'e'},
    {U'ẹ', U'e'}, {U'ẻ', U'e'}, {U'ề', U'ê'}, {U'ế', U'ê'}, {U'ễ', U'ê'}, {U'ệ', U'ê'},
    {U'ể', U'ê'}, {U'ì', U'i'}, {U'í', U'i'}, {U'ĩ', U'i'}, {U'ị', U'i'}, {U'ỉ', U'i'},
    {U'ò', U'o'}, {U'ó', U'o'}, {U'õ', U'o'}, {U'ọ', U'o'}, {U'ỏ', U'o'}, {U'ồ', U'ô'},
    {U'ố', U'ô'}, {U'ỗ', U'ô'}, {U'ộ', U'ô'}, {U'ổ', U'ô'}, {U'ờ', U'ơ'}, {U'ớ', U'ơ'},
    {U'ỡ', U'ơ'}, {U'ợ', U'ơ'}, {U'ở', U'ơ'}, {U'ù', U'u'}, {U'ú', U'u'}, {U'ũ', U'u'},
    {U'ụ', U'u'}, {U'ủ', U'u'}, {U'ừ', U'ư'}, {U'ứ', U'ư'}, {U'ữ', U'ư'}, {U'ự', U'ư'},
    {U'ử', U'ư'}, {U'ỳ', U'y'}, {U'ý', U'y'}, {U'ỹ', U'y'}, {U'ỵ', U'y'}, {U'ỷ', U'y'}};

inline const std::unordered_map<char32_t, Tone> TONE_MAP = {
    {U'á', Tone::ACUTE}, {U'ắ', Tone::ACUTE}, {U'ấ', Tone::ACUTE}, {U'é', Tone::ACUTE},
    {U'ế', Tone::ACUTE}, {U'í', Tone::ACUTE}, {U'ó', Tone::ACUTE}, {U'ố', Tone::ACUTE},
    {U'ớ', Tone::ACUTE}, {U'ú', Tone::ACUTE}, {U'ứ', Tone::ACUTE}, {U'ý', Tone::ACUTE},
    {U'à', Tone::GRAVE}, {U'ằ', Tone::GRAVE}, {U'ầ', Tone::GRAVE}, {U'è', Tone::GRAVE},
    {U'ề', Tone::GRAVE}, {U'ì', Tone::GRAVE}, {U'ò', Tone::GRAVE}, {U'ồ', Tone::GRAVE},
    {U'ờ', Tone::GRAVE}, {U'ù', Tone::GRAVE}, {U'ừ', Tone::GRAVE}, {U'ỳ', Tone::GRAVE},
    {U'ả', Tone::HOOK},  {U'ẳ', Tone::HOOK},  {U'ẩ', Tone::HOOK},  {U'ẻ', Tone::HOOK},
    {U'ể', Tone::HOOK},  {U'ỉ', Tone::HOOK},  {U'ỏ', Tone::HOOK},  {U'ổ', Tone::HOOK},
    {U'ở', Tone::HOOK},  {U'ủ', Tone::HOOK},  {U'ử', Tone::HOOK},  {U'ỷ', Tone::HOOK},
    {U'ã', Tone::TILDE}, {U'ẵ', Tone::TILDE}, {U'ẫ', Tone::TILDE}, {U'ẽ', Tone::TILDE},
    {U'ễ', Tone::TILDE}, {U'ĩ', Tone::TILDE}, {U'õ', Tone::TILDE}, {U'ỗ', Tone::TILDE},
    {U'ỡ', Tone::TILDE}, {U'ũ', Tone::TILDE}, {U'ữ', Tone::TILDE}, {U'ỹ', Tone::TILDE},
    {U'ạ', Tone::DOT},   {U'ặ', Tone::DOT},   {U'ậ', Tone::DOT},   {U'ẹ', Tone::DOT},
    {U'ệ', Tone::DOT},   {U'ị', Tone::DOT},   {U'ọ', Tone::DOT},   {U'ộ', Tone::DOT},
    {U'ợ', Tone::DOT},   {U'ụ', Tone::DOT},   {U'ự', Tone::DOT},   {U'ỵ', Tone::DOT}};

inline const std::map<std::pair<char32_t, char32_t>, char32_t> COMPOSITION_MAP = {
    {{U'a', 0x0301}, U'á'}, {{U'ă', 0x0301}, U'ắ'}, {{U'â', 0x0301}, U'ấ'}, {{U'e', 0x0301}, U'é'},
    {{U'ê', 0x0301}, U'ế'}, {{U'i', 0x0301}, U'í'}, {{U'o', 0x0301}, U'ó'}, {{U'ô', 0x0301}, U'ố'},
    {{U'ơ', 0x0301}, U'ớ'}, {{U'u', 0x0301}, U'ú'}, {{U'ư', 0x0301}, U'ứ'}, {{U'y', 0x0301}, U'ý'},
    {{U'a', 0x0300}, U'à'}, {{U'ă', 0x0300}, U'ằ'}, {{U'â', 0x0300}, U'ầ'}, {{U'e', 0x0300}, U'è'},
    {{U'ê', 0x0300}, U'ề'}, {{U'i', 0x0300}, U'ì'}, {{U'o', 0x0300}, U'ò'}, {{U'ô', 0x0300}, U'ồ'},
    {{U'ơ', 0x0300}, U'ờ'}, {{U'u', 0x0300}, U'ù'}, {{U'ư', 0x0300}, U'ừ'}, {{U'y', 0x0300}, U'ỳ'},
    {{U'a', 0x0309}, U'ả'}, {{U'ă', 0x0309}, U'ẳ'}, {{U'â', 0x0309}, U'ẩ'}, {{U'e', 0x0309}, U'ẻ'},
    {{U'ê', 0x0309}, U'ể'}, {{U'i', 0x0309}, U'ỉ'}, {{U'o', 0x0309}, U'ỏ'}, {{U'ô', 0x0309}, U'ổ'},
    {{U'ơ', 0x0309}, U'ở'}, {{U'u', 0x0309}, U'ủ'}, {{U'ư', 0x0309}, U'ử'}, {{U'y', 0x0309}, U'ỷ'},
    {{U'a', 0x0303}, U'ã'}, {{U'ă', 0x0303}, U'ẵ'}, {{U'â', 0x0303}, U'ẫ'}, {{U'e', 0x0303}, U'ẽ'},
    {{U'ê', 0x0303}, U'ễ'}, {{U'i', 0x0303}, U'ĩ'}, {{U'o', 0x0303}, U'õ'}, {{U'ô', 0x0303}, U'ỗ'},
    {{U'ơ', 0x0303}, U'ỡ'}, {{U'u', 0x0303}, U'ũ'}, {{U'ư', 0x0303}, U'ữ'}, {{U'y', 0x0303}, U'ỹ'},
    {{U'a', 0x0323}, U'ạ'}, {{U'ă', 0x0323}, U'ặ'}, {{U'â', 0x0323}, U'ậ'}, {{U'e', 0x0323}, U'ẹ'},
    {{U'ê', 0x0323}, U'ệ'}, {{U'i', 0x0323}, U'ị'}, {{U'o', 0x0323}, U'ọ'}, {{U'ô', 0x0323}, U'ộ'},
    {{U'ơ', 0x0323}, U'ợ'}, {{U'u', 0x0323}, U'ụ'}, {{U'ư', 0x0323}, U'ự'}, {{U'y', 0x0323}, U'ỵ'},
    {{U'a', 0x0302}, U'â'}, {{U'e', 0x0302}, U'ê'}, {{U'o', 0x0302}, U'ô'}, {{U'A', 0x0302}, U'Â'},
    {{U'E', 0x0302}, U'Ê'}, {{U'O', 0x0302}, U'Ô'}, {{U'a', 0x0306}, U'ă'}, {{U'A', 0x0306}, U'Ă'},
    {{U'o', 0x031B}, U'ơ'}, {{U'u', 0x031B}, U'ư'}, {{U'O', 0x031B}, U'Ơ'}, {{U'U', 0x031B}, U'Ư'}};

}  // namespace detail

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
    auto it = detail::LOWER_MAP.find(cp);
    return it != detail::LOWER_MAP.end() ? it->second : cp;
}

inline char32_t to_upper(char32_t cp) {
    if (cp >= 'a' && cp <= 'z')
        return cp - ('a' - 'A');
    auto it = detail::UPPER_MAP.find(cp);
    return it != detail::UPPER_MAP.end() ? it->second : cp;
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
    auto it = detail::STRIP_TONE_MAP.find(cp);
    return it != detail::STRIP_TONE_MAP.end() ? it->second : cp;
}

/**
 * @brief Returns the Tone of a precomposed Vietnamese character.
 */
inline Tone get_tone(char32_t cp) {
    auto it = detail::TONE_MAP.find(cp);
    return it != detail::TONE_MAP.end() ? it->second : Tone::NONE;
}

/**
 * @brief Normalize Vietnamese text to NFC (Precomposed) form.
 */
inline std::string normalize_nfc(const std::string& input) {
    std::u32string u32 = to_utf32(input);
    std::u32string res;

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t current = u32[i];
        while (i + 1 < u32.size()) {
            char32_t next = u32[i + 1];
            if (next >= 0x0300 && next <= 0x036F) {
                auto it = detail::COMPOSITION_MAP.find({current, next});
                if (it != detail::COMPOSITION_MAP.end()) {
                    current = it->second;
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
