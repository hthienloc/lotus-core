#pragma once

#include "lotus_engine/types.h"
#include <string>
#include <map>

namespace lotus_engine {

class ShortcutManager {
public:
    void add_shortcut(const std::string& trigger, const std::string& replacement);
    bool handle(char32_t key, const std::u32string& buffer, EngineResult& result);
private:
    std::map<std::string, std::string> shortcuts;
    EngineResult _make_transformation_result(const std::u32string& final_u32, size_t prev_size) const;
};

} // namespace lotus_engine
