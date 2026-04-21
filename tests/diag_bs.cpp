#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"
#include <iostream>
#include <iomanip>

using namespace lotus_engine;

int main() {
    Engine engine;
    Modifiers mods;
    
    // Test Telex "vậy"
    std::string keys = "vayj";
    engine.reset();
    for (char c : keys) {
        auto res = engine.process_key(c, mods);
        std::cout << "Key: " << c << " -> BS: " << (int)res.backspace << ", Res: '" << res.to_string() << "' (len: " << unicode::to_utf32(res.to_string()).size() << ")\n";
    }
    
    // Now space
    auto res_space = engine.process_key(' ', mods);
    std::cout << "Key: SPACE -> BS: " << (int)res_space.backspace << "\n";
    
    // Now backspace
    auto res_bs = engine.process_key(8, mods);
    std::cout << "Key: BACKSPACE -> BS: " << (int)res_bs.backspace << ", Action: " << (int)res_bs.action << "\n";
    
    // Now 's'
    auto res_s = engine.process_key('s', mods);
    std::cout << "Key: 's' -> BS: " << (int)res_s.backspace << ", Res: '" << res_s.to_string() << "'\n";
    
    return 0;
}
