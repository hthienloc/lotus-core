#include "lotus_engine/parser_components.h"
#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"

namespace lotus_engine {

/**
 * @brief Extracts the vowel nucleus sequence and identifies the tone mark if present.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the nucleus.
 */
size_t NucleusParser::parse(const std::u32string& input, size_t pos, Syllable& s) {
    size_t n = input.size();
    size_t len = 0;

    while (pos + len < n && SyllableParser::is_vowel(input[pos + len])) {
        if (s.tone == Tone::NONE) {
            Tone t = unicode::get_tone(input[pos + len]);
            if (t != Tone::NONE)
                s.tone = t;
        }
        s.vowel += unicode::strip_tone(input[pos + len]);
        len++;
    }

    return len;
}

} // namespace lotus_engine
