#pragma once

#include "lotus_engine/types.h"
#include "lotus_engine/common.h"
#include <string>
#include <map>

namespace lotus_engine {

class ShortcutManager {
public:
    void add_shortcut(const std::string& trigger, const std::string& replacement);
    void clear();
    bool handle(char32_t key, const std::u32string& buffer, EngineResult& result, MacroMode mode = MacroMode::ADAPTIVE);
private:
    std::map<std::string, std::string> shortcuts; // lowercase trigger -> replacement
    std::map<std::string, std::string> exact_shortcuts; // exact trigger -> replacement
    
    bool is_trigger_key(char32_t key) const;
    EngineResult _make_transformation_result(const std::u32string& final_u32, size_t prev_size) const;
};

} // namespace lotus_engine
