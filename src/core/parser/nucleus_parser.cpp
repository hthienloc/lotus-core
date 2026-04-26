#include "lotus_core/parser_components.h"
#include "lotus_core/parser.h"
#include "lotus_core/unicode.h"

using namespace lotus_core;

namespace lotus_core {

/**
 * @brief Extracts the vowel nucleus sequence and identifies the tone mark if present.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @return NucleusParseResult The parsed nucleus, tone, and number of characters consumed.
 */
NucleusParseResult NucleusParser::parse(std::u32string_view input, size_t pos) {
    NucleusParseResult result;
    size_t n = input.size();
    size_t len = 0;

    while (pos + len < n && SyllableParser::is_vowel(input[pos + len])) {
        if (result.tone == Tone::NONE) {
            Tone t = unicode::get_tone(input[pos + len]);
            if (t != Tone::NONE)
                result.tone = t;
        }
        result.vowel += unicode::strip_tone(input[pos + len]);
        len++;
    }

    result.consumed = len;
    return result;
}

} // namespace lotus_core
