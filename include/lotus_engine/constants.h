#pragma once

#include <string_view>
#include <vector>
#include <array>

namespace lotus_engine {

namespace constants {

const std::vector<std::string_view> VALID_INITIALS = {
    "b", "c", "d", "đ",  "g",  "h",  "k",  "l",  "m",  "n",  "p",  "q",  "r",  "s",
    "t", "v", "x", "ch", "gh", "gi", "kh", "ng", "nh", "ph", "qu", "th", "tr", "ngh"};

const std::vector<std::string_view> VALID_GLIDES = {"o", "u"};

const std::vector<std::string_view> VALID_NUCLEI = {
    // Monophthongs
    "a", "ă", "â", "e", "ê", "i", "o", "ô", "ơ", "u", "ư", "y",
    // Centering Diphthongs
    "ia", "iê", "ua", "uô", "ưa", "ươ", "yê", "ya",
    // Glide-based Diphthongs
    "oa", "oe", "oă", "uâ", "uê", "uơ", "uy",
    // Closing Diphthongs
    "ai", "ao", "au", "âu", "ay", "ây", "eo", "êu", "iu", "oi", "ôi", "ơi", "ui", "ưi", "ưu",
    // Triphthongs & Long Vowels
    "iêu", "yêu", "uôi", "ươi", "ươu"};

const std::vector<std::string_view> VALID_FINALS = {"c", "ch", "m", "n", "ng", "nh",
                                                    "p", "t",  "i", "y", "o",  "u"};

const std::vector<std::string_view> ENGLISH_CLUSTERS = {
    "br", "cl",  "cr", "dr", "dw", "fl", "fr", "gl",  "gr",  "pl",  "pr", "sc",  "scr",
    "sh", "shr", "sk", "sl", "sm", "sn", "sp", "spl", "spr", "squ", "st", "str", "sw"};

const std::vector<std::string_view> INVALID_FINALS = {"b", "d", "f", "g", "j", "k",
                                                      "l", "r", "s", "v", "x", "z"};

const std::vector<char32_t> INVALID_FINALS_U32 = {'b', 'd', 'f', 'g', 'j', 'k',
                                                  'l', 'r', 's', 'v', 'x', 'z'};

const std::vector<std::u32string_view> VALID_INITIALS_U32 = {
    U"b", U"c", U"d", U"đ",  U"g",  U"h",  U"k",  U"l",  U"m",  U"n",  U"p",  U"q",  U"r",  U"s",
    U"t", U"v", U"x", U"ch", U"gh", U"gi", U"kh", U"ng", U"nh", U"ph", U"qu", U"th", U"tr", U"ngh"};

const std::string_view TELEX_TONE_MARKERS = "srfxj";
const std::string_view VNI_TONE_MARKERS = "12345";
const std::string_view TELEX_MARKERS = "srfxjzw0adeow";
const std::string_view VOWELS = "aeiouyw";

const std::vector<std::u32string_view> VALID_GLIDES_U32 = {U"o", U"u"};

const std::vector<std::u32string_view> VALID_NUCLEI_U32 = {
    U"a",  U"ă",  U"â",  U"e",  U"ê",  U"i",  U"o",   U"ô",   U"ơ",   U"u",   U"ư",  U"y",
    U"ia", U"iê", U"ua", U"uô", U"ưa", U"ươ", U"yê",  U"ya",  U"oa",  U"oe",  U"oă", U"uâ",
    U"uê", U"uơ", U"uy", U"ai", U"ao", U"au", U"âu",  U"ay",  U"ây",  U"eo",  U"êu", U"iu",
    U"oi", U"ôi", U"ơi", U"ui", U"ưi", U"ưu", U"iêu", U"yêu", U"uôi", U"ươi", U"ươu"};

const std::vector<std::u32string_view> VALID_FINALS_U32 = {U"c", U"ch", U"m", U"n", U"ng", U"nh",
                                                           U"p", U"t",  U"i", U"y", U"o",  U"u"};

constexpr std::array<char32_t, 4> FRONT_VOWELS = {'e', U'ê', 'i', 'y'};
constexpr std::array<char32_t, 3> FRONT_VOWELS_STRICT = {'e', U'ê', 'i'};
constexpr std::array<char32_t, 2> E_VOWELS = {'e', U'ê'};

constexpr std::array<char32_t, 4> CH_NH_NUCLEI = {'a', U'ê', 'i', 'y'};

constexpr std::array<std::u32string_view, 4> CENTERING_DIPHTHONGS_REQ_CODA = {U"iê", U"uô", U"ươ", U"yê"};
constexpr std::array<std::u32string_view, 3> CENTERING_DIPHTHONGS_NO_CODA = {U"ia", U"ua", U"ưa"};

constexpr std::array<char32_t, 3> GLIDE_O_NEXT = {'a', 'e', U'ă'};
constexpr std::array<char32_t, 11> GLIDE_U_NEXT_QU = {'a', 'e', 'i', U'â', U'ê', 'o', U'ô', U'ơ', 'u', U'ư', 'y'};
constexpr std::array<char32_t, 5> GLIDE_U_NEXT = {U'ê', 'y', U'â', U'ơ', U'ô'};

// Navigation and special keys
const char32_t KEY_UP = 0x11;
const char32_t KEY_DOWN = 0x12;
const char32_t KEY_LEFT = 0x13;
const char32_t KEY_RIGHT = 0x14;
const char32_t KEY_ENTER = 0x0D;
const char32_t KEY_TAB = 0x09;
const char32_t KEY_ESC = 0x1B;

}  // namespace constants

}  // namespace lotus_engine
