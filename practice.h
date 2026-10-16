#pragma once

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include <vector>

// Pronunciation practice: a learner reads a known sentence, and what Whisper
// heard is compared with it word by word.
//
// Whisper is a sentence model, not a sound recogniser. Given a near miss it
// tends to write the word that was meant, which is exactly the fault a learner
// needs to see. Two things push back against that: the comparison shows every
// word that differs, and a word Whisper was unsure of is marked for checking
// even when the text matches. Neither proves a word was said well.

// A word as the worker reported it, with the lowest probability Whisper gave
// any of its spoken tokens.
struct HeardWord {
    std::string text;
    float probability = 1.0f;
};

enum class WordMark {
    Correct,
    // Matched the target, but Whisper was not confident it heard it.
    Unclear,
    // Something else was heard in its place.
    Substituted,
    // In the target, not heard at all.
    Missed,
    // Heard, but not in the target.
    Extra,
};

struct WordResult {
    WordMark mark = WordMark::Correct;
    // Normalised, as compared.
    std::string expected;
    std::string heard;
    // The expected word with the target's own capitals, for display.
    std::string shown;
};

// Below this a matching word is marked Unclear. Whisper reports 0.9 and above
// for words it is sure of; a word it guessed at usually lands well under half.
inline constexpr float unclear_word_probability = 0.5f;

// Whisper's tokens carry their own leading space when they start a word. Every
// other token continues the word before it. A token with no letter or digit,
// such as a comma, is kept in the text but does not lower the word's
// probability: Whisper's doubt about punctuation says nothing about speech.
inline void accumulate_token(std::vector<HeardWord>& words, const std::string& token, float probability) {
    if (token.empty()) {
        return;
    }
    const bool starts_word = words.empty() || token.front() == ' ';
    const std::size_t first = token.find_first_not_of(' ');
    if (first == std::string::npos) {
        return;
    }
    const std::string text = token.substr(first);
    const bool spoken = std::any_of(text.begin(), text.end(),
                                    [](unsigned char c) { return std::isalnum(c) != 0; });
    if (starts_word) {
        words.push_back({text, spoken ? probability : 1.0f});
        return;
    }
    words.back().text += text;
    if (spoken) {
        words.back().probability = std::min(words.back().probability, probability);
    }
}

// Lower case, apostrophes dropped, anything else that is not a letter or digit
// treated as a gap. "Don't" and "dont" compare equal; "thirty-three" is two
// words, which is also how Whisper usually writes it once punctuation goes.
// keep_case splits identically but leaves the letters alone, for display.
inline std::vector<std::string> practice_words(const std::string& text, bool keep_case = false) {
    std::vector<std::string> words;
    std::string current;
    for (const char raw : text) {
        const unsigned char c = static_cast<unsigned char>(raw);
        if (c == '\'') {
            continue;
        }
        if (std::isalnum(c) != 0 || c >= 0x80) {
            current += keep_case ? raw : static_cast<char>(std::tolower(c));
        } else if (!current.empty()) {
            words.push_back(current);
            current.clear();
        }
    }
    if (!current.empty()) {
        words.push_back(current);
    }
    return words;
}

// Splits each heard word as practice_words would, keeping its probability, so
// both sides of the comparison are tokenised the same way.
inline std::vector<HeardWord> normalised_heard(const std::vector<HeardWord>& heard) {
    std::vector<HeardWord> words;
    for (const HeardWord& word : heard) {
        for (const std::string& part : practice_words(word.text)) {
            words.push_back({part, word.probability});
        }
    }
    return words;
}

// Word-level edit distance, then a walk back through it. Ties prefer a
// substitution over a deletion and an insertion, because "you said X instead
// of Y" is more useful to a learner than two separate marks.
inline std::vector<WordResult> compare_words(const std::string& target, const std::vector<HeardWord>& heard_raw) {
    const std::vector<std::string> expected = practice_words(target);
    const std::vector<std::string> shown = practice_words(target, true);
    const std::vector<HeardWord> heard = normalised_heard(heard_raw);
    const std::size_t rows = expected.size();
    const std::size_t columns = heard.size();

    std::vector<std::vector<std::size_t>> cost(rows + 1, std::vector<std::size_t>(columns + 1, 0));
    for (std::size_t i = 0; i <= rows; ++i) {
        cost[i][0] = i;
    }
    for (std::size_t j = 0; j <= columns; ++j) {
        cost[0][j] = j;
    }
    for (std::size_t i = 1; i <= rows; ++i) {
        for (std::size_t j = 1; j <= columns; ++j) {
            const std::size_t change = expected[i - 1] == heard[j - 1].text ? 0 : 1;
            cost[i][j] = std::min({cost[i - 1][j - 1] + change, cost[i - 1][j] + 1, cost[i][j - 1] + 1});
        }
    }

    std::vector<WordResult> results;
    std::size_t i = rows;
    std::size_t j = columns;
    while (i > 0 || j > 0) {
        if (i > 0 && j > 0) {
            const bool same = expected[i - 1] == heard[j - 1].text;
            if (cost[i][j] == cost[i - 1][j - 1] + (same ? 0 : 1)) {
                WordResult result;
                result.expected = expected[i - 1];
                result.heard = heard[j - 1].text;
                result.shown = shown[i - 1];
                if (!same) {
                    result.mark = WordMark::Substituted;
                } else if (heard[j - 1].probability < unclear_word_probability) {
                    result.mark = WordMark::Unclear;
                }
                results.push_back(result);
                --i;
                --j;
                continue;
            }
        }
        if (i > 0 && (j == 0 || cost[i][j] == cost[i - 1][j] + 1)) {
            results.push_back({WordMark::Missed, expected[i - 1], std::string(), shown[i - 1]});
            --i;
        } else {
            results.push_back({WordMark::Extra, std::string(), heard[j - 1].text, std::string()});
            --j;
        }
    }
    std::reverse(results.begin(), results.end());
    return results;
}

// Words of the target that were heard as written and with confidence.
inline std::size_t correct_word_count(const std::vector<WordResult>& results) {
    return static_cast<std::size_t>(std::count_if(results.begin(), results.end(),
        [](const WordResult& result) { return result.mark == WordMark::Correct; }));
}

inline std::size_t target_word_count(const std::vector<WordResult>& results) {
    return static_cast<std::size_t>(std::count_if(results.begin(), results.end(),
        [](const WordResult& result) { return result.mark != WordMark::Extra; }));
}

// Built-in sentences, grouped by the sound they exercise. None contains a
// number: Whisper may write "3" for "three", and that would read as a mistake
// the learner did not make.
struct PracticeSentence {
    const char* sound;
    const char* text;
};

inline const std::vector<PracticeSentence>& practice_sentences() {
    static const std::vector<PracticeSentence> sentences{
        {"th", "I think both things are worth the thought."},
        {"th", "Their brother thanked them for the thick cloth."},
        {"th", "This month the weather was rather smooth."},
        {"th", "Both of them breathe through their mouths."},
        {"r and l", "The red lorry rolled along the long road."},
        {"r and l", "Laura really likes rice with lemon."},
        {"r and l", "Please collect the right letters from the library."},
        {"r and l", "The river flowed slowly past the rocks."},
        {"v and w", "We visited a very wet village in winter."},
        {"v and w", "The waiter wore a velvet vest."},
        {"v and w", "Vincent walked while Victor waited."},
        {"v and w", "Wave to the van when it arrives."},
        {"Short and long vowels", "The sheep on the ship slept in a heap."},
        {"Short and long vowels", "Fill the bowl with a full spoon of food."},
        {"Short and long vowels", "He bit the beet and hit the heat."},
        {"Short and long vowels", "Look at the pool near the pull cord."},
        {"s, z and sh", "She sells sea shells by the seashore."},
        {"s, z and sh", "The busy zoo shows its zebras on Sunday."},
        {"s, z and sh", "Sharon chose shiny shoes for the show."},
        {"s, z and sh", "His sister is shy but sure of herself."},
        {"Word endings", "She walked home and cooked dinner."},
        {"Word endings", "He wants new books and blue pens."},
        {"Word endings", "They asked, waited, and then decided."},
        {"Word endings", "The cats jumped over the boxes."},
    };
    return sentences;
}

// The sound groups, in the order they first appear.
inline std::vector<std::string> practice_sounds() {
    std::vector<std::string> sounds;
    for (const PracticeSentence& sentence : practice_sentences()) {
        if (std::find(sounds.begin(), sounds.end(), sentence.sound) == sounds.end()) {
            sounds.emplace_back(sentence.sound);
        }
    }
    return sounds;
}
