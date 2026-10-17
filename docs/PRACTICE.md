# Pronunciation practice

The Practise screen is for anyone learning to pronounce English, at any age:
a child learning sounds, an adult preparing for an interview, someone new to the
language. They read a known sentence aloud, and the result shows, word by word,
what came through.

It coaches rather than marks. The result says how many words were clear, then
names the one word to try next ("7 of 8 words were clear. Try “thought”
again."), and every hint is phrased as what *we* heard, not what *you* got
wrong. Children and nervous learners give up when they feel graded; they keep
going when they are told what to do next.

## The problem it works against

Whisper is a sentence model, not a sound recogniser. Given "I sink so" it will
often write "I think so", because that is the likelier sentence. For dictation
that is a feature. For a learner it hides the very mistake they need to see.

Nothing in Whisper turns this off. Practice mode pushes back in three ways, and
is honest about the rest in the tooltip on the colour key under each result.

1. **Comparison with the target.** The learner's sentence is known, so every
   word Whisper wrote differently is shown, with what it heard instead.
2. **Confidence.** Whisper reports a probability for every token. A word it
   matched but was unsure of is marked **say again**, even though the text agrees.
   A "corrected" word usually lands here.
3. **Literal decoding.** The worker's `--practice` flag sets `no_context`, so
   earlier words do not steer the guess, and turns off temperature fallback, so
   Whisper does not retry until the result reads well.

## Marks

| Mark | Meaning | Hint shown |
|---|---|---|
| clear | heard as written, with confidence | none |
| heard differently | something else was heard in its place; both words are shown | We heard “sink” instead of “think” |
| not heard | in the sentence, not heard at all | We didn't hear “road” |
| say again | matched, but Whisper was below 50% sure of it | Say “lorry” again, slowly |
| extra | heard, but not in the sentence | We also heard “the” |

Words are compared after folding case, dropping apostrophes and splitting on
punctuation and hyphens, so "Don't" matches "dont" and punctuation never counts
as a mistake. The comparison is a word-level edit distance that prefers a
substitution to a separate miss and extra, because "heard *sink* for *think*"
is what a learner can act on.

## Sentences

Twenty-four built-in sentences, grouped by sound: *th*, *r and l*, *v and w*,
short and long vowels, *s, z and sh*, and word endings. **Type My Own** accepts
any text instead.

None of the built-in sentences contains a number. Whisper may write "3" for
"three", which would read as a mistake the learner did not make.

## What is kept

Nothing. A practice attempt runs with `--no-retain-audio` and
`--no-retain-transcript`: it is a drill, not something to file. The limit is
fixed at 15 seconds.

## Worker output

With `--practice`, the worker prints each word and the lowest probability of
its spoken tokens, before `FINAL`:

```text
WORDS|I:0.98|sink:0.41|so.:0.95
FINAL|I sink so.
```

The word is everything before the last colon. Punctuation tokens are kept in
the text but do not lower a word's probability.

To see it without the interface:

```bash
./build-release/eco_escrito_cli --benchmark --input speech.wav --threads 8 \
    --practice models/ggml-small.en.bin | grep WORDS
```

## Limits

- Whisper still hears words, not sounds. A wrong vowel inside a word it
  recognises with confidence will be marked right. Catching that needs a
  phoneme recogniser, which this application does not have yet.
- **More accurate** (`small.en`) hears noticeably better than **Faster**
  (`base.en`) on the same recording. Practise uses whichever is chosen on Write
  by Voice.
- English only.

The logic lives in `practice.h` and is covered by the unit tests.
