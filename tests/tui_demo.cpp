/**
 * @file tui_demo.cpp
 * @brief Interactive TUI demo for the Lotus engine.
 * @author Huỳnh Thiện Lộc
 */

#include "lotus_core/engine.h"
#include "lotus_core/constants.h"
#include "lotus_core/log.h"
#include "lotus_core/unicode.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <termios.h>
#include <unistd.h>

using namespace lotus_core;

using namespace constants;

// ============================================================================
// [ Terminal Utilities ]
// ============================================================================

/**
 * @brief RAII wrapper for setting and restoring terminal raw mode.
 */
struct RawTerminal {
    struct termios orig_termios;
    RawTerminal() {
        tcgetattr(STDIN_FILENO, &orig_termios);
        struct termios raw = orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }
    ~RawTerminal() {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        std::cout << "\33[?25h\33[0m";
    }
};

/**
 * @brief Copies the provided text to the system clipboard (Wayland/X11).
 * @param text The text to copy.
 */
void copy_to_clipboard(const std::string& text) {
    FILE* pipe = popen("wl-copy 2>/dev/null || xclip -selection clipboard 2>/dev/null", "w");
    if (pipe) {
        fwrite(text.c_str(), 1, text.size(), pipe);
        pclose(pipe);
    }
}

/**
 * @brief Converts a character code to a human-readable name for logging.
 * @param key The character code.
 * @return A descriptive name of the key.
 */
std::string key_to_name(char32_t key) {
    if (key == ' ')
        return "' '";
    if (key == KEY_BACKSPACE)
        return "CTRL_BACKSPACE";
    if (key == KEY_CTRL_W)
        return "CTRL_W";
    if (key == KEY_DELETE)
        return "BACKSPACE";
    if (key == KEY_ENTER)
        return "ENTER";
    if (key == KEY_ESC)
        return "ESC";
    if (key >= KEY_F1 && key <= KEY_F12)
        return "F" + std::to_string(key - 0xF000);
    if (key < ASCII_LIMIT)
        return std::string("'") + (char)key + "'";
    return "U+" + (std::stringstream() << std::hex << std::uppercase << (uint32_t)key).str();
}

/**
 * @brief Reads a single key or escape sequence from the terminal.
 * @return The decoded char32_t key code.
 */
char32_t read_key() {
    unsigned char buf[RAW_KEY_BUFFER_SIZE];
    int n = read(STDIN_FILENO, &buf[0], 1);
    if (n <= 0)
        return 0;

    if (buf[0] == KEY_ESC) {  // Escape sequence
        // Set non-blocking to peek
        struct termios raw;
        tcgetattr(STDIN_FILENO, &raw);
        struct termios tmp = raw;
        tmp.c_cc[VMIN] = 0;
        tmp.c_cc[VTIME] = 1;
        tcsetattr(STDIN_FILENO, TCSANOW, &tmp);

        n = read(STDIN_FILENO, &buf[1], 1);
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        if (n <= 0)
            return KEY_ESC;  // Pure ESC

        if (buf[1] == 'O') {  // VT100 / Xterm style F1-F4 (\033OP - \033OS)
            if (read(STDIN_FILENO, &buf[2], 1) <= 0)
                return KEY_ESC;
            if (buf[2] == 'P')
                return KEY_F1;  // F1
            if (buf[2] == 'Q')
                return KEY_F2;  // F2
            if (buf[2] == 'R')
                return KEY_F3;  // F3
            if (buf[2] == 'S')
                return KEY_F4;  // F4
            return KEY_ESC;
        } else if (buf[1] == '[') {  // CSI sequences (\033[...] )
            std::string seq;
            unsigned char c;
            while (read(STDIN_FILENO, &c, 1) > 0) {
                if (c >= '@' && c <= '~') {  // Terminator
                    seq += (char)c;
                    break;
                }
                seq += (char)c;
                if (seq.size() > RAW_KEY_BUFFER_SIZE)
                    break;
            }

            // Xterm/Gnome/Konsole style (\033[15~, \033[1;5A etc)
            if (seq == "11~" || seq == "1~" || seq == "P")
                return KEY_F1;  // F1
            if (seq == "12~" || seq == "2~" || seq == "Q")
                return KEY_F2;  // F2
            if (seq == "13~" || seq == "3~" || seq == "R")
                return KEY_F3;  // F3
            if (seq == "14~" || seq == "4~" || seq == "S")
                return KEY_F4;  // F4
            if (seq == "15~" || seq == "5~")
                return KEY_F5;  // F5
            if (seq == "17~" || seq == "6~")
                return KEY_F6;  // F6
            if (seq == "18~")
                return KEY_F7;  // F7
            if (seq == "19~")
                return KEY_F8;  // F8
            if (seq == "20~")
                return KEY_F9;  // F9
            if (seq == "21~")
                return KEY_F10;  // F10
            if (seq == "23~")
                return KEY_F11;  // F11
            if (seq == "24~")
                return KEY_F12;  // F12

            // Simple Arrows
            if (seq == "A")
                return 0xE001;  // Up
            if (seq == "B")
                return 0xE002;  // Down
            if (seq == "C")
                return 0xE003;  // Right
            if (seq == "D")
                return 0xE004;  // Left
            if (seq == "H" || seq == "1~")
                return 0xE005;  // Home
            if (seq == "F" || seq == "4~")
                return 0xE006;  // End
            if (seq == "5~")
                return 0xE007;  // PageUp
            if (seq == "6~")
                return 0xE008;  // PageDown
        }
        return KEY_ESC;
    }

    // UTF-8 Decoding
    if (buf[0] < ASCII_LIMIT)
        return buf[0];
    if ((buf[0] & 0xE0) == 0xC0) {
        read(STDIN_FILENO, &buf[1], 1);
        return ((buf[0] & 0x1F) << 6) | (buf[1] & 0x3F);
    }
    if ((buf[0] & 0xF0) == 0xE0) {
        read(STDIN_FILENO, &buf[1], 1);
        read(STDIN_FILENO, &buf[2], 1);
        return ((buf[0] & 0x0F) << 12) | ((buf[1] & 0x3F) << 6) | (buf[2] & 0x3F);
    }
    return buf[0];
}
// ============================================================================
// [ UI Rendering ]
// ============================================================================

/**
 * @brief Returns a padded string based on visual display width.
 */
std::string pad_right(const std::string& str, size_t width) {
    size_t cur_w = unicode::display_width(str);
    if (cur_w >= width)
        return str;
    return str + std::string(width - cur_w, ' ');
}

/**
 * @brief Prints the current engine configuration and hotkey status.
 * @param engine The engine instance.
 */
void print_status(const Engine& engine) {
    std::cout << "\33[K"  // Clear line
              << "\33[1;36m[F1] \33[0mMethod: "
              << (engine.get_method() == InputMethod::VNI ? "VNI" : "Telex") << " | "
              << "\33[1;36m[F2] \33[0mTone: "
              << (engine.get_tone_style() == ToneStyle::NEW ? "New" : "Old") << " | "
              << "\33[1;36m[F3] \33[0mFreeW: ";

    switch (engine.get_free_w()) {
        case FreeWOption::OFF:
            std::cout << "Off";
            break;
        case FreeWOption::NON_START:
            std::cout << "Non-Start";
            break;
        case FreeWOption::ALWAYS:
            std::cout << "Always";
            break;
    }

    std::cout << " | \33[1;36m[F4] \33[0mHooks: " << (engine.get_std_uo() ? "ON" : "OFF")
              << std::endl;
    std::cout << "\33[K" << "\33[1;36m[F5] \33[0mDouble-Space: "
              << (engine.get_double_space_to_period() ? "ON" : "OFF") << " | "
              << "\33[1;36m[F6] \33[0mAuto-Caps: " << (engine.get_auto_capitalize() ? "ON" : "OFF") << " | "
              << "\33[1;36m[F7] \33[0mAuto-Restore: " << (engine.get_auto_restore() ? "ON" : "OFF") << " | "
              << "\33[1;36m[F8] \33[0mInitials(z,w,j,f): " << (engine.get_allow_non_standard_initials() ? "ON" : "OFF")
              << std::endl;
              
    std::cout << "\33[K" << "\33[1;36m[F9] \33[0mMacro Mode: ";
    switch (engine.get_macro_mode()) {
        case MacroMode::OFF: std::cout << "OFF"; break;
        case MacroMode::EXACT: std::cout << "EXACT"; break;
        case MacroMode::FIXED: std::cout << "FIXED"; break;
        case MacroMode::ADAPTIVE: std::cout << "ADAPTIVE"; break;
    }
    std::cout << " | \33[1;36m[F10] \33[0mBackspace Style: "
              << (engine.get_backspace_style() == BackspaceStyle::SURGICAL ? "SURGICAL" : "REVERT")
              << std::endl;
}

// ============================================================================
// [ Main Loop ]
// ============================================================================

/**
 * @brief Main entry point for the TUI demo.
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    Engine engine;
    Modifiers mods;
    std::u32string screen;
    size_t cursor_pos = 0;
    std::stringstream debug_log;

    const char* vndebug = std::getenv("LOTUSDEBUG");
    if (vndebug && std::string(vndebug) == "1") {
        set_log_callback([&debug_log](LogLevel level, const std::string& stage, double time_us, const std::string& msg) {
            (void)stage; (void)time_us;
            debug_log << "[" << (level == LogLevel::ERROR ? "ERR" : "DBG") << "] " << msg << "\n";
        });
        // Print table header
        debug_log << "| " << pad_right("Key", 8) << "| " << pad_right("Result", 13) << "| "
                  << pad_right("BS", 4) << "| " << pad_right("Current Buffer", 28) << "| "
                  << pad_right("Opts", 10) << " |" << std::endl;
        debug_log << std::string(8 + 13 + 4 + 28 + 10 + 12, '-') << std::endl;
    }

    std::cout << "\33[2J\33[H";  // Clear screen and home
    std::cout << "\33[1;35m--- \U0001FAB7  Lotus Vietnamese TUI Demo \U0001FAB7 ---\33[0m"
              << std::endl;
    std::cout << "Type to compose Vietnamese. Press ESC to exit and copy log." << std::endl;

    {
        RawTerminal raw;
        while (true) {
            std::cout << "\33[H\33[3B";  // Move to 4th line
            print_status(engine);
            
            // Render line with cursor
            std::u32string pre = screen.substr(0, cursor_pos);
            std::u32string post = screen.substr(cursor_pos);
            std::cout << "\n\r> \33[2K" << unicode::to_utf8(pre) 
                      << "\33[7m" << (post.empty() ? " " : unicode::to_utf8(post.substr(0, 1))) << "\33[0m"
                      << (post.size() > 1 ? unicode::to_utf8(post.substr(1)) : "")
                      << std::flush;

            char32_t key = read_key();
            if (key == KEY_ESC || key == 0)
                break;

            if (key == KEY_F1) {  // F1
                engine.set_method(engine.get_method() == InputMethod::TELEX ? InputMethod::VNI
                                                                            : InputMethod::TELEX);
                debug_log << "[CONFIG] Method changed to: "
                          << (engine.get_method() == InputMethod::VNI ? "VNI" : "Telex")
                          << std::endl;
                continue;
            }
            if (key == KEY_F2) {  // F2
                engine.set_tone_style(engine.get_tone_style() == ToneStyle::NEW ? ToneStyle::OLD
                                                                                : ToneStyle::NEW);
                debug_log << "[CONFIG] ToneStyle changed to: "
                          << (engine.get_tone_style() == ToneStyle::NEW ? "New" : "Old")
                          << std::endl;
                continue;
            }
            if (key == KEY_F3) {  // F3
                int next = ((int)engine.get_free_w() + 1) % 3;
                engine.set_free_w((FreeWOption)next);
                debug_log << "[CONFIG] FreeW changed to: " << (int)engine.get_free_w() << std::endl;
                continue;
            }
            if (key == KEY_F4) {  // F4
                engine.set_std_uo(!engine.get_std_uo());
                debug_log << "[CONFIG] StdUO changed to: " << (engine.get_std_uo() ? "ON" : "OFF")
                          << std::endl;
                continue;
            }
            if (key == KEY_F5) {  // F5
                engine.set_double_space_to_period(!engine.get_double_space_to_period());
                debug_log << "[CONFIG] Double-Space changed to: " << (engine.get_double_space_to_period() ? "ON" : "OFF")
                          << std::endl;
                continue;
            }
            if (key == KEY_F6) {  // F6
                engine.set_auto_capitalize(!engine.get_auto_capitalize());
                debug_log << "[CONFIG] Auto-Caps changed to: " << (engine.get_auto_capitalize() ? "ON" : "OFF")
                          << std::endl;
                continue;
            }
            if (key == KEY_F7) {  // F7
                engine.set_auto_restore(!engine.get_auto_restore());
                debug_log << "[CONFIG] Auto-Restore changed to: " << (engine.get_auto_restore() ? "ON" : "OFF")
                          << std::endl;
                continue;
            }
            if (key == KEY_F8) {  // F8
                engine.set_allow_non_standard_initials(!engine.get_allow_non_standard_initials());
                debug_log << "[CONFIG] Initials(z,w,j,f) changed to: " << (engine.get_allow_non_standard_initials() ? "ON" : "OFF")
                          << std::endl;
                continue;
            }
            if (key == KEY_F9) {  // F9
                int next = ((int)engine.get_macro_mode() + 1) % 4;
                engine.set_macro_mode((MacroMode)next);
                debug_log << "[CONFIG] Macro Mode changed to: " << (int)engine.get_macro_mode() << std::endl;
                continue;
            }
            if (key == KEY_F10) {  // F10
                engine.set_backspace_style(engine.get_backspace_style() == BackspaceStyle::SURGICAL ? BackspaceStyle::KEYSTROKE : BackspaceStyle::SURGICAL);
                debug_log << "[CONFIG] Backspace Style changed to: " << (engine.get_backspace_style() == BackspaceStyle::SURGICAL ? "SURGICAL" : "REVERT")
                          << std::endl;
                continue;
            }

            // Handle Navigation in TUI
            if (key == KEY_LEFT) {
                if (cursor_pos > 0) cursor_pos--;
                engine.reset();
                debug_log << "[NAV] Left (Cursor: " << cursor_pos << ")" << std::endl;
                continue;
            }
            if (key == KEY_RIGHT) {
                if (cursor_pos < screen.size()) cursor_pos++;
                engine.reset();
                debug_log << "[NAV] Right (Cursor: " << cursor_pos << ")" << std::endl;
                continue;
            }
            if (key == KEY_HOME) {
                cursor_pos = 0;
                engine.reset();
                debug_log << "[NAV] Home" << std::endl;
                continue;
            }
            if (key == KEY_END) {
                cursor_pos = screen.size();
                engine.reset();
                debug_log << "[NAV] End" << std::endl;
                continue;
            }

            // Handle Ctrl + Backspace or Ctrl + W (Delete whole word)
            if (key == KEY_BACKSPACE || key == KEY_CTRL_W) {
                engine.reset();
                debug_log << "[ACTION] Word deleted (Ctrl+W/BS)" << std::endl;
                while (cursor_pos > 0 && screen[cursor_pos - 1] == ' ') {
                    screen.erase(cursor_pos - 1, 1);
                    cursor_pos--;
                }
                while (cursor_pos > 0 && screen[cursor_pos - 1] != ' ') {
                    screen.erase(cursor_pos - 1, 1);
                    cursor_pos--;
                }
                continue;
            }

            auto res = engine.process_key(key, mods);

            // Manual Backspace handling if engine doesn't consume it
            if (key == KEY_DELETE && res.backspace == 0 && res.count == 0) {
                if (cursor_pos > 0) {
                    screen.erase(cursor_pos - 1, 1);
                    cursor_pos--;
                }
                continue;
            }

            std::string opts = "[";
            opts += (engine.get_method() == InputMethod::VNI ? "V" : "T");
            opts += (engine.get_tone_style() == ToneStyle::NEW ? "n" : "o");
            opts += " W" + std::to_string((int)engine.get_free_w());
            opts += (engine.get_std_uo() ? "U" : "-");
            opts += (engine.get_at_sentence_start() ? "S" : "-");
            opts += "]";

            debug_log << "| " << pad_right(key_to_name(key), 8) << "| "
                      << pad_right(res.to_string(), 13) << "| "
                      << pad_right(std::to_string((int)res.backspace), 4) << "| "
                      << pad_right(unicode::to_utf8(screen), 28) << "| " << pad_right(opts, 10)
                      << " |" << std::endl;

            // Apply engine results at cursor position
            if (res.backspace > 0) {
                screen.erase(cursor_pos - res.backspace, res.backspace);
                cursor_pos -= res.backspace;
            }
            if (res.count > 0) {
                std::u32string chars;
                for (int i = 0; i < res.count; i++) chars += res.chars[i];
                screen.insert(cursor_pos, chars);
                cursor_pos += res.count;
            }
        }
    }

    debug_log << "\n--- Final Engine Configuration ---\n"
              << "Method: " << (engine.get_method() == InputMethod::VNI ? "VNI" : "Telex") << "\n"
              << "Tone Style: " << (engine.get_tone_style() == ToneStyle::NEW ? "New" : "Old")
              << "\n"
              << "Free W: " << (int)engine.get_free_w() << "\n"
              << "Std UO: " << (engine.get_std_uo() ? "ON" : "OFF") << "\n"
              << "Double Space to Period: " << (engine.get_double_space_to_period() ? "ON" : "OFF")
              << "\n"
              << "Auto Capitalize: " << (engine.get_auto_capitalize() ? "ON" : "OFF") << "\n"
              << "Sentence Start: " << (engine.get_at_sentence_start() ? "ON" : "OFF") << "\n";

    copy_to_clipboard(debug_log.str());
    std::cout << "\n\33[1;32mDemo finished. Debug log copied to clipboard.\33[0m" << std::endl;
    return 0;
}
