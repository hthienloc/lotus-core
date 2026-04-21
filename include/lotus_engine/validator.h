#pragma once

#include "lotus_engine/types.h"
#include <unordered_set>
#include <string>
#include <string_view>

namespace lotus_engine {

class Validator {
public:
    // Valid sets are encapsulated internally in validator.cpp

    static bool is_valid_initial(std::string_view initial);

    /**
     * @brief Performs comprehensive phonotactic validation of a syllable.
     */
    static bool is_valid(const Syllable& syllable);
};

} // namespace lotus_engine
