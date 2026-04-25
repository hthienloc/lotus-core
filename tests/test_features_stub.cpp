#include "lotus_core/shortcut_manager.h"
#include "lotus_core/smart_typing.h"
#include <iostream>

using namespace lotus_core;

void test_features() {
    // ShortcutManagerExpands
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");

        EngineResult res;
        bool handled = mgr.handle(' ', U"vn", res);
        if (!handled || res.action != EngineAction::TRANSFORM || res.backspace != 2) {
            std::cerr << "\033[1;31m[FAIL]\033[0m ShortcutManagerExpands failed." << std::endl;
            exit(1);
        }
    }

    // ShortcutManagerCaseSensitivity
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");

        EngineResult res;
        bool handled = mgr.handle(' ', U"Vn", res);
        if (!handled || res.action != EngineAction::TRANSFORM || res.backspace != 2 || res.chars[0] != 'V') {
            std::cerr << "\033[1;31m[FAIL]\033[0m ShortcutManagerCaseSensitivity (Vn) failed." << std::endl;
            exit(1);
        }

        handled = mgr.handle(' ', U"VN", res);
        if (!handled || res.action != EngineAction::TRANSFORM || res.backspace != 2 || res.chars[0] != 'V') {
            std::cerr << "\033[1;31m[FAIL]\033[0m ShortcutManagerCaseSensitivity (VN) failed." << std::endl;
            exit(1);
        }
    }

    // ShortcutManagerMacroModes
    {
        ShortcutManager mgr;
        mgr.add_shortcut("vn", "Việt Nam");

        EngineResult res;
        if (mgr.handle(' ', U"vn", res, MacroMode::OFF)) {
            std::cerr << "\033[1;31m[FAIL]\033[0m MacroMode::OFF failed." << std::endl; exit(1);
        }
        if (!mgr.handle(' ', U"vn", res, MacroMode::EXACT) || mgr.handle(' ', U"Vn", res, MacroMode::EXACT)) {
            std::cerr << "\033[1;31m[FAIL]\033[0m MacroMode::EXACT failed." << std::endl; exit(1);
        }
        if (!mgr.handle(' ', U"vn", res, MacroMode::FIXED) || res.chars[0] != 'V' || !mgr.handle(' ', U"Vn", res, MacroMode::FIXED) || res.chars[0] != 'V') {
            std::cerr << "\033[1;31m[FAIL]\033[0m MacroMode::FIXED failed." << std::endl; exit(1);
        }
        if (!mgr.handle(' ', U"vn", res, MacroMode::ADAPTIVE) || res.chars[0] != 'v' || !mgr.handle(' ', U"VN", res, MacroMode::ADAPTIVE) || res.chars[0] != 'V') {
            std::cerr << "\033[1;31m[FAIL]\033[0m MacroMode::ADAPTIVE failed." << std::endl; exit(1);
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
            std::cerr << "\033[1;31m[FAIL]\033[0m ShortcutManagerClear failed." << std::endl;
            exit(1);
        }
    }

    // SmartTypingDoubleSpace
    {
        char32_t key = ' ';
        EngineResult res2;
        std::u32string last_committed_text;
        bool handled = SmartTyping::handle(key, true, false, ' ', false, U"", res2, last_committed_text);
        if (!handled || res2.action != EngineAction::TRANSFORM || res2.chars[0] != '.' || res2.chars[1] != ' ') {
            std::cerr << "\033[1;31m[FAIL]\033[0m SmartTypingDoubleSpace failed." << std::endl;
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
            std::cerr << "\033[1;31m[FAIL]\033[0m SmartTypingAutoCapitalization failed." << std::endl;
            exit(1);
        }
    }

    std::cout << "  \033[1;32m[PASS]\033[0m test_features" << std::endl;
}
