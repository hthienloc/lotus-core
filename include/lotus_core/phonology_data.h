#pragma once

#include "lotus_core/common.h"

#include <array>
#include <map>
#include <string>
#include <unordered_map>

namespace lotus_core {
namespace phonology {

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
    {{U'A', 0x0301}, U'Á'}, {{U'Ă', 0x0301}, U'Ắ'}, {{U'Â', 0x0301}, U'Ấ'}, {{U'E', 0x0301}, U'É'},
    {{U'Ê', 0x0301}, U'Ế'}, {{U'I', 0x0301}, U'Í'}, {{U'O', 0x0301}, U'Ó'}, {{U'Ô', 0x0301}, U'Ố'},
    {{U'Ơ', 0x0301}, U'Ớ'}, {{U'U', 0x0301}, U'Ú'}, {{U'Ư', 0x0301}, U'Ứ'}, {{U'Y', 0x0301}, U'Ý'},
    {{U'A', 0x0300}, U'À'}, {{U'Ă', 0x0300}, U'Ằ'}, {{U'Â', 0x0300}, U'Ầ'}, {{U'E', 0x0300}, U'È'},
    {{U'Ê', 0x0300}, U'Ề'}, {{U'I', 0x0300}, U'Ì'}, {{U'O', 0x0300}, U'Ò'}, {{U'Ô', 0x0300}, U'Ồ'},
    {{U'Ơ', 0x0300}, U'Ờ'}, {{U'U', 0x0300}, U'Ù'}, {{U'Ư', 0x0300}, U'Ừ'}, {{U'Y', 0x0300}, U'Ỳ'},
    {{U'A', 0x0309}, U'Ả'}, {{U'Ă', 0x0309}, U'Ẳ'}, {{U'Â', 0x0309}, U'Ẩ'}, {{U'E', 0x0309}, U'Ẻ'},
    {{U'Ê', 0x0309}, U'Ể'}, {{U'I', 0x0309}, U'Ỉ'}, {{U'O', 0x0309}, U'Ỏ'}, {{U'Ô', 0x0309}, U'Ổ'},
    {{U'Ơ', 0x0309}, U'Ở'}, {{U'U', 0x0309}, U'Ủ'}, {{U'Ư', 0x0309}, U'Ử'}, {{U'Y', 0x0309}, U'Ỷ'},
    {{U'A', 0x0303}, U'Ã'}, {{U'Ă', 0x0303}, U'Ẵ'}, {{U'Â', 0x0303}, U'Ẫ'}, {{U'E', 0x0303}, U'Ẽ'},
    {{U'Ê', 0x0303}, U'Ễ'}, {{U'I', 0x0303}, U'Ĩ'}, {{U'O', 0x0303}, U'Õ'}, {{U'Ô', 0x0303}, U'Ỗ'},
    {{U'Ơ', 0x0303}, U'Ỡ'}, {{U'U', 0x0303}, U'Ũ'}, {{U'Ư', 0x0303}, U'Ữ'}, {{U'Y', 0x0303}, U'Ỹ'},
    {{U'A', 0x0323}, U'Ạ'}, {{U'Ă', 0x0323}, U'Ặ'}, {{U'Â', 0x0323}, U'Ậ'}, {{U'E', 0x0323}, U'Ẹ'},
    {{U'Ê', 0x0323}, U'Ệ'}, {{U'I', 0x0323}, U'Ị'}, {{U'O', 0x0323}, U'Ọ'}, {{U'Ô', 0x0323}, U'Ộ'},
    {{U'Ơ', 0x0323}, U'Ợ'}, {{U'U', 0x0323}, U'Ụ'}, {{U'Ư', 0x0323}, U'Ự'}, {{U'Y', 0x0323}, U'Ỵ'},
    {{U'a', 0x0302}, U'â'}, {{U'e', 0x0302}, U'ê'}, {{U'o', 0x0302}, U'ô'}, {{U'A', 0x0302}, U'Â'},
    {{U'E', 0x0302}, U'Ê'}, {{U'O', 0x0302}, U'Ô'}, {{U'a', 0x0306}, U'ă'}, {{U'A', 0x0306}, U'Ă'},
    {{U'o', 0x031B}, U'ơ'}, {{U'u', 0x031B}, U'ư'}, {{U'O', 0x031B}, U'Ơ'}, {{U'U', 0x031B}, U'Ư'}};

constexpr std::u32string_view BASE_VOWELS = U"aăâeêioôơuưy";

struct GlideRule {
    char32_t glide_char;
    std::u32string_view initial_context; // empty means any initial
    std::u32string_view valid_next_chars;
};

// Map of glide rules: glide character -> (initial context -> valid next characters)
constexpr std::array<GlideRule, 3> GLIDE_RULES = {{
    {'o', U"", U"aeă"},
    {'u', U"q", U"aeiâêoôơuưy"},
    {'u', U"", U"êyâơô"}
}};

struct InitialOverrideRule {
    std::u32string_view initial;
    size_t new_len; // length to truncate to if condition is met
    bool requires_no_vowel_at_index; // if true, condition is input.size() <= 2 or !is_vowel(input[2])
    size_t check_index;
};

constexpr std::array<InitialOverrideRule, 2> INITIAL_OVERRIDE_RULES = {{
    {U"gi", 1, true, 2},
    {U"qu", 1, false, 0}
}};

}  // namespace phonology
}  // namespace lotus_core
