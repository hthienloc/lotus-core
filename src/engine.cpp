#include "lotus_engine/engine.h"

#include "lotus_engine/parser.h"
#include "lotus_engine/unicode.h"
#include "lotus_engine/validator.h"
#include "lotus_engine/linguistics.h"

#include <algorithm>
#include <cstring>

namespace lotus_engine {

std::string EngineResult::to_string() const {
    std::u32string u32;
    for (uint8_t i = 0; i < count; ++i)
        u32 += chars[i];
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

void Engine::rebuild_from_text(const std::string& text) {
    reset();
    word_history.clear();
    if (text.empty())
        return;

    std::vector<std::string> words;
    std::string current_word;
    for (char32_t c : unicode::to_utf32(text)) {
        bool is_boundary = (c == ' ' || c == '\r' || c == '\n' ||
                            (c < 128 && (ispunct((int)c) || c == '\t')));
        if (is_boundary) {
            if (!current_word.empty()) {
                words.push_back(current_word);
                current_word.clear();
            }
            words.push_back(unicode::to_utf8(std::u32string(1, c)));
        } else {
            current_word += unicode::to_utf8(std::u32string(1, c));
        }
    }
    if (!current_word.empty()) {
        words.push_back(current_word);
    }

    if (words.empty())
        return;

    // Last word becomes the active buffer
    std::string last_word = words.back();
    words.pop_back();

    for (const auto& w : words) {
        word_history.push(unicode::to_utf32(w));
        // If the word we just pushed ends in a boundary, we should set last_boundary_key
        std::u32string w32 = unicode::to_utf32(w);
        if (!w32.empty()) {
            char32_t last_c = w32.back();
            bool is_boundary = (last_c == ' ' || last_c == '\r' || last_c == '\n' ||
                                (last_c < 128 && (ispunct((int)last_c) || last_c == '\t')));
            if (is_boundary) {
                last_boundary_key = last_c;
            } else {
                last_boundary_key = 0; // Word doesn't end in boundary
            }
        }
    }

    // Process last word
    Syllable s = SyllableParser::parse(last_word);
    std::vector<char32_t> keys = s.to_keys(method);
    buffer.clear();
    for (char32_t k : keys)
        buffer.push_back(k);
    last_committed_text = unicode::to_utf32(last_word);
    // If last_word is a boundary, set it
    std::u32string last32 = unicode::to_utf32(last_word);
    if (!last32.empty()) {
        char32_t last_c = last32.back();
        bool is_boundary = (last_c == ' ' || last_c == '\r' || last_c == '\n' ||
                            (last_c < 128 && (ispunct((int)last_c) || last_c == '\t')));
        if (is_boundary) {
            last_boundary_key = last_c;
            // If it's a boundary, the buffer should be empty and the word pushed to history
            word_history.push(last32);
            buffer.clear();
            last_committed_text.clear();
        } else {
            last_boundary_key = 0;
        }
    }
}

EngineResult Engine::process_key(char32_t original_key, const Modifiers& mods) {
    char32_t key = original_key;
    if (std_uo) {
        if (key == '[')
            key = U'ư';
        else if (key == ']')
            key = U'ơ';
        else if (key == '{')
            key = U'Ư';
        else if (key == '}')
            key = U'Ơ';
    }

    if (key == 8 || key == 127) {
        if (!buffer.empty()) {
            Syllable s = SyllableParser::parse(unicode::to_utf8(last_committed_text));
            s.remove_last_char();
            std::vector<char32_t> keys = s.to_keys(method);
            buffer.clear();
            for (char32_t k : keys)
                buffer.push_back(k);
            if (buffer.empty()) {
                last_committed_text.clear();
                return make_transformation_result(U"");
            }
            return process_key(0, mods);
        } else if (last_boundary_key != 0) {
            auto recovered = word_history.pop();
            if (!recovered.empty()) {
                buffer = recovered;
                last_boundary_key = 0;
                (void)process_key(0, mods);
                EngineResult res{};
                res.action = 1;
                res.backspace = 1;
                res.count = 0;
                return res;
            }
        }
        return EngineResult{};
    }

    // 2. Word Boundary
    bool is_boundary = (key == ' ' || key == '\r' || key == '\n' ||
                        (key < 128 && (ispunct((int)key) || key == '\t')));
    if (is_boundary) {
        std::string raw_word = unicode::to_utf8(buffer);
        if (is_english_word(raw_word)) {
            std::u32string output = buffer;
            output.push_back(key);
            EngineResult res = make_transformation_result(output);
            res.action = 2;  // Restore
            reset();
            last_boundary_key = key;
            return res;
        }

        if (!buffer.empty())
            word_history.push(buffer);

        // Shortcuts
        std::string trigger_raw = unicode::to_utf8(buffer);
        std::string trigger_lower = unicode::to_lower(trigger_raw);
        if (shortcuts.count(trigger_lower)) {
            std::string replacement = shortcuts[trigger_lower];
            bool is_all_upper =
                std::all_of(trigger_raw.begin(), trigger_raw.end(),
                            [](unsigned char c) { return !isalpha(c) || isupper(c); });
            bool is_first_upper = isupper((unsigned char)trigger_raw[0]);
            if (is_all_upper) {
                std::u32string u32 = unicode::to_utf32(replacement);
                for (auto& c : u32)
                    c = unicode::to_upper(c);
                replacement = unicode::to_utf8(u32);
            } else if (is_first_upper) {
                std::u32string u32 = unicode::to_utf32(replacement);
                if (!u32.empty())
                    u32[0] = unicode::to_upper(u32[0]);
                replacement = unicode::to_utf8(u32);
            }
            std::u32string repl_u32 = unicode::to_utf32(replacement);
            repl_u32.push_back(key);
            EngineResult res = make_transformation_result(repl_u32);
            reset();
            last_boundary_key = key;
            return res;
        }

        EngineResult res{};
        res.action = 0;
        res.count = 1;
        res.chars[0] = key;
        res.backspace = 0;
        reset();
        last_boundary_key = key;
        return res;
    }

    // 3. Normal Typing
    if (key != 0)
        buffer.push_back(key);
    std::string current_str = unicode::to_utf8(buffer);
    Tone tone_state = Tone::NONE;
    bool key_consumed = (key == 0);

    if (key != 0)
        last_boundary_key = 0;

    if (method == InputMethod::TELEX) {
        apply_telex_modifiers(current_str, key, key_consumed, tone_state);
    } else {
        apply_vni_modifiers(current_str, key, key_consumed, tone_state);
    }

    if (!key_consumed)
        last_modifier_key = 0;

    current_str = unicode::normalize_nfc(current_str);
    Syllable s = SyllableParser::parse(current_str);
    s.tone = tone_state;

    std::string final_v_word = s.to_string(tone_style);
    std::string raw_word = unicode::to_utf8(buffer);

    // Stage 7: Real-time English Auto-Restore
    // Priority 1: If raw input matches English whitelist, ALWAYS restore.
    bool whitelist_match = Linguistics::is_on_whitelist(raw_word);
    
    bool is_valid_vn = Validator::is_valid(s);
    bool is_english_pattern = Linguistics::is_likely_english(raw_word);
    bool invalid_initial = !s.initial.empty() && !Validator::is_valid_initial(s.initial);
    bool malformed_syllable =
        s.initial.empty() && !buffer.empty() && !SyllableParser::is_vowel(buffer[0]);

    if (auto_restore) {
        if (whitelist_match) {
            return make_transformation_result(buffer);
        }
        if (!is_valid_vn && (is_english_pattern || invalid_initial || malformed_syllable)) {
            return make_transformation_result(buffer);
        }
    }
    return make_transformation_result(unicode::to_utf32(final_v_word));
}

void Engine::apply_telex_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                                 Tone& tone_state) {
    const std::string raw_str = unicode::to_utf8(buffer);

    // English Compatibility: If word matches English patterns or whitelist, 
    // bypass transformations.
    if (Linguistics::is_definite_english(raw_str)) {
        return;
    }

    // Leading-W Guard (Specific to Free-W logic)
    bool is_start_w = (raw_str.size() > 0 && (raw_str[0] == 'w' || raw_str[0] == 'W'));
    if (is_start_w && free_w != FreeWOption::ALWAYS) {
        return;
    }

    current_str = raw_str;

    // Stage 1: Standard Consonants (dd -> đ)
    if (current_str.find("dd") != std::string::npos) {
        if (!key_consumed && key == 'd' && last_modifier_key == 'd') {
            // Revert double-typing (ddd -> dd)
            unicode::replace_all(current_str, "ddd", "dd");
            last_modifier_key = 0;
            key_consumed = true;
        } else {
            unicode::replace_all(current_str, "dd", "đ");
            if (!key_consumed && key == 'd') {
                last_modifier_key = 'd';
                key_consumed = true;
            }
        }
    }

    // Stage 2: Standard Vowel Modifiers (aa->â, ee->ê, oo->ô)
    auto handle_v_mod = [&](const std::string& pat, const std::string& rep, char m) {
        if (current_str.find(pat) != std::string::npos) {
            if (!key_consumed && key == (char32_t)m && last_modifier_key == (char32_t)m) {
                // Revert logic (aaa -> aa)
                unicode::replace_all(current_str, pat + std::string(1, m), pat);
                last_modifier_key = 0;
                key_consumed = true;
            } else {
                unicode::replace_all(current_str, pat, rep);
                if (!key_consumed && key == (char32_t)m) {
                    last_modifier_key = (char)m;
                    key_consumed = true;
                }
            }
        }
    };
    handle_v_mod("aa", "â", 'a');
    handle_v_mod("ee", "ê", 'e');
    handle_v_mod("oo", "ô", 'o');

    // Stage 3: Explicit Combined Hooks (uw, ow, aw, etc)
    // These are standard TELEX and should always work if method is TELEX.
    if (raw_str.find('w') != std::string::npos) {
        if (current_str.find("uo") != std::string::npos)
            unicode::replace_all(current_str, "uo", "ươ");
        if (current_str.find("uaw") != std::string::npos)
            unicode::replace_all(current_str, "uaw", "ưa");
        if (current_str.find("aw") != std::string::npos)
            unicode::replace_all(current_str, "aw", "ă");
        if (current_str.find("ow") != std::string::npos)
            unicode::replace_all(current_str, "ow", "ơ");
        if (current_str.find("uw") != std::string::npos)
            unicode::replace_all(current_str, "uw", "ư");
        
        // Note: we don't set key_consumed yet, Stage 4/5/stripping will handle it.
    }

    // Stage 4: Intelligent/Free Hook (u/o/a + w -> ư/ơ/ă or standalone w -> ư)
    if (raw_str.find('w') != std::string::npos) {
        bool tx = false;
        bool has_pre = (current_str.find("ư") != std::string::npos ||
                        current_str.find("ơ") != std::string::npos ||
                        current_str.find("ă") != std::string::npos);

        bool is_start = (raw_str.size() > 0 && raw_str[0] == 'w');
        // Only allowed to hook vowels if NOT at start OR if explicitly ALWAYS.
        bool allowed_to_hook = (free_w == FreeWOption::ALWAYS) || !is_start;

        // 4.1. Intelligent Vowel Hooking
        if (allowed_to_hook && !has_pre) {
            if (current_str.find('u') != std::string::npos &&
                current_str.find('o') != std::string::npos) {
                unicode::replace_all(current_str, "u", "ư");
                unicode::replace_all(current_str, "o", "ơ");
                tx = true;
            } else if (current_str.find('u') != std::string::npos) {
                unicode::replace_all(current_str, "u", "ư");
                tx = true;
            } else if (current_str.find('o') != std::string::npos) {
                unicode::replace_all(current_str, "o", "ơ");
                tx = true;
            } else if (current_str.find('a') != std::string::npos) {
                unicode::replace_all(current_str, "a", "ă");
                tx = true;
            }
        }

        // 4.2. Standalone/Fallback W -> ư (Gated by FreeW settings)
        bool allowed_standalone = (free_w == FreeWOption::ALWAYS) || 
                                  (free_w == FreeWOption::NON_START && !is_start);
        
        if (!tx && !has_pre && allowed_standalone) {
            if (current_str == "w" || current_str.find('w') != std::string::npos) {
                unicode::replace_all(current_str, "w", "ư");
                tx = true;
            }
        }

        if (!key_consumed && key == 'w' && (tx || has_pre))
            key_consumed = true;
    }

    // Tones
    size_t init_len = 0;
    for (size_t len = 3; len >= 1; --len) {
        if (len <= raw_str.size() &&
            Validator::is_valid_initial(unicode::to_lower(raw_str.substr(0, len)))) {
            init_len = len;
            break;
        }
    }

    const std::string TONE_KEYS = "sfrxj", REMOVE_KEYS = "z0";
    for (int i = (int)raw_str.length() - 1; i >= (int)init_len; --i) {
        char k = raw_str[i];
        if (TONE_KEYS.find(k) != std::string::npos || REMOVE_KEYS.find(k) != std::string::npos) {
            static const std::map<char, Tone> tone_map = {{'s', Tone::ACUTE},
                                                          {'f', Tone::GRAVE},
                                                          {'r', Tone::HOOK},
                                                          {'x', Tone::TILDE},
                                                          {'j', Tone::DOT}};
            tone_state = tone_map.count(k) ? tone_map.at(k) : Tone::NONE;
            if (!key_consumed && (char32_t)k == key && last_modifier_key == key) {
                tone_state = Tone::NONE;
                last_modifier_key = 0;
                key_consumed = true;
                unicode::replace_all(current_str, std::string(2, (char)key),
                                     std::string(1, (char)key));
            } else if (!key_consumed && (char32_t)k == key) {
                last_modifier_key = key;
                key_consumed = true;
            }
            break;
        }
    }

    // Stripping
    bool should_strip =
        (tone_state != Tone::NONE || raw_str.find_last_of("z0") != std::string::npos ||
         (raw_str.size() > 1 && raw_str.find('w') != std::string::npos));
    if (should_strip) {
        auto is_mod_key = [&](unsigned char c) {
            if (TONE_KEYS.find(c) != std::string::npos || REMOVE_KEYS.find(c) != std::string::npos)
                return true;
            if (c == 'w') {
                // Strip 'w' only if it actually caused a transformation in Stage 3 or 4.
                // We check if current_str has hooks or if 'key' was consumed.
                bool has_hooks = (current_str.find("ư") != std::string::npos ||
                                  current_str.find("ơ") != std::string::npos ||
                                  current_str.find("ă") != std::string::npos);
                return has_hooks;
            }
            return false;
        };
        std::string stripped = "";
        std::u32string c32 = unicode::to_utf32(current_str);
        for (size_t i = 0; i < c32.size(); ++i) {
            // Protection: Never strip at index 0 if it's a tone marker (e.g. 'for' -> 'f' is kept)
            if (i == 0 && is_mod_key((unsigned char)c32[i])) {
                stripped += unicode::to_utf8(c32[i]);
                continue;
            }
            if (i >= init_len && c32[i] < 128 && is_mod_key((unsigned char)c32[i]))
                continue;
            stripped += unicode::to_utf8(c32[i]);
        }
        current_str = stripped;
    }
}

void Engine::apply_vni_modifiers(std::string& current_str, char32_t key, bool& key_consumed,
                                 Tone& tone_state) {
    const std::string raw_str = unicode::to_utf8(buffer);
    
    // English Compatibility
    if (Linguistics::is_definite_english(raw_str)) {
        return;
    }

    bool has_mod = false;
    for (int i = 0; i < (int)raw_str.length(); ++i) {
        char k = raw_str[i];
        if (k >= '1' && k <= '9') {
            has_mod = true;
            if (k >= '1' && k <= '5')
                tone_state = static_cast<Tone>(k - '0');
            if (k == '6') {
                unicode::replace_all(current_str, "a", "â");
                unicode::replace_all(current_str, "e", "ê");
                unicode::replace_all(current_str, "o", "ô");
            } else if (k == '7') {
                unicode::replace_all(current_str, "u", "ư");
                unicode::replace_all(current_str, "o", "ơ");
            } else if (k == '8')
                unicode::replace_all(current_str, "a", "ă");
            else if (k == '9')
                unicode::replace_all(current_str, "d", "đ");
            if ((size_t)i == raw_str.length() - 1 && key == (char32_t)k) {
                last_modifier_key = key;
                key_consumed = true;
            }
        } else if (k == '0') {
            tone_state = Tone::NONE;
            has_mod = true;
        }
    }
    if (has_mod)
        current_str.erase(std::remove_if(current_str.begin(), current_str.end(),
                                         [](unsigned char c) { return isdigit(c); }),
                          current_str.end());
}

EngineResult Engine::make_transformation_result(const std::u32string& final_u32) {
    EngineResult result{};
    result.action = 1;
    result.backspace = (uint8_t)last_committed_text.size();
    result.count = (uint8_t)std::min((size_t)32, final_u32.size());
    for (int i = 0; i < result.count; i++)
        result.chars[i] = final_u32[i];
    last_committed_text = final_u32;
    return result;
}

bool Engine::is_english_word(const std::string& word) const {
    if (Linguistics::is_on_whitelist(word))
        return true;

    // If it's valid Vietnamese after transformation, don't force English
    std::string transformed = word;
    Tone tone = Tone::NONE;
    bool consumed = false;
    // Temporary engine-like apply (simplified)
    if (method == InputMethod::TELEX) {
        const_cast<Engine*>(this)->apply_telex_modifiers(transformed, 0, consumed, tone);
    } else {
        const_cast<Engine*>(this)->apply_vni_modifiers(transformed, 0, consumed, tone);
    }
    Syllable s = SyllableParser::parse(transformed);
    s.tone = tone;
    if (Validator::is_valid(s))
        return false;

    return Linguistics::is_likely_english(word);
}

}  // namespace lotus_engine
