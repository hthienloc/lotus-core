/**
 * @file context_tracker.cpp
 * @brief Implementation of ContextTracker for historical state management.
 */

#include "lotus_core/context_tracker.h"
#include "lotus_core/linguistics.h"
#include "lotus_core/unicode.h"
#include "lotus_core/parser.h"
#include "lotus_core/validator.h"

#include <algorithm>
#include <cctype>

using namespace lotus_core;

namespace lotus_core {

void ContextTracker::push_word(const std::u32string& word) {
    if (!word.empty()) {
        history.push(word);
    }
}

void ContextTracker::push_boundary(char32_t boundary) {
    history.push({boundary});
}

std::optional<std::u32string> ContextTracker::reclaim_last_word() {
    std::u32string recovered = history.pop();
    if (recovered.empty()) {
        return std::nullopt;
    }
    return recovered;
}

void ContextTracker::clear() {
    history.clear();
    at_sentence_start = true;
}

bool ContextTracker::is_word_boundary(char32_t c) {
    return c == ' ' || c == '\r' || c == '\n' || (c < 128 && (ispunct((int)c) || c == '\t'));
}

bool ContextTracker::is_sentence_ending(char32_t c) {
    return c == '.' || c == '!' || c == '?' || c == '\n' || c == '\r';
}

ReconstructResult ContextTracker::reconstruct(const std::string& text, InputMethod method) {
    ReconstructResult result;
    result.at_sentence_start = true;

    if (text.empty()) {
        return result;
    }

    std::u32string full_text = unicode::to_utf32(text);
    if (!full_text.empty()) {
        auto it = std::find_if(full_text.rbegin(), full_text.rend(),
                               [](char32_t c) { return c != ' ' && c != '\t'; });
        result.at_sentence_start = (it != full_text.rend() && is_sentence_ending(*it));
    }

    std::vector<std::string> words;
    std::string current_word;
    for (char32_t c : full_text) {
        if (is_word_boundary(c)) {
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

    if (words.empty()) {
        return result;
    }

    std::string last_word = words.back();
    words.pop_back();

    for (const auto& w : words) {
        result.history_words.push_back(unicode::to_utf32(w));
        std::u32string w32 = unicode::to_utf32(w);
        if (!w32.empty()) {
            char32_t last_c = w32.back();
            result.last_boundary_key = is_word_boundary(last_c) ? last_c : 0;
        }
    }

    Syllable s = SyllableParser::parse(unicode::to_utf32(last_word));
    StaticString keys = s.to_keys(method);
    for (char32_t k : keys) {
        result.active_buffer.push_back(k);
    }
    result.last_committed_text = unicode::to_utf32(last_word);

    std::u32string last32 = unicode::to_utf32(last_word);
    if (!last32.empty()) {
        char32_t last_c = last32.back();
        bool is_boundary = is_word_boundary(last_c);
        if (is_boundary) {
            result.last_boundary_key = last_c;
            result.history_words.push_back(last32);
            result.active_buffer.clear();
            result.last_committed_text.clear();
        } else {
            result.last_boundary_key = 0;
        }
    }

    return result;
}

// Note: `is_likely_english` in `Engine` used to perform an internal `apply_telex_rules` to test
// the word against the rules. Since `apply_telex_rules` is private to `Engine`, the `Engine`
// needs to test validation directly, or `ContextTracker` just provides the pure heuristic check.
// We provide the pure heuristic check here.
// Helper to perform the full English heuristic check that was previously in Engine.
// It requires testing if the word parses as a valid Vietnamese syllable, but without
// performing a full IM transformation (which is private to Engine).
bool ContextTracker::is_likely_english(const std::string& word, bool is_valid_vn) const {
    if (Linguistics::is_on_whitelist(word))
        return true;
    if (is_valid_vn)
        return false;
    return Linguistics::is_likely_english(word);
}

} // namespace lotus_core