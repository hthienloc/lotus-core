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
    return buffer.size() >= commit_threshold || buffer.size() >= StaticString::MAX_LEN_CONST;
}

void CompositionBuffer::clear() {
    buffer.clear();
    last_modifier_key = 0;
}

void CompositionBuffer::set_raw(std::u32string_view new_buffer) {
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

std::optional<StaticString> CompositionBuffer::handle_manual_tone_escape(char32_t key) {
    if (key != 0 && key == last_modifier_key && !buffer.empty()) {
        char32_t lk = unicode::to_lower(key);
        bool revertible =
            (TELEX_MARKERS.find(static_cast<char>(lk)) != std::string_view::npos);
        if (revertible) {
            buffer.push_back(key);
            last_modifier_key = 0;
            StaticString out = buffer;
            if (!out.empty())
                out.pop_back();
            return out;
        }
    }
    return std::nullopt;
}

void CompositionBuffer::apply_telex_rules(StaticString& current_str, char32_t key, bool& key_consumed,
                                   Tone& tone_state, FreeWOption free_w, bool tone_less, bool mark_less) const {
    const StaticString& u32 = buffer;
    if (u32.empty())
        return;

    if (Linguistics::is_definite_english(unicode::to_utf8(u32.view()))) {
        current_str = u32;
        return;
    }

    if (unicode::to_lower(u32[0]) == 'w' && free_w != FreeWOption::ALWAYS) {
        current_str = u32;
        return;
    }

    char32_t lk = unicode::to_lower(key);

    // Tracker for single-character indices
    struct CharTracker {
        std::array<size_t, StaticString::MAX_LEN_CONST> data{};
        size_t count = 0;
        void push_back(size_t val) { if (count < StaticString::MAX_LEN_CONST) data[count++] = val; }
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
    if (!mark_less && (!key_consumed || key == 0) && trackers['d' - 'a'].size() >= 2) {
        size_t first = trackers['d' - 'a'][0], last = trackers['d' - 'a'].back();
        u32_copy[first] = (u32[first] == 'D') ? U'Đ' : U'đ';
        u32_copy[last] = 0; // Mark for stripping
        if (lk == 'd' && key != 0) {
            self->last_modifier_key = key;
            key_consumed = true;
        }
    }

    // Stage 2: Flexible Vowels (aa -> â, ee -> ê, oo -> ô)
    if (!mark_less && (!key_consumed || key == 0)) {
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
    if (!mark_less && !trackers['w' - 'a'].empty()) {
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
    if (!mark_less && !key_consumed && lk == 'w' && free_w != FreeWOption::OFF) {
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
    if (!tone_less && !tone_indices.empty()) {
        std::array<bool, StaticString::MAX_LEN_CONST> is_literal_marker{};
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
    current_str = final_u32;
}

void CompositionBuffer::apply_vni_rules(StaticString& current_str, char32_t key, bool& key_consumed,
                                 Tone& tone_state, bool tone_less, bool mark_less) const {
    const StaticString& u32 = buffer;
    if (u32.empty())
        return;

    if (Linguistics::is_definite_english(unicode::to_utf8(u32.view()))) {
        current_str = u32;
        return;
    }

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

        bool is_tone = (k >= '1' && k <= '5') || k == '0';
        bool is_mark = (k >= '6' && k <= '9');

        if ((is_tone && !tone_less) || (is_mark && !mark_less)) {
            has_mod = true;
            u32_copy[i] = 0; // Mark for stripping

            if (!tone_less && k >= '1' && k <= '5') tone_state = static_cast<Tone>(k - '0');
            else if (!tone_less && k == '0') tone_state = Tone::NONE;
            else if (!mark_less && k == '6') has_6 = true;
            else if (!mark_less && k == '7') has_7 = true;
            else if (!mark_less && k == '8') has_8 = true;
            else if (!mark_less && k == '9') has_9 = true;

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

    current_str = final_u32;
}

void CompositionBuffer::apply_viqr_rules(StaticString& current_str, char32_t key, bool& key_consumed,
                                  Tone& tone_state, bool tone_less, bool mark_less) const {
    const StaticString& u32 = buffer;
    if (u32.empty())
        return;

    if (Linguistics::is_definite_english(unicode::to_utf8(u32.view()))) {
        current_str = u32;
        return;
    }

    StaticString u32_copy(u32);
    auto* self = const_cast<CompositionBuffer*>(this);

    bool has_mod = false;
    bool has_circumflex = false;
    bool has_plus = false;
    bool has_lparen = false;
    bool has_rparen = false;
    bool has_dd = false;

    struct CharTracker {
        std::array<size_t, StaticString::MAX_LEN_CONST> data{};
        size_t count = 0;
        void push_back(size_t val) { if (count < StaticString::MAX_LEN_CONST) data[count++] = val; }
        bool empty() const { return count == 0; }
        size_t back() const { return data[count - 1]; }
        size_t operator[](size_t i) const { return data[i]; }
        size_t size() const { return count; }
        auto begin() const { return data.begin(); }
        auto end() const { return data.begin() + count; }
    };

    std::array<CharTracker, 26> trackers;

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t k = u32[i];
        char32_t l = unicode::to_lower(k);
        if (l >= 'a' && l <= 'z') {
            trackers[l - 'a'].push_back(i);
        }
    }

    if (!mark_less && trackers['d' - 'a'].size() >= 2) {
        size_t first = trackers['d' - 'a'][0];
        size_t last = trackers['d' - 'a'].back();
        if (last == u32.size() - 1) {
            has_dd = true;
            u32_copy[first] = (u32[first] == 'D') ? U'Đ' : U'đ';
            u32_copy[last] = 0;
            if (key == 'd' && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        }
    }

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t k = u32[i];
        if (!mark_less && k == '^') {
            has_mod = true;
            has_circumflex = true;
            u32_copy[i] = 0;
            if (key == '^' && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        } else if (!mark_less && k == '+') {
            has_mod = true;
            has_plus = true;
            u32_copy[i] = 0;
            if (key == '+' && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        } else if (!mark_less && k == '(') {
            has_mod = true;
            has_lparen = true;
            u32_copy[i] = 0;
            if (key == '(' && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        } else if (!mark_less && k == ')') {
            has_mod = true;
            has_rparen = true;
            u32_copy[i] = 0;
            if (key == ')' && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        } else if (!tone_less && (k == '\'' || k == '`' || k == '?' || k == '~' || k == '.' || k == '-')) {
            has_mod = true;
            u32_copy[i] = 0;
            if (key == k && !key_consumed) {
                self->last_modifier_key = key;
                key_consumed = true;
            }
        }
    }

    if (!has_mod && !has_dd) return;

    for (size_t i = 0; i < u32.size(); ++i) {
        char32_t c = u32_copy[i];
        if (c == 0) continue;

        if (has_lparen) {
            if (c == 'u') c = U'ư';
            else if (c == 'U') c = U'Ư';
        }
        if (has_rparen) {
            if (c == 'o') c = U'ơ';
            else if (c == 'O') c = U'Ơ';
        }
        if (has_plus) {
            if (c == 'a') c = U'ă';
            else if (c == 'A') c = U'Ă';
        }
        if (has_circumflex) {
            if (c == 'a' || c == U'ă') c = U'â';
            else if (c == 'A' || c == U'Ă') c = U'Â';
            else if (c == 'e') c = U'ê';
            else if (c == 'E') c = U'Ê';
            else if (c == 'o' || c == U'ơ') c = U'ô';
            else if (c == 'O' || c == U'Ơ') c = U'Ô';
        }

        u32_copy[i] = c;
    }

    if (!tone_less) {
        for (size_t i = 0; i < u32.size(); ++i) {
            char32_t k = u32[i];
            if (k == '\'') tone_state = Tone::ACUTE;
            else if (k == '`') tone_state = Tone::GRAVE;
            else if (k == '?') tone_state = Tone::HOOK;
            else if (k == '~') tone_state = Tone::TILDE;
            else if (k == '.') tone_state = Tone::DOT;
            else if (k == '-') tone_state = Tone::NONE;
        }
    }

    StaticString final_u32;
    for (size_t i = 0; i < u32.size(); ++i) {
        if (u32_copy[i] != 0) {
            final_u32.push_back(u32_copy[i]);
        }
    }

    current_str = final_u32;
}

TransformationResult CompositionBuffer::transform(char32_t key, InputMethod method, FreeWOption free_w, ToneStyle style, bool allow_non_standard, bool tone_less, bool mark_less) {
    if (buffer.empty()) {
        return TransformationResult{StaticString(std::u32string(U"")), false, false, false};
    }

    StaticString current_str = buffer;
    Tone tone_state = Tone::NONE;
    bool key_consumed = (key == 0);
    
    switch (method) {
        case InputMethod::TELEX:
            apply_telex_rules(current_str, key, key_consumed, tone_state, free_w, tone_less, mark_less);
            break;
        case InputMethod::VNI:
            apply_vni_rules(current_str, key, key_consumed, tone_state, tone_less, mark_less);
            break;
        case InputMethod::TELEX_VNI:
            apply_telex_rules(current_str, key, key_consumed, tone_state, free_w, tone_less, mark_less);
            if (!key_consumed && key != 0) {
                apply_vni_rules(current_str, key, key_consumed, tone_state, tone_less, mark_less);
            }
            break;
        case InputMethod::VIQR:
            apply_viqr_rules(current_str, key, key_consumed, tone_state, tone_less, mark_less);
            break;
        case InputMethod::TELEX_VNI_VIQR:
            apply_telex_rules(current_str, key, key_consumed, tone_state, free_w, tone_less, mark_less);
            if (!key_consumed && key != 0) {
                apply_vni_rules(current_str, key, key_consumed, tone_state, tone_less, mark_less);
            }
            if (!key_consumed && key != 0) {
                apply_viqr_rules(current_str, key, key_consumed, tone_state, tone_less, mark_less);
            }
            break;
    }

    LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "After IM: " + unicode::to_utf8(current_str.view()) +
                    " (Tone: " + std::to_string(static_cast<int>(tone_state)) +
                    ", Consumed: " + (key_consumed ? "Y" : "N") + ")"));

    if (!key_consumed)
        last_modifier_key = 0;

    current_str = unicode::normalize_nfc_static(current_str.view());
    Syllable s = SyllableParser::parse(current_str.view(), allow_non_standard);
    if (tone_state != Tone::NONE)
        s.tone = tone_state;

    LOTUS_LOG_DEBUG(format_log_message("PIPELINE", "Parsed: " + s.to_string(style)));

    bool has_valid_initial = true;
    if (!current_str.empty()) {
        size_t first_vowel = std::u32string::npos;
        for (size_t i = 0; i < current_str.size(); ++i) {
            if (SyllableParser::is_vowel(current_str[i])) {
                first_vowel = i;
                break;
            }
        }
        StaticString prefix =
            (first_vowel == std::u32string::npos) ? current_str : current_str.substr(0, first_vowel);
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

    StaticString final_v_word = unicode::to_utf32_static(s.to_string(style));

    return TransformationResult{
        final_v_word,
        key_consumed,
        is_valid_vn,
        has_valid_initial,
        diagnostic
    };
}

bool CompositionBuffer::is_likely_english(std::u32string_view word, InputMethod method, FreeWOption free_w, bool allow_non_standard, bool tone_less, bool mark_less) const {
    StaticString transformed(word);
    Tone tone = Tone::NONE;
    bool consumed = false;
    switch (method) {
        case InputMethod::TELEX:
            apply_telex_rules(transformed, 0, consumed, tone, free_w, tone_less, mark_less);
            break;
        case InputMethod::VNI:
            apply_vni_rules(transformed, 0, consumed, tone, tone_less, mark_less);
            break;
        case InputMethod::TELEX_VNI:
            apply_telex_rules(transformed, 0, consumed, tone, free_w, tone_less, mark_less);
            apply_vni_rules(transformed, 0, consumed, tone, tone_less, mark_less);
            break;
        case InputMethod::VIQR:
            apply_viqr_rules(transformed, 0, consumed, tone, tone_less, mark_less);
            break;
        case InputMethod::TELEX_VNI_VIQR:
            apply_telex_rules(transformed, 0, consumed, tone, free_w, tone_less, mark_less);
            apply_vni_rules(transformed, 0, consumed, tone, tone_less, mark_less);
            apply_viqr_rules(transformed, 0, consumed, tone, tone_less, mark_less);
            break;
    }
    Syllable s = SyllableParser::parse(transformed.view(), allow_non_standard);
    if (tone != Tone::NONE)
        s.tone = tone;
    bool is_valid_vn = Validator::is_valid(s, nullptr, allow_non_standard);
    
    return is_valid_vn;
}

}
