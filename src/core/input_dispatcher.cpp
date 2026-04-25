#include "lotus_core/input_dispatcher.h"
#include "lotus_core/constants.h"
#include <algorithm>
#include <cctype>

namespace lotus_core {
using namespace lotus_core::constants;

bool InputDispatcher::is_nav_key(char32_t key, const Modifiers& mods) {
    bool is_nav = std::find(NAV_KEYS.begin(), NAV_KEYS.end(), key) != NAV_KEYS.end();
    bool is_ctrl_nav = mods.ctrl && std::find(CTRL_NAV_KEYS.begin(), CTRL_NAV_KEYS.end(), key) != CTRL_NAV_KEYS.end();
    return is_nav || is_ctrl_nav;
}

bool InputDispatcher::is_word_boundary(char32_t c) {
    return c == ' ' || c == '\r' || c == '\n' || (c < 128 && (std::ispunct(static_cast<int>(c)) || c == '\t'));
}

InputCategory InputDispatcher::categorize(char32_t key, const Modifiers& mods) {
    if (is_nav_key(key, mods)) {
        return InputCategory::NAVIGATION;
    }
    if (key == KEY_BACKSPACE || key == KEY_DELETE) {
        return InputCategory::BACKSPACE;
    }
    if (is_word_boundary(key)) {
        return InputCategory::BOUNDARY;
    }
    return InputCategory::CHARACTER;
}

} // namespace lotus_core
