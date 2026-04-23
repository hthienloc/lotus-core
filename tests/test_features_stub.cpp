// Simple test file for features without gtest for lotus tests framework
#include "lotus_engine/shortcut_manager.h"
#include "lotus_engine/smart_typing.h"
#include <iostream>

using namespace lotus_engine;

void test_features() {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    bool handled = mgr.handle(' ', U"vn", res);
    if (!handled || res.action != 1 || res.backspace != 2) {
        std::cerr << "[FAIL] ShortcutManagerExpands failed." << std::endl;
        exit(1);
    }

    char32_t key = ' ';
    EngineResult res2;
    std::u32string last_committed_text;
    handled = SmartTyping::handle(key, true, false, ' ', false, U"", res2, last_committed_text);
    if (!handled || res2.action != 1 || res2.chars[0] != '.' || res2.chars[1] != ' ') {
        std::cerr << "[FAIL] SmartTypingDoubleSpace failed." << std::endl;
        exit(1);
    }
}
