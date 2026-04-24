#pragma once

#include "lotus_engine/types.h"

#include <string>

namespace lotus_engine {

class InitialParser {
public:
    static size_t parse(const std::u32string& input, Syllable& s, bool allow_non_standard = false);
};

class GlideParser {
public:
    static size_t parse(const std::u32string& input, size_t pos, Syllable& s);
};

class NucleusParser {
public:
    static size_t parse(const std::u32string& input, size_t pos, Syllable& s);
};

class CodaParser {
public:
    static size_t parse(const std::u32string& input, size_t pos, Syllable& s);
};

} // namespace lotus_engine
