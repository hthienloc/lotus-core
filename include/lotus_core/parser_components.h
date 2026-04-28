#pragma once

#include "lotus_core/types.h"

#include <string>
#include <string_view>
#include <optional>

namespace lotus_core {

struct InitialParseResult {
    SmallString initial;
    size_t consumed = 0;
};

struct GlideParseResult {
    std::optional<char32_t> glide;
    size_t consumed = 0;
};

struct NucleusParseResult {
    SmallString vowel;
    Tone tone = Tone::NONE;
    size_t consumed = 0;
};

struct CodaParseResult {
    TinyString final_c;
    size_t consumed = 0;
};

struct ReorderParseResult {
    std::optional<char32_t> glide;
    SmallString vowel;
    Tone tone = Tone::NONE;
};

class InitialParser {
public:
    static InitialParseResult parse(std::u32string_view input, bool allow_non_standard = false);
};

class GlideParser {
public:
    static GlideParseResult parse(std::u32string_view input, size_t pos, std::u32string_view initial);
};

class NucleusParser {
public:
    static NucleusParseResult parse(std::u32string_view input, size_t pos);
};

class CodaParser {
public:
    static CodaParseResult parse(std::u32string_view input, size_t pos);
};

} // namespace lotus_core
