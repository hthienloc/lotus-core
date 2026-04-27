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

bool CompositionBuffer::append_key(char32_t key, size_t commit_threshold) {
    if (key != 0) {
        buffer.push_back(key);
    }
    return buffer.size() >= commit_threshold || buffer.size() >= StaticString::MAX_LEN;
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
            StaticString out(buffer);
            if (!out.empty())
                out.pop_back();
            return out.to_u32string();
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

    // Tracker for single-character indices
    struct CharTracker {
        std::array<size_t, StaticString::MAX_LEN> data{};
        size_t count = 0;
        void push_back(size_t val) { if (count < StaticString::MAX_LEN) data[count++] = val; }
        bool empty() const { return count == 0; }
        size_t back() const { return data[count - 1]; }
        size_t operator[](size_t i) const { return data[i]; }
        size_t size() const { return count; }
        auto begin() const { return data.begin(); }
        auto end() const { return data.begin() + count; }
    };
    
    std::array<CharTracker, 26> trackers;
    CharTracker tone_indices;
    size_t ir_len = Validator::find_longest_initial(u32, 0);

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t l = unicode::to_lower(u32[i]);
        if (l >= 'a' && l <= 'z') {
            trackers[l - 'a'].push_back(i);
        }

        if (i > 0 && i >= ir_len) {
            if (l == 's' || l == 'f' || l == 'r' || l == 'x' || l == 'j' || l == 'z' || l == '0') {
                tone_indices.push_back(i);
            }
        }
    }

    // Use StaticString to avoid heap allocations
    StaticString u32_copy(u32);
    auto* self = const_cast<CompositionBuffer*>(this);

    // Stage 1: Flexible Consonants (dd -> đ)
    if ((!key_consumed || key == 0) && trackers['d' - 'a'].size() >= 2) {
        size_t first = trackers['d' - 'a'][0], last = trackers['d' - 'a'].back();
        u32_copy[first] = (u32[first] == 'D') ? U'Đ' : U'đ';
        u32_copy[last] = 0; // Mark for stripping
        if (lk == 'd' && key != 0) {
            self->last_modifier_key = key;
            key_consumed = true;
        }
    }

    // Stage 2: Flexible Vowels (aa -> â, ee -> ê, oo -> ô)
    if (!key_consumed || key == 0) {
        struct FlexRule {
            char32_t base;
            char32_t target_l;
            char32_t target_u;
        };
        constexpr std::array<FlexRule, 3> flex_rules = {{
            {'a', U'â', U'Â'},
            {'e', U'ê', U'Ê'},
            {'o', U'ô', U'Ô'}
        }};

        for (const auto& rule : flex_rules) {
            const auto& idxs = trackers[rule.base - 'a'];
            if (idxs.size() >= 2) {
                size_t first = idxs[0];
                size_t last = idxs.back();
                u32_copy[first] = (u32[first] == unicode::to_upper(rule.base)) ? rule.target_u : rule.target_l;
                u32_copy[last] = 0; // Mark for stripping
                if (lk == rule.base && key != 0) {
                    self->last_modifier_key = key;
                    key_consumed = true;
                }
            }
        }
    }

    // Stage 3: Combined Hooks (uo, uaw, aw, ow, uw)
    if (!trackers['w' - 'a'].empty()) {
        size_t u_pos = trackers['u' - 'a'].empty() ? std::u32string::npos : trackers['u' - 'a'][0];
        size_t o_pos = trackers['o' - 'a'].empty() ? std::u32string::npos : trackers['o' - 'a'][0];
        size_t a_pos = trackers['a' - 'a'].empty() ? std::u32string::npos : trackers['a' - 'a'][0];
        
        bool has_u = (u_pos != std::u32string::npos);
        bool has_o = (o_pos != std::u32string::npos);
        bool has_a = (a_pos != std::u32string::npos);
        bool hooked = false;

        struct HookPattern {
            bool has_u, has_o, has_a;
            bool apply_u, apply_o, apply_a;
        };

        constexpr std::array<HookPattern, 7> hook_rules = {{
            {true, true, false, true, true, false},   // uo
            {true, true, true, true, true, false},    // uo (ignore a)
            {true, false, true, true, false, false},  // ua -> uaw
            {true, false, false, true, false, false}, // u
            {false, true, false, false, true, false}, // o
            {false, true, true, false, false, true},  // oa -> aw
            {false, false, true, false, false, true}  // a
        }};

        for (const auto& rule : hook_rules) {
            if (has_u == rule.has_u && has_o == rule.has_o && has_a == rule.has_a) {
                if (rule.apply_u) u32_copy[u_pos] = (u32[u_pos] == 'U') ? U'Ư' : U'ư';
                if (rule.apply_o) u32_copy[o_pos] = (u32[o_pos] == 'O') ? U'Ơ' : U'ơ';
                if (rule.apply_a) u32_copy[a_pos] = (u32[a_pos] == 'A') ? U'Ă' : U'ă';
                hooked = (rule.apply_u || rule.apply_o || rule.apply_a);
                break;
            }
        }

        if (hooked) {
            for (size_t idx : trackers['w' - 'a']) {
                if (idx > 0) u32_copy[idx] = 0; // Mark for stripping
            }
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
            if (!has_real_v && !trackers['w' - 'a'].empty()) {
                size_t first_w = trackers['w' - 'a'][0];
                u32_copy[first_w] = (u32[first_w] == 'W') ? U'Ư' : U'ư';
                key_consumed = true;
                self->last_modifier_key = key;
            }
        }
    }

    // Stage 5: Tone Marks
    if (!tone_indices.empty()) {
        std::array<bool, StaticString::MAX_LEN> is_literal_marker{};
        CharTracker potential_active_indices;

        for (size_t i = 0; i < tone_indices.size(); ++i) {
            size_t idx = tone_indices[i];
            char32_t current_key = unicode::to_lower(u32[idx]);

            if (i + 1 < tone_indices.size() &&
                current_key == unicode::to_lower(u32[tone_indices[i + 1]])) {
                is_literal_marker[idx] = true;
                u32_copy[tone_indices[i + 1]] = 0; // Mark for stripping
                i++;
            } else {
                potential_active_indices.push_back(idx);
            }
        }

        CharTracker active_tone_indices;
        for (size_t idx : potential_active_indices) {
            char32_t current_key = unicode::to_lower(u32[idx]);

            StaticString base_u32;
            for (size_t j = 0; j < idx; ++j) {
                if (u32_copy[j] != 0) {
                    bool is_tone_idx = false;
                    for (size_t t_idx : tone_indices) {
                        if (t_idx == j) {
                            is_tone_idx = true;
                            break;
                        }
                    }
                    
                    if (is_literal_marker[j] || !is_tone_idx) {
                        base_u32.push_back(u32_copy[j]);
                    }
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

            u32_copy[idx] = 0; // Mark for stripping
        }
    }

    StaticString final_u32;
    for (size_t i = 0; i < u32.size(); ++i) {
        if (u32_copy[i] != 0) {
            final_u32.push_back(u32_copy[i]);
        }
    }
    current_str = unicode::to_utf8(final_u32.to_u32string());
}

void CompositionBuffer::apply_vni_rules(std::string& current_str, char32_t key, bool& key_consumed,
                                 Tone& tone_state) const {
    const std::u32string& u32 = buffer;
    if (u32.empty())
        return;

    const std::string raw_str = unicode::to_utf8(u32);
    if (Linguistics::is_definite_english(raw_str))
        return;

    StaticString u32_copy(u32);

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
            u32_copy[i] = 0; // Mark for stripping

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

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t c = u32_copy[i];
        if (c == 0) continue;

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

    StaticString final_u32;
    for (size_t i = 0; i < u32.size(); ++i) {
        if (u32_copy[i] != 0) {
            final_u32.push_back(u32_copy[i]);
        }
    }

    current_str = unicode::to_utf8(final_u32.to_u32string());
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
        StaticString u32_curr = unicode::to_utf32_static(current_str);
        size_t first_vowel = std::u32string::npos;
        for (size_t i = 0; i < u32_curr.size(); ++i) {
            if (SyllableParser::is_vowel(u32_curr[i])) {
                first_vowel = i;
                break;
            }
        }
        StaticString prefix =
            (first_vowel == std::u32string::npos) ? u32_curr : u32_curr.substr(0, first_vowel);
        if (!prefix.empty() && !Validator::is_valid_initial(unicode::to_lower_static(prefix.view()).view(), allow_non_standard)) {
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
