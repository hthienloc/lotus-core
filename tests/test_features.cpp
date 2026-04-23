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
    // V-i-ệ-t- -N-a-m- 
    // Just a quick assertion
    EXPECT_GT(res.count, 0);
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

} // namespace testing
} // namespace lotus_engine
