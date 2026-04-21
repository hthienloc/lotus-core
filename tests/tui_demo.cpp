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

// ... (keep RawTerminal, copy_to_clipboard, key_to_name the same)

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
        std::cout << "\33[?25h";
    }
};

void copy_to_clipboard(const std::string& text) {
    FILE* pipe = popen("wl-copy 2>/dev/null || xclip -selection clipboard 2>/dev/null", "w");
    if (pipe) {
        fwrite(text.c_str(), 1, text.size(), pipe);
        pclose(pipe);
    }
}

std::string key_to_name(char32_t key) {
    if (key == ' ')
        return "' '";
    if (key == 8 || key == 127)
        return "BACKSPACE";
    if (key == 13)
        return "ENTER";
    if (key == 27)
        return "ESC";
    if (key < 128)
        return std::string("'") + (char)key + "'";
    return "U+" + (std::stringstream() << std::hex << std::uppercase << (uint32_t)key).str();
}

int main(int argc, char** argv) {
    Engine engine;
    Modifiers mods;
    std::u32string screen;
    std::stringstream debug_log;

    // Enable logging if VNDEBUG=1
    const char* vndebug = std::getenv("VNDEBUG");
    if (vndebug && std::string(vndebug) == "1") {
        set_log_callback([&debug_log](LogLevel level, const std::string& msg) {
            std::string level_str = "[DEBUG]";
            if (level == LogLevel::INFO)
                level_str = "[INFO]";
            else if (level == LogLevel::ERROR)
                level_str = "[ERROR]";
            debug_log << level_str << " " << msg << "\n";
        });
    }

    bool vni_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--vni") {
            vni_mode = true;
            engine.set_method(InputMethod::VNI);
        }
    }

    debug_log << "--- Vietnamese Input Engine Debug Log ---\n";
    debug_log << "Input Method: " << (vni_mode ? "VNI" : "Telex") << "\n";
    debug_log << "Format: [Key] -> Action: [Act], BS: [Backspace], Result: '[Chars]'\n\n";

    std::cout << "--- Vietnamese Input TUI Demo (DEBUG MODE) ---" << std::endl;
    std::cout << "Input Method: " << (vni_mode ? "VNI" : "Telex") << std::endl;
    std::cout << "Press ESC to exit. Detailed log will be copied to clipboard." << std::endl;
    std::cout << "> " << std::flush;

    {
        RawTerminal raw;
        std::cout << "\33[?25h";
        unsigned char buf[4];
        while (read(STDIN_FILENO, &buf[0], 1) == 1) {
            char32_t key = 0;

            // Basic UTF-8 Decoding
            if (buf[0] < 0x80) {
                key = buf[0];
            } else if ((buf[0] & 0xE0) == 0xC0) {
                read(STDIN_FILENO, &buf[1], 1);
                key = ((buf[0] & 0x1F) << 6) | (buf[1] & 0x3F);
            } else if ((buf[0] & 0xF0) == 0xE0) {
                read(STDIN_FILENO, &buf[1], 1);
                read(STDIN_FILENO, &buf[2], 1);
                key = ((buf[0] & 0x0F) << 12) | ((buf[1] & 0x3F) << 6) | (buf[2] & 0x3F);
            } else if ((buf[0] & 0xF8) == 0xF0) {
                read(STDIN_FILENO, &buf[1], 1);
                read(STDIN_FILENO, &buf[2], 1);
                read(STDIN_FILENO, &buf[3], 1);
                key = ((buf[0] & 0x07) << 18) | ((buf[1] & 0x3F) << 12) | ((buf[2] & 0x3F) << 6) |
                      (buf[3] & 0x3F);
            }

            if (key == 27)
                break;

            auto res = engine.process_key(key, mods);

            // Handle Backspace Pass-through
            if ((key == 8 || key == 127) && res.backspace == 0 && res.count == 0) {
                if (!screen.empty())
                    screen.pop_back();
            }

            // Record debug info
            debug_log << "Key: " << std::left << std::setw(10) << key_to_name(key)
                      << " -> Act: " << (int)res.action << ", BS: " << (int)res.backspace
                      << ", Res: '" << res.to_string() << "'\n";

            // Update screen based on engine result
            for (int i = 0; i < res.backspace; i++) {
                if (!screen.empty())
                    screen.pop_back();
            }
            for (int i = 0; i < res.count; i++) {
                screen.push_back(res.chars[i]);
            }

            std::cout << "\r> " << "\33[2K" << "> " << unicode::to_utf8(screen) << std::flush;
            // Diagnostic footer
            std::cout << "\n[SCREEN_CP: " << screen.size() << " | LAST_BS: " << (int)res.backspace
                      << "]\33[A\r" << std::flush;
        }
    }

    std::string final_log = debug_log.str();
    final_log += "\nFinal Screen Content: " + unicode::to_utf8(screen) + "\n";

    copy_to_clipboard(final_log);

    std::cout << "\n\nDetailed debug log saved to clipboard!" << std::endl;
    std::cout << "Exiting demo..." << std::endl;
    return 0;
}
