#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace lotus_core {

/**
 * @brief Output charset encodings for legacy Vietnamese text.
 */
enum class OutputCharset {
    UNICODE = 0,
    TCVN3   = 1,
    VNI     = 2,
    VISCII  = 3
};

/**
 * @brief Encode a UTF-8 Vietnamese string to a legacy charset.
 * @param charset Target encoding.
 * @param input UTF-8 input string.
 * @return Encoded string (for TCVN3/VNI/VISCII, each char is one byte).
 */
std::string charset_encode(OutputCharset charset, const std::string& input);

/**
 * @brief Get the number of available charset encodings.
 */
int charset_get_count();

/**
 * @brief Get the name of a charset encoding by index.
 */
const char* charset_get_name(int index);

/**
 * @brief Get the OutputCharset enum value by name.
 */
OutputCharset charset_from_name(const char* name);

}  // namespace lotus_core
