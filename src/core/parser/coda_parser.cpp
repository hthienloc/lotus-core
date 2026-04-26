#include "lotus_core/parser_components.h"

using namespace lotus_core;

namespace lotus_core {

/**
 * @brief Extracts the remaining characters as the final coda.
 * @param input The raw input character sequence.
 * @param pos The current parsing position.
 * @return CodaParseResult The parsed coda and the number of characters consumed.
 */
CodaParseResult CodaParser::parse(std::u32string_view input, size_t pos) {
    CodaParseResult result;
    if (pos < input.size()) {
        result.final_c = input.substr(pos);
        result.consumed = result.final_c.size();
    }
    return result;
}

} // namespace lotus_core
