#include "lotus_engine/shortcut_manager.h"
#include "lotus_engine/smart_typing.h"
#include <gtest/gtest.h>

namespace lotus_engine {
namespace testing {

TEST(FeatureTests, ShortcutManagerExpands) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    bool handled = mgr.handle(' ', U"vn", res);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, 1);
    EXPECT_EQ(res.backspace, 2);
    EXPECT_GT(res.count, 0);
}

TEST(FeatureTests, ShortcutManagerCaseSensitivity) {
    ShortcutManager mgr;
    mgr.add_shortcut("vn", "Việt Nam");

    EngineResult res;
    bool handled = mgr.handle(' ', U"Vn", res);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, 1);
    EXPECT_EQ(res.backspace, 2);
    // Should be capitalized
    // The first char in U"Việt Nam" is 'V'
    EXPECT_EQ(res.chars[0], 'V');

    handled = mgr.handle(' ', U"VN", res);
    EXPECT_TRUE(handled);
    EXPECT_EQ(res.action, 1);
    EXPECT_EQ(res.backspace, 2);
    // All caps -> VIỆT NAM (first char 'V')
    EXPECT_EQ(res.chars[0], 'V');
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
    EXPECT_EQ(res.action, 1);
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
} // namespace lotus_engine
