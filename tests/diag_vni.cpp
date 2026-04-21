#include "lotus_engine/engine.h"
#include "lotus_engine/unicode.h"
#include <iostream>
#include <iomanip>

using namespace lotus_engine;

void print_bytes(const std::string& label, const std::string& s) {
    std::cout << label << ": ";
    for (unsigned char c : s) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)c << " ";
    }
    std::cout << std::endl;
}

int main() {
    Engine engine;
    engine.set_method(InputMethod::VNI);
    Modifiers mods;
    
    // a61
    engine.reset();
    engine.process_key('a', mods);
    engine.process_key('6', mods);
    auto res = engine.process_key('1', mods);
    
    std::string out = res.to_string(); // Wait, res.to_string() in tests?
    // In our test it uses a screen buffer.
    
    std::u32string screen;
    engine.reset();
    for (char c : std::string("a61")) {
        auto r = engine.process_key((char32_t)c, mods);
        for (int i = 0; i < r.backspace && !screen.empty(); i++) screen.pop_back();
        for (int i = 0; i < r.count; i++) screen.push_back(r.chars[i]);
    }
    std::string final_out = unicode::to_utf8(screen);
    print_bytes("a61 result", final_out);
    print_bytes("target    ", "\xC3\xA2\xCC\x81");
    
    return 0;
}
