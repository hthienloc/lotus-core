#include "lotus_engine/shortcut_manager.h"
#include "lotus_engine/smart_typing.h"
#include <iostream>

using namespace lotus_engine;

void test_features() {
    // ShortcutManagerExpands
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");

        EngineResult res;
        bool handled = mgr.handle(' ', U"vn", res);
        if (!handled || res.action != 1 || res.backspace != 2) {
            std::cerr << "[FAIL] ShortcutManagerExpands failed." << std::endl;
            exit(1);
        }
    }

    // ShortcutManagerCaseSensitivity
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");

        EngineResult res;
        bool handled = mgr.handle(' ', U"Vn", res);
        if (!handled || res.action != 1 || res.backspace != 2 || res.chars[0] != 'V') {
            std::cerr << "[FAIL] ShortcutManagerCaseSensitivity (Vn) failed." << std::endl;
            exit(1);
        }

        handled = mgr.handle(' ', U"VN", res);
        if (!handled || res.action != 1 || res.backspace != 2 || res.chars[0] != 'V') {
            std::cerr << "[FAIL] ShortcutManagerCaseSensitivity (VN) failed." << std::endl;
            exit(1);
        }
    }

    // ShortcutManagerMacroModes
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");

        EngineResult res;
        if (mgr.handle(' ', U"vn", res, MacroMode::OFF)) {
            std::cerr << "[FAIL] MacroMode::OFF failed." << std::endl; exit(1);
        }
        if (!mgr.handle(' ', U"vn", res, MacroMode::EXACT) || mgr.handle(' ', U"Vn", res, MacroMode::EXACT)) {
            std::cerr << "[FAIL] MacroMode::EXACT failed." << std::endl; exit(1);
        }
        if (!mgr.handle(' ', U"vn", res, MacroMode::FIXED) || res.chars[0] != 'V' || !mgr.handle(' ', U"Vn", res, MacroMode::FIXED) || res.chars[0] != 'V') {
            std::cerr << "[FAIL] MacroMode::FIXED failed." << std::endl; exit(1);
        }
        if (!mgr.handle(' ', U"vn", res, MacroMode::ADAPTIVE) || res.chars[0] != 'v' || !mgr.handle(' ', U"VN", res, MacroMode::ADAPTIVE) || res.chars[0] != 'V') {
            std::cerr << "[FAIL] MacroMode::ADAPTIVE failed." << std::endl; exit(1);
        }
    }

    // ShortcutManagerClear
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");
        mgr.clear();

        EngineResult res;
        bool handled = mgr.handle(' ', U"vn", res);
        if (handled) {
            std::cerr << "[FAIL] ShortcutManagerClear failed." << std::endl;
            exit(1);
        }
    }

    // SmartTypingDoubleSpace
    {
        char32_t key = ' ';
        EngineResult res2;
        std::u32string last_committed_text;
        bool handled = SmartTyping::handle(key, true, false, ' ', false, U"", res2, last_committed_text);
        if (!handled || res2.action != 1 || res2.chars[0] != '.' || res2.chars[1] != ' ') {
            std::cerr << "[FAIL] SmartTypingDoubleSpace failed." << std::endl;
            exit(1);
        }
    }

    // SmartTypingAutoCapitalization
    {
        char32_t key = 'a';
        EngineResult res;
        std::u32string last_committed_text;
        bool handled = SmartTyping::handle(key, false, true, 0, true, U"", res, last_committed_text);
        if (handled || key != 'A') {
            std::cerr << "[FAIL] SmartTypingAutoCapitalization failed." << std::endl;
            exit(1);
        }
    }

    std::cout << "test_features PASSED" << std::endl;
}
