#pragma once

#include "lotus_engine/types.h"
#include <string>

namespace lotus_engine {

class SmartTyping {
public:
    static bool handle(char32_t& key, bool double_space_to_period, bool auto_capitalize, char32_t last_boundary_key, bool at_sentence_start, const std::u32string& buffer, EngineResult& result, std::u32string& last_committed_text);
};

} // namespace lotus_engine
