#pragma once

#include "lotus_core/types.h"
#include <string>
#include <vector>
#include <optional>

namespace lotus_core {

/**
 * @brief Represents the result of a reconstruction from surrounding text.
 */
struct ReconstructResult {
    std::vector<std::u32string> history_words;
    std::u32string active_buffer;
    std::u32string last_committed_text;
    char32_t last_boundary_key = 0;
    bool at_sentence_start = true;
};

/**
 * @brief A fixed-capacity ring buffer that stores the most recently committed words.
 * Used to support cross-word backspace and English auto-restore.
 */
struct WordHistory {
    static constexpr size_t CAPACITY = 10;
    std::u32string data[CAPACITY];
    size_t head = 0;
    size_t size = 0;

    /**
     * @brief Appends a word to the history, evicting the oldest entry if over capacity.
     */
    void push(const std::u32string& word) {
        data[head] = word;
        head = (head + 1) % CAPACITY;
        if (size < CAPACITY)
            size++;
    }

    /**
     * @brief Removes and returns the most recently committed word.
     * @return The most recent word, or an empty string if the history is empty.
     */
    std::u32string pop() {
        if (size == 0)
            return U"";
        head = (head + CAPACITY - 1) % CAPACITY;
        size--;
        return data[head];
    }

    /**
     * @brief Clears the entire word history.
     */
    void clear() {
        head = 0;
        size = 0;
    }
};

/**
 * @brief Manages the historical context of typed words and sentence boundaries.
 *
 * Encapsulates the logic for detecting sentence starts, tracking recently committed
 * words, checking for English-like words, and reconstructing history from text.
 */
class ContextTracker {
public:
    ContextTracker() = default;

    /**
     * @brief Adds a finalized word to the history.
     * @param word The word to push.
     */
    void push_word(const std::u32string& word);

    /**
     * @brief Adds a boundary character to the history.
     * @param boundary The boundary key to push.
     */
    void push_boundary(char32_t boundary);

    /**
     * @brief Clears the entire word history and resets the sentence boundary state.
     */
    void clear();

    /**
     * @brief Checks if the engine is currently at the start of a sentence.
     * @return True if at the start of a sentence.
     */
    bool is_at_sentence_start() const { return at_sentence_start; }

    /**
     * @brief Explicitly sets the sentence start state.
     * @param enabled The new state.
     */
    void set_at_sentence_start(bool enabled) { at_sentence_start = enabled; }

    /**
     * @brief Checks if a character acts as a word boundary.
     * @param c The UTF-32 character.
     * @return True if it is a word boundary.
     */
    static bool is_word_boundary(char32_t c);

    /**
     * @brief Checks if a character acts as a sentence ending punctuation.
     * @param c The UTF-32 character.
     * @return True if it is a sentence ending character.
     */
    static bool is_sentence_ending(char32_t c);

    /**
     * @brief Parses surrounding text to extract history and determine active syllable context.
     * @param text The surrounding context string.
     * @param method The current input method for decomposing the active word.
     * @return ReconstructResult The extracted history, active buffer, and state variables.
     */
    ReconstructResult reconstruct(const std::string& text, InputMethod method);

    /**
     * @brief Determines if the given raw word is likely an English word.
     * @param word The raw key sequence.
     * @param is_valid_vn Whether the word is already validated as Vietnamese.
     * @return True if the word is likely English.
     */
    bool is_likely_english(const std::string& word, bool is_valid_vn) const;

    /**
     * @brief Reclaims the last committed word/boundary from history.
     * @return The last word or boundary, if any.
     */
    std::optional<std::u32string> reclaim_last_word();

private:
    WordHistory history;
    bool at_sentence_start = true;
};

} // namespace lotus_core
