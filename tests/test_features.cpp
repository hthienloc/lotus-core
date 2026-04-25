#include "lotus_core/shortcut_manager.h"
#include "lotus_core/smart_typing.h"
#include <gtest/gtest.h>

namespace lotus_core {
namespace testing {

TEST(FeatureTests, ShortcutManagerExpands) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    bool handled = mgr.handle(' ', U"vn", res);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, EngineAction::TRANSFORM);
    EXPECT_EQ(res.backspace, 2);
    EXPECT_GT(res.count, 0);
}

TEST(FeatureTests, ShortcutManagerCaseSensitivity) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    bool handled = mgr.handle(' ', U"Vn", res);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, EngineAction::TRANSFORM);
    EXPECT_EQ(res.backspace, 2);
    // Should be capitalized
    // The first char in U"Việt Nam" is 'V'
    EXPECT_EQ(res.chars[0], 'V');

    handled = mgr.handle(' ', U"VN", res);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, EngineAction::TRANSFORM);
    EXPECT_EQ(res.backspace, 2);
    // All caps -> VIỆT NAM (first char 'V')
    EXPECT_EQ(res.chars[0], 'V');
}

TEST(FeatureTests, ShortcutManagerMacroModes) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    
    // OFF Mode
    EXPECT_FALSE(mgr.handle(' ', U"vn", res, MacroMode::OFF));
    EXPECT_FALSE(mgr.handle(' ', U"Vn", res, MacroMode::OFF));
    
    // EXACT Mode
    EXPECT_TRUE(mgr.handle(' ', U"vn", res, MacroMode::EXACT));
    EXPECT_FALSE(mgr.handle(' ', U"Vn", res, MacroMode::EXACT));
    
    // FIXED Mode
    EXPECT_TRUE(mgr.handle(' ', U"vn", res, MacroMode::FIXED));
    EXPECT_EQ(res.chars[0], 'V'); // Việt Nam
    EXPECT_TRUE(mgr.handle(' ', U"Vn", res, MacroMode::FIXED));
    EXPECT_EQ(res.chars[0], 'V'); // Việt Nam
    EXPECT_TRUE(mgr.handle(' ', U"vn", res, MacroMode::FIXED));
    EXPECT_EQ(res.chars[1], 'i'); // Việt Nam
    
    // ADAPTIVE Mode
    EXPECT_TRUE(mgr.handle(' ', U"vn", res, MacroMode::ADAPTIVE));
    EXPECT_EQ(res.chars[0], 'v'); // việt nam
    EXPECT_TRUE(mgr.handle(' ', U"VN", res, MacroMode::ADAPTIVE));
    EXPECT_EQ(res.chars[0], 'V'); // VIỆT NAM
    
    // Proper Case Logic testing in Adaptive Mode
    EXPECT_TRUE(mgr.handle(' ', U"Vn", res, MacroMode::ADAPTIVE));
    EXPECT_EQ(res.chars[0], 'V'); // V
    EXPECT_EQ(res.chars[1], 'i'); // i
    EXPECT_EQ(res.chars[5], 'N'); // N in Nam
}

TEST(FeatureTests, ShortcutManagerPunctuationTriggers) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    EXPECT_TRUE(mgr.handle(',', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], ',');
    EXPECT_TRUE(mgr.handle('.', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], '.');
    EXPECT_TRUE(mgr.handle('!', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], '!');
    EXPECT_TRUE(mgr.handle('?', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], '?');
    EXPECT_TRUE(mgr.handle(':', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], ':');
    EXPECT_TRUE(mgr.handle(';', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], ';');
    EXPECT_TRUE(mgr.handle('\n', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], '\n');
    EXPECT_TRUE(mgr.handle('\r', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], '\r');
    EXPECT_TRUE(mgr.handle('\t', U"vn", res));
    EXPECT_EQ(res.chars[res.count - 1], '\t');
    
    // Non-trigger keys
    EXPECT_FALSE(mgr.handle('a', U"vn", res));
    EXPECT_FALSE(mgr.handle('1', U"vn", res));
}

TEST(FeatureTests, ShortcutManagerClear) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");
    mgr.clear();

    EngineResult res;
    bool handled = mgr.handle(' ', U"vn", res);
    EXPECT_FALSE(handled);
}

TEST(FeatureTests, SmartTypingDoubleSpace) {
    char32_t key = ' ';
    EngineResult res;
    std::u32string last_committed_text;
    bool handled = SmartTyping::handle(key, true, false, ' ', false, U"", res, last_committed_text);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, EngineAction::TRANSFORM);
    EXPECT_EQ(res.chars[0], '.');
    EXPECT_EQ(res.chars[1], ' ');
}

TEST(FeatureTests, SmartTypingAutoCapitalization) {
    char32_t key = 'a';
    EngineResult res;
    std::u32string last_committed_text;
    bool handled = SmartTyping::handle(key, false, true, 0, true, U"", res, last_committed_text);
    // Auto-capitalization just mutates the key, returns false for handled
    EXPECT_FALSE(handled);
    EXPECT_EQ(key, 'A');
}

} // namespace testing
} // namespace lotus_core
