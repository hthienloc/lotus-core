#include "lotus_core/composition_buffer.h"
#include "lotus_core/constants.h"
#include "lotus_core/linguistics.h"
#include "lotus_core/log.h"
#include "lotus_core/parser.h"
#include "lotus_core/unicode.h"
#include "lotus_core/validator.h"

#include <algorithm>
#include <map>

using namespace lotus_core;
using namespace constants;

namespace lotus_core {

void CompositionBuffer::append_key(char32_t key) {
    if (key != 0) {
        buffer.push_back(key);
    }
}

void CompositionBuffer::clear() {
    buffer.clear();
    last_modifier_key = 0;
}

void CompositionBuffer::set_raw(const std::u32string& new_buffer) {
    buffer = new_buffer;
}

void CompositionBuffer::pop_back() {
    if (!buffer.empty()) {
        buffer.pop_back();
    }
}

void CompositionBuffer::handle_hook_key_shortcuts(char32_t& key, bool std_uo) {
    if (!std_uo)
        return;
    if (key == '[')
        key = U'ư';
    else if (key == ']')
        key = U'ơ';
    else if (key == '{')
        key = U'Ư';
    else if (key == '}')
        key = U'Ơ';
}

std::optional<std::u32string> CompositionBuffer::handle_manual_tone_escape(char32_t key) {
    if (key != 0 && key == last_modifier_key && !buffer.empty()) {
        char32_t lk = unicode::to_lower(key);
        bool revertible =
            (TELEX_MARKERS.find(static_cast<char>(lk)) != std::string_view::npos);
        if (revertible) {
            buffer.push_back(key);
            last_modifier_key = 0;
            std::u32string out = buffer;
            if (!out.empty())
                out.pop_back();
            return out;
        }
    }
    return std::nullopt;
}

void CompositionBuffer::apply_telex_rules(std::string& current_str, char32_t key, bool& key_consumed,
                                   Tone& tone_state, FreeWOption free_w) const {
    const std::u32string& u32 = buffer;
    if (u32.empty())
        return;

    const std::string raw_str = unicode::to_utf8(u32);
    if (Linguistics::is_definite_english(raw_str)) {
        current_str = raw_str;
        return;
    }

    if (unicode::to_lower(u32[0]) == 'w' && free_w != FreeWOption::ALWAYS) {
        current_str = raw_str;
        return;
    }

    char32_t lk = unicode::to_lower(key);
    std::vector<bool> to_strip(u32.size(), false);

    // Single pass to gather all candidate indices for transformations.
    std::map<char32_t, std::vector<size_t>> indices;
    std::vector<size_t> tone_indices;
    size_t ir_len = Validator::find_longest_initial(u32, 0);

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t l = unicode::to_lower(u32[i]);
        if (l == 'd' || l == 'a' || l == 'e' || l == 'o' || l == 'u' || l == 'w') {
            indices[l].push_back(i);
        }

        if (i > 0 && i >= ir_len) {
            if (l == 's' || l == 'f' || l == 'r' || l == 'x' || l == 'j' || l == 'z' || l == '0') {
                tone_indices.push_back(i);
            }
        }
    }

    std::u32string u32_copy = u32;
    auto* self = const_cast<CompositionBuffer*>(this);

    // Stage 1: Flexible Consonants (dd -> đ)
    if ((!key_consumed || key == 0) && indices['d'].size() >= 2) {
        size_t first = indices['d'][0], last = indices['d'].back();
        u32_copy[first] = (u32[first] == 'D') ? U'Đ' : U'đ';
        to_strip[last] = true;
        if (lk == 'd' && key != 0) {
            self->last_modifier_key = key;
            key_consumed = true;
        }
    }

    // Stage 2: Flexible Vowels (aa -> â, ee -> ê, oo -> ô)
    if (!key_consumed || key == 0) {
        auto try_flex = [&](char32_t base, char32_t target_l, char32_t target_u) {
            auto& idxs = indices[base];
            if (idxs.size() >= 2) {
                size_t first = idxs[0], last = idxs.back();
                u32_copy[first] = (u32[first] == unicode::to_upper(base)) ? target_u : target_l;
                to_strip[last] = true;
                if (lk == base && key != 0) {
                    self->last_modifier_key = key;
                    key_consumed = true;
                }
                return true;
            }
            return false;
        };
        try_flex('a', U'â', U'Â');
        try_flex('e', U'ê', U'Ê');
        try_flex('o', U'ô', U'Ô');
    }

    // Stage 3: Combined Hooks (uo, uaw, aw, ow, uw)
    if (!indices['w'].empty()) {
        size_t u_pos = indices['u'].empty() ? std::u32string::npos : indices['u'][0];
        size_t o_pos = indices['o'].empty() ? std::u32string::npos : indices['o'][0];
        size_t a_pos = indices['a'].empty() ? std::u32string::npos : indices['a'][0];
        bool hooked = false;

        if (u_pos != std::u32string::npos && o_pos != std::u32string::npos) {
            u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
            u32_copy[o_pos] = (u32[o_pos] == 'O') ? U'Ơ' : U'ơ';
            hooked = true;
        } else if (u_pos != std::u32string::npos && a_pos != std::u32string::npos) {
            u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
            hooked = true;
        } else {
            if (u_pos != std::u32string::npos) {
                u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
                hooked = true;
            }
            if (o_pos != std::u32string::npos) {
                if (a_pos == std::u32string::npos) {
                    u32_copy[o_pos] = (u32[o_pos] == 'O') ? U'Ơ' : U'ơ';
                    hooked = true;
                }
            }
            if (a_pos != std::u32string::npos) {
                u32_copy[a_pos] = (u32[a_pos] == 'A') ? U'Ă' : U'ă';
                hooked = true;
            }
        }

        if (hooked) {
            for (size_t idx : indices['w'])
                if (idx > 0)
                    to_strip[idx] = true;
            if (lk == 'w' && key != 0) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        }
    }

    // Stage 4: Standalone W -> ư
    if (!key_consumed && lk == 'w' && free_w != FreeWOption::OFF) {
        bool can_transform = (free_w == FreeWOption::ALWAYS) || (u32.size() > 1);
        if (can_transform) {
            bool has_real_v = false;
            for (auto c : u32)
                if (SyllableParser::is_vowel(c) && unicode::to_lower(c) != 'w')
                    has_real_v = true;
            if (!has_real_v && !indices['w'].empty()) {
                size_t first_w = indices['w'][0];
                u32_copy[first_w] = (u32[first_w] == 'W') ? U'Ư' : U'ư';
                key_consumed = true;
                self->last_modifier_key = key;
            }
        }
    }

    // Stage 5: Tone Marks
    if (!tone_indices.empty()) {
        std::vector<bool> is_literal_marker(u32.size(), false);
        std::vector<size_t> potential_active_indices;

        for (size_t i = 0; i < tone_indices.size(); ++i) {
            size_t idx = tone_indices[i];
            char32_t current_key = unicode::to_lower(u32[idx]);

            if (i + 1 < tone_indices.size() &&
                current_key == unicode::to_lower(u32[tone_indices[i + 1]])) {
                is_literal_marker[idx] = true;
                to_strip[tone_indices[i + 1]] = true;
                i++;
            } else {
                potential_active_indices.push_back(idx);
            }
        }

        std::vector<size_t> active_tone_indices;
        for (size_t idx : potential_active_indices) {
            char32_t current_key = unicode::to_lower(u32[idx]);

            std::u32string base_u32;
            for (size_t j = 0; j < idx; ++j) {
                if (!to_strip[j] &&
                    (is_literal_marker[j] || std::find(tone_indices.begin(), tone_indices.end(),
                                                       j) == tone_indices.end())) {
                    base_u32 += u32_copy[j];
                }
            }

            bool is_literal = false;
            if (!base_u32.empty() && current_key != 'z' && current_key != '0') {
                char32_t last_base = unicode::to_lower(base_u32.back());
                if (last_base == 's' || last_base == 'r' || last_base == 'x' || last_base == 'f' ||
                    last_base == 'j' || last_base == 'w') {
                    is_literal = true;
                }

                bool has_vowel = false;
                for (char32_t cp : base_u32) {
                    if (SyllableParser::is_vowel(cp)) {
                        has_vowel = true;
                        break;
                    }
                }
                if (!has_vowel)
                    is_literal = true;
            }

            if (!is_literal) {
                active_tone_indices.push_back(idx);
            }
        }

        tone_state = Tone::NONE;
        for (size_t idx : active_tone_indices) {
            char32_t marker = unicode::to_lower(u32[idx]);

            if (marker == 's')
                tone_state = Tone::ACUTE;
            else if (marker == 'f')
                tone_state = Tone::GRAVE;
            else if (marker == 'r')
                tone_state = Tone::HOOK;
            else if (marker == 'x')
                tone_state = Tone::TILDE;
            else if (marker == 'j')
                tone_state = Tone::DOT;
            else if (marker == 'z' || marker == '0')
                tone_state = Tone::NONE;

            to_strip[idx] = true;
        }
    }

    std::u32string final_u32;
    for (size_t i = 0; i < u32.size(); ++i)
        if (!to_strip[i])
            final_u32 += u32_copy[i];
    current_str = unicode::to_utf8(final_u32);
}

void CompositionBuffer::apply_vni_rules(std::string& current_str, char32_t key, bool& key_consumed,
                                 Tone& tone_state) const {
    const std::u32string& u32 = buffer;
    if (u32.empty())
        return;

    const std::string raw_str = unicode::to_utf8(u32);
    if (Linguistics::is_definite_english(raw_str))
        return;

    std::u32string u32_copy = u32;
    std::vector<bool> to_strip(u32.size(), false);

    bool has_mod = false;
    char32_t lk = unicode::to_lower(key);

    bool has_6 = false;
    bool has_7 = false;
    bool has_8 = false;
    bool has_9 = false;
    bool has_a = false;
    
    auto* self = const_cast<CompositionBuffer*>(this);

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t k = u32[i];
        if (k == 'a' || k == 'A' || k == U'ă' || k == U'Ă' || k == U'â' || k == U'Â') has_a = true;

        if (k >= '0' && k <= '9') {
            has_mod = true;
            to_strip[i] = true;

            if (k >= '1' && k <= '5') tone_state = static_cast<Tone>(k - '0');
            else if (k == '0') tone_state = Tone::NONE;
            else if (k == '6') has_6 = true;
            else if (k == '7') has_7 = true;
            else if (k == '8') has_8 = true;
            else if (k == '9') has_9 = true;

            if (k == lk && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        }
    }

    if (!has_mod) return;

    for (size_t i = 0; i < u32_copy.size(); ++i) {
        char32_t c = u32_copy[i];

        if (has_9) {
            if (c == 'd') c = U'đ';
            else if (c == 'D') c = U'Đ';
        }
        if (has_8) {
            if (c == 'a') c = U'ă';
            else if (c == 'A') c = U'Ă';
        }
        if (has_7) {
            if (c == 'u') c = U'ư';
            else if (c == 'U') c = U'Ư';
            if (!has_a) {
                if (c == 'o') c = U'ơ';
                else if (c == 'O') c = U'Ơ';
            }
        }

        if (has_6) {
            if (c == 'a' || c == U'ă') c = U'â';
            else if (c == 'A' || c == U'Ă') c = U'Â';
            else if (c == 'e') c = U'ê';
            else if (c == 'E') c = U'Ê';
            else if (c == 'o' || c == U'ơ') c = U'ô';
            else if (c == 'O' || c == U'Ơ') c = U'Ô';
        }

        u32_copy[i] = c;
    }

    std::u32string final_u32;
    for (size_t i = 0; i < u32.size(); ++i) {
        if (!to_strip[i]) {
            final_u32 += u32_copy[i];
        }
    }

    current_str = unicode::to_utf8(final_u32);
}

TransformationResult CompositionBuffer::transform(char32_t key, InputMethod method, FreeWOption free_w, ToneStyle style, bool allow_non_standard) {
    if (buffer.empty()) {
        return TransformationResult{U"", false, false, false};
    }

    std::string current_str = unicode::to_utf8(buffer);
    Tone tone_state = Tone::NONE;
    bool key_consumed = (key == 0); 
    
    if (method == InputMethod::TELEX) {
        apply_telex_rules(current_str, key, key_consumed, tone_state, free_w);
    } else {
        apply_vni_rules(current_str, key, key_consumed, tone_state);
    }

    LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "After IM: " + current_str +
                    " (Tone: " + std::to_string(static_cast<int>(tone_state)) +
                    ", Consumed: " + (key_consumed ? "Y" : "N") + ")"));

    if (!key_consumed)
        last_modifier_key = 0;

    current_str = unicode::normalize_nfc(current_str);
    Syllable s = SyllableParser::parse(unicode::to_utf32(current_str), allow_non_standard);
    if (tone_state != Tone::NONE)
        s.tone = tone_state;

    LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "Parsed: " + s.to_string(style)));

    bool has_valid_initial = true;
    if (!current_str.empty()) {
        std::u32string u32_curr = unicode::to_utf32(current_str);
        size_t first_vowel = std::u32string::npos;
        for (size_t i = 0; i < u32_curr.size(); ++i) {
            if (SyllableParser::is_vowel(u32_curr[i])) {
                first_vowel = i;
                break;
            }
        }
        std::u32string prefix =
            (first_vowel == std::u32string::npos) ? u32_curr : u32_curr.substr(0, first_vowel);
        if (!prefix.empty() && !Validator::is_valid_initial(unicode::to_lower(prefix), allow_non_standard)) {
            has_valid_initial = false;
        }
    }

    DiagnosticCode diagnostic = DiagnosticCode::SUCCESS;
    bool is_valid_vn = Validator::is_valid(s, &diagnostic, allow_non_standard);
    
    if (!has_valid_initial) {
        is_valid_vn = false;
        if (diagnostic == DiagnosticCode::SUCCESS) {
            diagnostic = DiagnosticCode::INVALID_INITIAL;
        }
    }

    std::string final_v_word = s.to_string(style);

    return TransformationResult{
        unicode::to_utf32(final_v_word),
        key_consumed,
        is_valid_vn,
        has_valid_initial,
        diagnostic
    };
}

bool CompositionBuffer::is_likely_english(const std::string& word, InputMethod method, FreeWOption free_w, bool allow_non_standard) const {
    std::string transformed = word;
    Tone tone = Tone::NONE;
    bool consumed = false;
    if (method == InputMethod::TELEX) {
        apply_telex_rules(transformed, 0, consumed, tone, free_w);
    } else {
        apply_vni_rules(transformed, 0, consumed, tone);
    }
    Syllable s = SyllableParser::parse(unicode::to_utf32(transformed), allow_non_standard);
    if (tone != Tone::NONE)
        s.tone = tone;
    bool is_valid_vn = Validator::is_valid(s, nullptr, allow_non_standard);
    
    return is_valid_vn;
}

}
