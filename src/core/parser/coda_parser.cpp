#include "lotus_core/parser_components.h"

namespace lotus_core {

/**
 * @brief Extracts the remaining characters as the final coda.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @param s OUT: The Syllable object to populate.
 * @return size_t The number of characters consumed as the coda.
 */
size_t CodaParser::parse(const std::u32string& input, size_t pos, Syllable& s) {
    if (pos < input.size()) {
        s.final_c = input.substr(pos);
        return s.final_c.size();
    }
    return 0;
}

} // namespace lotus_core
