/**
 * @file tui_demo.cpp
 * @brief Interactive TUI demo for the Lotus engine.
 * @author Gemini CLI
 */

#include "lotus_engine/engine.h"
#include "lotus_engine/log.h"
#include "lotus_engine/unicode.h"

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

using namespace lotus_engine;

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
    if (key == 8)
        return "CTRL_BACKSPACE";
    if (key == 23)
        return "CTRL_W";
    if (key == 127)
        return "BACKSPACE";
    if (key == 13)
        return "ENTER";
    if (key == 27)
        return "ESC";
    if (key >= 0xF001 && key <= 0xF00C)
        return "F" + std::to_string(key - 0xF000);
    if (key < 128)
        return std::string("'") + (char)key + "'";
    return "U+" + (std::stringstream() << std::hex << std::uppercase << (uint32_t)key).str();
}

/**
 * @brief Reads a single key or escape sequence from the terminal.
 * @return The decoded char32_t key code.
 */
char32_t read_key() {
    unsigned char buf[8];
    int n = read(STDIN_FILENO, &buf[0], 1);
    if (n <= 0)
        return 0;

    if (buf[0] == 27) {  // Escape sequence
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
            return 27;  // Pure ESC

        if (buf[1] == 'O') {  // VT100 / Xterm style F1-F4 (\033OP - \033OS)
            if (read(STDIN_FILENO, &buf[2], 1) <= 0)
                return 27;
            if (buf[2] == 'P')
                return 0xF001;  // F1
            if (buf[2] == 'Q')
                return 0xF002;  // F2
            if (buf[2] == 'R')
                return 0xF003;  // F3
            if (buf[2] == 'S')
                return 0xF004;  // F4
            return 27;
        } else if (buf[1] == '[') {  // CSI sequences (\033[...] )
            std::string seq;
            unsigned char c;
            while (read(STDIN_FILENO, &c, 1) > 0) {
                if (c >= '@' && c <= '~') {  // Terminator
                    seq += (char)c;
                    break;
                }
                seq += (char)c;
                if (seq.size() > 8)
                    break;
            }

            // Xterm/Gnome/Konsole style (\033[15~, \033[1;5A etc)
            if (seq == "11~" || seq == "1~" || seq == "P")
                return 0xF001;  // F1
            if (seq == "12~" || seq == "2~" || seq == "Q")
                return 0xF002;  // F2
            if (seq == "13~" || seq == "3~" || seq == "R")
                return 0xF003;  // F3
            if (seq == "14~" || seq == "4~" || seq == "S")
                return 0xF004;  // F4
            if (seq == "15~" || seq == "5~")
                return 0xF005;  // F5
            if (seq == "17~" || seq == "6~")
                return 0xF006;  // F6
            if (seq == "18~")
                return 0xF007;  // F7
            if (seq == "19~")
                return 0xF008;  // F8
            if (seq == "20~")
                return 0xF009;  // F9
            if (seq == "21~")
                return 0xF00A;  // F10
            if (seq == "23~")
                return 0xF00B;  // F11
            if (seq == "24~")
                return 0xF00C;  // F12

            // Simple Arrows
            if (seq == "A")
                return 0xE001;  // Up
            if (seq == "B")
                return 0xE002;  // Down
            if (seq == "C")
                return 0xE003;  // Right
            if (seq == "D")
                return 0xE004;  // Left
        }
        return 27;
    }

    // UTF-8 Decoding
    if (buf[0] < 0x80)
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
              << "\33[1;36m[F6] \33[0mAuto-Caps: " << (engine.get_auto_capitalize() ? "ON" : "OFF")
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
    std::stringstream debug_log;

    const char* vndebug = std::getenv("LOTUSDEBUG");
    if (vndebug && std::string(vndebug) == "1") {
        set_log_callback([&debug_log](LogLevel level, const std::string& msg) {
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
            std::cout << "\n\r> \33[2K" << unicode::to_utf8(screen) << "\33[5m_\33[0m"
                      << std::flush;

            char32_t key = read_key();
            if (key == 27 || key == 0)
                break;

            if (key == 0xF001) {  // F1
                engine.set_method(engine.get_method() == InputMethod::TELEX ? InputMethod::VNI
                                                                            : InputMethod::TELEX);
                debug_log << "[CONFIG] Method changed to: "
                          << (engine.get_method() == InputMethod::VNI ? "VNI" : "Telex")
                          << std::endl;
                continue;
            }
            if (key == 0xF002) {  // F2
                engine.set_tone_style(engine.get_tone_style() == ToneStyle::NEW ? ToneStyle::OLD
                                                                                : ToneStyle::NEW);
                debug_log << "[CONFIG] ToneStyle changed to: "
                          << (engine.get_tone_style() == ToneStyle::NEW ? "New" : "Old")
                          << std::endl;
                continue;
            }
            if (key == 0xF003) {  // F3
                int next = ((int)engine.get_free_w() + 1) % 3;
                engine.set_free_w((FreeWOption)next);
                debug_log << "[CONFIG] FreeW changed to: " << (int)engine.get_free_w() << std::endl;
                continue;
            }
            if (key == 0xF004) {  // F4
                engine.set_std_uo(!engine.get_std_uo());
                debug_log << "[CONFIG] StdUO changed to: " << (engine.get_std_uo() ? "ON" : "OFF")
                          << std::endl;
                continue;
            }
            if (key == 0xF005) {  // F5
                engine.set_double_space_to_period(!engine.get_double_space_to_period());
                debug_log << "[CONFIG] DoubleSpace changed to: "
                          << (engine.get_double_space_to_period() ? "ON" : "OFF") << std::endl;
                continue;
            }
            if (key == 0xF006) {  // F6
                engine.set_auto_capitalize(!engine.get_auto_capitalize());
                debug_log << "[CONFIG] AutoCapitalize changed to: "
                          << (engine.get_auto_capitalize() ? "ON" : "OFF") << std::endl;
                continue;
            }

            // Handle Ctrl + Backspace or Ctrl + W (Delete whole word)
            if (key == 8 || key == 23) {
                engine.reset();
                debug_log << "[ACTION] Word deleted (Ctrl+W/BS)" << std::endl;
                // Delete until we hit a word boundary
                while (!screen.empty() && screen.back() == ' ')
                    screen.pop_back();
                while (!screen.empty() && screen.back() != ' ')
                    screen.pop_back();
                continue;
            }

            auto res = engine.process_key(key, mods);

            if (key == 127 && res.backspace == 0 && res.count == 0) {
                if (!screen.empty())
                    screen.pop_back();
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

            for (int i = 0; i < res.backspace; i++) {
                if (!screen.empty())
                    screen.pop_back();
            }
            for (int i = 0; i < res.count; i++) {
                screen.push_back(res.chars[i]);
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
