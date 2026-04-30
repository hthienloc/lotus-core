#include "lotus_core/types.h"
#include "lotus_core/parser.h"
#include "lotus_core/unicode.h"
#include <gtest/gtest.h>

using namespace lotus_core;

TEST(CharStateTest, FromUnicode) {
    // â
    CharState s1 = CharState::from_unicode(U'â');
    EXPECT_EQ(s1.base, U'a');
    EXPECT_EQ(s1.modifier, Modifier::HAT);
    EXPECT_EQ(s1.tone, Tone::NONE);
    EXPECT_FALSE(s1.upper);

    // Ế
    CharState s2 = CharState::from_unicode(U'Ế');
    EXPECT_EQ(s2.base, U'e');
    EXPECT_EQ(s2.modifier, Modifier::HAT);
    EXPECT_EQ(s2.tone, Tone::ACUTE);
    EXPECT_TRUE(s2.upper);

    // ơ
    CharState s3 = CharState::from_unicode(U'ơ');
    EXPECT_EQ(s3.base, U'o');
    EXPECT_EQ(s3.modifier, Modifier::HOOK);
    EXPECT_EQ(s3.tone, Tone::NONE);

    // ặ
    CharState s4 = CharState::from_unicode(U'ặ');
    EXPECT_EQ(s4.base, U'a');
    EXPECT_EQ(s4.modifier, Modifier::BREVE);
    EXPECT_EQ(s4.tone, Tone::DOT);

    // Đ
    CharState s5 = CharState::from_unicode(U'Đ');
    EXPECT_EQ(s5.base, U'd');
    EXPECT_EQ(s5.modifier, Modifier::BAR);
    EXPECT_TRUE(s5.upper);
}

TEST(CharStateTest, ToUnicode) {
    CharState s1{U'a', Modifier::HAT, Tone::NONE, false};
    EXPECT_EQ(s1.to_unicode(), U'â');

    CharState s2{U'e', Modifier::HAT, Tone::ACUTE, true};
    EXPECT_EQ(s2.to_unicode(), U'Ế');

    CharState s3{U'd', Modifier::BAR, Tone::NONE, true};
    EXPECT_EQ(s3.to_unicode(), U'Đ');
}

TEST(CharStateTest, SyllableToCharStates) {
    Syllable s1 = SyllableParser::parse(U"việt");
    CharStateArray states1 = s1.to_char_states(ToneStyle::NEW);
    EXPECT_EQ(states1.size(), 4);
    EXPECT_EQ(states1[0].to_unicode(), U'v');
    EXPECT_EQ(states1[1].to_unicode(), U'i');
    EXPECT_EQ(states1[2].to_unicode(), U'ệ');
    EXPECT_EQ(states1[3].to_unicode(), U't');

    Syllable s2 = SyllableParser::parse(U"hoà");
    CharStateArray states2 = s2.to_char_states(ToneStyle::NEW);
    EXPECT_EQ(states2.size(), 3);
    EXPECT_EQ(states2[0].to_unicode(), U'h');
    EXPECT_EQ(states2[1].to_unicode(), U'o');
    EXPECT_EQ(states2[2].to_unicode(), U'à');

    CharStateArray states2_old = s2.to_char_states(ToneStyle::OLD);
    EXPECT_EQ(states2_old.size(), 3);
    EXPECT_EQ(states2_old[0].to_unicode(), U'h');
    EXPECT_EQ(states2_old[1].to_unicode(), U'ò');
    EXPECT_EQ(states2_old[2].to_unicode(), U'a');
}
