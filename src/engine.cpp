#include "lotus_engine/engine.h"
#include "lotus_engine/parser.h"
#include "lotus_engine/validator.h"
#include "lotus_engine/unicode.h"
#include <algorithm>
#include <cstring>

namespace lotus_engine {

std::string EngineResult::to_string() const {
    std::u32string u32;
    for (uint8_t i = 0; i < count; ++i) u32 += chars[i];
    return unicode::to_utf8(u32);
}

Engine::Engine() : last_modifier_key(0), last_boundary_key(0), method(InputMethod::TELEX) {}

void Engine::add_shortcut(const std::string& trigger, const std::string& replacement) {
    shortcuts[trigger] = replacement;
}

void Engine::reset() {
    buffer.clear();
    last_modifier_key = 0;
    last_boundary_key = 0;
    last_committed_text.clear();
}

EngineResult Engine::process_key(char32_t key, const Modifiers& mods) {
    // 1. Backspace Recovery
    if (key == 8 || key == 127) {
        if (!buffer.empty()) {
            buffer.pop_back();
            if (buffer.empty()) {
                // Buffer just became empty (deleted last char of current syllable)
                EngineResult res{};
                res.action = 1;
                res.backspace = (uint8_t)last_committed_text.size();
                res.count = 0;
                last_committed_text.clear();
                // Note: we don't set last_boundary_key here, it should keep its old value
                // because we are now "at" the boundary again.
                return res;
            }
            // Re-process remaining buffer to update diacritics
            return process_key(0, mods); 
        } else {
            // Buffer is empty. Can we recover previous word?
            if (last_boundary_key != 0) {
                auto recovered = word_history.pop();
                if (!recovered.empty()) {
                    buffer = recovered;
                    last_boundary_key = 0; 
                    
                    // Re-run pipeline internally to update last_committed_text and syllable state
                    // We call process_key with 0 to trigger the full transform/parse cycle
                    // but we discard its normal result and return our boundary-delete result.
                    (void)process_key(0, mods);
                    
                    EngineResult res{};
                    res.action = 1;
                    res.backspace = 1; // Delete the boundary
                    res.count = 0;
                    return res;
                }
            }
            // Truly start of input
            return EngineResult{}; 
        }
    }

    // 2. Word Boundary / English Auto-Restore
    bool is_boundary = (key == ' ' || key == '\r' || key == '\n' || (key < 128 && ispunct((int)key)));
    if (is_boundary) {
        std::string raw_word = unicode::to_utf8(buffer);
        if (is_english_word(raw_word)) {
            EngineResult res{};
            res.action = 2; // Restore
            res.backspace = (uint8_t)last_committed_text.size();
            std::u32string output = buffer;
            output.push_back(key);
            res.count = (uint8_t)std::min((size_t)32, output.size());
            for (int i = 0; i < res.count; i++) res.chars[i] = output[i];
            reset();
            last_boundary_key = key;
            return res;
        }

        if (!buffer.empty()) {
            word_history.push(buffer);
        }

        // Run Stage 7 (Shortcut)
        std::string trigger_raw = unicode::to_utf8(buffer);
        std::string trigger_lower = unicode::to_lower(trigger_raw);
        
        if (shortcuts.count(trigger_lower)) {
            std::string replacement = shortcuts[trigger_lower];
            // Apply Case Matching
            bool is_all_upper = std::all_of(trigger_raw.begin(), trigger_raw.end(), [](unsigned char c){ return !isalpha(c) || isupper(c); });
            bool is_first_upper = isupper((unsigned char)trigger_raw[0]);
            
            if (is_all_upper) {
                std::u32string u32 = unicode::to_utf32(replacement);
                for (auto& c : u32) c = unicode::to_upper(c);
                replacement = unicode::to_utf8(u32);
            } else if (is_first_upper) {
                std::u32string u32 = unicode::to_utf32(replacement);
                if (!u32.empty()) u32[0] = unicode::to_upper(u32[0]);
                replacement = unicode::to_utf8(u32);
            }

            std::u32string repl_u32 = unicode::to_utf32(replacement);
            EngineResult res{};
            res.action = 1;
            res.backspace = (uint8_t)last_committed_text.size();
            res.count = (uint8_t)std::min((size_t)32, repl_u32.size() + 1);
            for (uint8_t i = 0; i < (res.count - 1); ++i) res.chars[i] = repl_u32[i];
            res.chars[res.count - 1] = key;
            reset();
            last_boundary_key = key;
            return res;
        }

        EngineResult res{};
        res.action = 0; // Passthrough boundary
        res.count = 1;
        res.chars[0] = key;
        res.backspace = 0; // Explicitly ensure no deletion
        reset();
        last_boundary_key = key;
        return res;
    }

    // 3. Normal Typing / Pipeline
    if (key != 0) buffer.push_back(key);
    
    std::string raw_str = unicode::to_utf8(buffer);
    std::string current_str = raw_str;
    Tone tone_state = Tone::NONE;
    bool key_consumed = (key == 0); // Consider "0" (internal re-process) as already handled

    if (key != 0) {
        last_boundary_key = 0; 
    }

    if (method == InputMethod::TELEX) {
        // Stage 1: Stroke (dd -> đ)
        if (raw_str.find("dd") != std::string::npos) {
            if (key == 'd' && last_modifier_key == 'd') {
                unicode::replace_all(current_str, "ddd", "dd");
                last_modifier_key = 0;
                key_consumed = true;
            } else {
                unicode::replace_all(current_str, "dd", "đ");
                last_modifier_key = 'd';
                key_consumed = (key == 'd');
            }
        }

        // Stage 2: Basic Vowel Modifiers (Consecutive)
        if (!key_consumed && key == 'w' && current_str.find("uo") != std::string::npos) {
             unicode::replace_all(current_str, "u", "ư");
             unicode::replace_all(current_str, "o", "ơ");
             key_consumed = true;
             last_modifier_key = 'w';
        }

        if (!key_consumed && key == 'w' && current_str.find("uaw") != std::string::npos) {
             if (current_str.find("quaw") == std::string::npos) {
                 unicode::replace_all(current_str, "uaw", "ưa");
                 key_consumed = true;
                 last_modifier_key = 'w';
             }
        }

        auto check_vowel_mod_pair = [&](const std::string& pattern, const std::string& diacritic, char32_t mod_key) {
            if (!key_consumed && current_str.find(pattern) != std::string::npos) {
                if (key == mod_key && last_modifier_key == mod_key) {
                    unicode::replace_all(current_str, pattern + (char)mod_key, pattern);
                    last_modifier_key = 0;
                    key_consumed = true;
                } else {
                    unicode::replace_all(current_str, pattern, diacritic);
                    last_modifier_key = mod_key;
                    key_consumed = (key == mod_key);
                }
            } else {
                unicode::replace_all(current_str, pattern, diacritic);
            }
        };

        check_vowel_mod_pair("aa", "â", 'a');
        check_vowel_mod_pair("ee", "ê", 'e');
        check_vowel_mod_pair("oo", "ô", 'o');
        // Skip "aw" -> "ă" if "uaw" is present — handled as "ưa" by the uaw rule above
        if (raw_str.find("uaw") == std::string::npos) {
            check_vowel_mod_pair("aw", "ă", 'w');
        }
        check_vowel_mod_pair("ow", "ơ", 'w');
        check_vowel_mod_pair("uw", "ư", 'w');

        // Stage 5 (Upgraded): Free 'w' Modifier
        if (!key_consumed && (key == 'w' || raw_str.find('w') != std::string::npos)) {
            bool transformed = false;
            size_t u_pos = current_str.find('u');
            size_t o_pos = current_str.find('o');
            
            if (u_pos != std::string::npos && o_pos != std::string::npos) {
                // uo -> ươ
                unicode::replace_all(current_str, "u", "ư");
                unicode::replace_all(current_str, "o", "ơ");
                transformed = true;
            } else if (u_pos != std::string::npos) {
                current_str.replace(u_pos, 1, "ư");
                transformed = true;
            } else if (o_pos != std::string::npos) {
                current_str.replace(o_pos, 1, "ơ");
                transformed = true;
            }
            
            if (transformed) {
                key_consumed = (key == 'w');
            } else if (current_str == "w" || current_str.find('w') != std::string::npos) {
                unicode::replace_all(current_str, "w", "ư");
                key_consumed = (key == 'w');
            }
        }

        // Stage 3 & 4: Mark and Remove
        const std::string TONE_KEYS = "sfrxj";
        const std::string REMOVE_KEYS = "z0";
        
        for (int i = (int)raw_str.length() - 1; i >= 0; --i) {
            char k = raw_str[i];
            bool is_potential_mod = (TONE_KEYS.find(k) != std::string::npos || REMOVE_KEYS.find(k) != std::string::npos);
            if (i > 0 && is_potential_mod) {
                if (k == 's') tone_state = Tone::ACUTE;
                else if (k == 'f') tone_state = Tone::GRAVE;
                else if (k == 'r') tone_state = Tone::HOOK;
                else if (k == 'x') tone_state = Tone::TILDE;
                else if (k == 'j') tone_state = Tone::DOT;
                else tone_state = Tone::NONE;
                
                if (!key_consumed && (char32_t)k == key && last_modifier_key == key) {
                    tone_state = Tone::NONE;
                    last_modifier_key = 0;
                    key_consumed = true;
                    std::string pattern = ""; pattern += (char)key; pattern += (char)key;
                    std::string repl = ""; repl += (char)key;
                    unicode::replace_all(current_str, pattern, repl);
                } else if (!key_consumed && (char32_t)k == key) {
                    last_modifier_key = key;
                    key_consumed = true;
                }
                break; 
            }
        }

        auto is_mod = [&](unsigned char c) { return TONE_KEYS.find(c) != std::string::npos || REMOVE_KEYS.find(c) != std::string::npos || c == 'w'; };
        bool should_strip = (tone_state != Tone::NONE || raw_str.find_last_of("z0") != std::string::npos || (raw_str.size() > 1 && raw_str.find('w') != std::string::npos));
        if (should_strip) {
            std::string stripped = "";
            std::u32string c32 = unicode::to_utf32(current_str);
            for (size_t i = 0; i < c32.size(); ++i) {
                char32_t cp = c32[i];
                if (i > 0 && cp < 128) {
                    char ch = (char)cp;
                    if (is_mod(ch)) continue;
                }
                stripped += unicode::to_utf8(cp);
            }
            current_str = stripped;
        }
    } else if (method == InputMethod::VNI) {
        // VNI Logic: Process ALL modifiers in the raw string
        bool has_modifier = false;
        for (int i = 0; i < (int)raw_str.length(); ++i) {
            char k = raw_str[i];
            if (k >= '1' && k <= '9') {
                has_modifier = true;
                // Marks 1-5 (Last one wins)
                if (k == '1') tone_state = Tone::ACUTE;
                else if (k == '2') tone_state = Tone::GRAVE;
                else if (k == '3') tone_state = Tone::HOOK;
                else if (k == '4') tone_state = Tone::TILDE;
                else if (k == '5') tone_state = Tone::DOT;
                
                // Vowel/Consonant mods 6-9
                if (k == '6') {
                    unicode::replace_all(current_str, "a", "â");
                    unicode::replace_all(current_str, "e", "ê");
                    unicode::replace_all(current_str, "o", "ô");
                } else if (k == '7') {
                    unicode::replace_all(current_str, "u", "ư");
                    unicode::replace_all(current_str, "o", "ơ");
                } else if (k == '8') {
                    unicode::replace_all(current_str, "a", "ă");
                } else if (k == '9') {
                    unicode::replace_all(current_str, "d", "đ");
                }
                
                if (static_cast<size_t>(i) == (raw_str.length() - 1)) {
                    if (key == (char32_t)k && last_modifier_key == key) {
                        // Potential revert for VNI? Usually VNI just types the number twice
                        // for literal. GoNhanh style would be:
                        // But let's stick to base VNI for now: 11 -> tone reset or literal?
                        // Usually 11 -> literal 1. 
                    } else if (key == (char32_t)k) {
                        last_modifier_key = key;
                        key_consumed = true;
                    }
                }
            } else if (k == '0') {
                tone_state = Tone::NONE;
                has_modifier = true;
            }
        }
        
        // Strip all numbers from current_str for the parser
        if (has_modifier) {
            current_str.erase(std::remove_if(current_str.begin(), current_str.end(), [](unsigned char c) { 
                return isdigit(c); 
            }), current_str.end());
        }
    }

    if (!key_consumed) last_modifier_key = 0;

    // Stage 6: Normal
    current_str = unicode::normalize_nfc(current_str);
    Syllable s = SyllableParser::parse(current_str);
    s.tone = tone_state;
    
    std::string final_text = s.to_string(tone_style);
    std::u32string final_u32 = unicode::to_utf32(final_text);
    
    EngineResult res{};
    res.action = 1;
    res.backspace = (uint8_t)last_committed_text.size();
    res.count = (uint8_t)std::min((size_t)32, final_u32.size());
    for (int i = 0; i < res.count; i++) res.chars[i] = final_u32[i];
    
    last_committed_text = final_u32;
    return res;
}

} // namespace lotus_engine
