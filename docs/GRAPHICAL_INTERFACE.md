# Graphical interface

The Qt6 interface is built as `eco_escrito`. The recorder and Whisper pipeline
live in `eco_escrito_cli`; the GUI drives that worker with `QProcess` and
never loads Whisper itself, so the event loop stays free.

## What it is for

**Eco-escrito helps you speak clearly, and keeps what you say.** The name says
the same thing: *eco* (echo) and *escrito* (written), so your voice, echoed in
writing.

It is built for anyone, of any age and anywhere: a child practising English
sounds, an older person writing a letter without typing, a student taking notes,
or someone whose words must not go online. So the interface is organised around
what a person wants to do, not around the files the program writes.

| A person wants to... | Screen |
|---|---|
| say something and see it written | **Write by Voice** |
| be understood when they speak | **Practise** |
| find what they said, read it, or hear it again | **My Words** |

Under all three is one promise, said on the welcome page and in the bar: it
works without the internet, and nothing leaves the computer.

## Screens

A welcome page and three screens, chosen from the bar across the top in the
order people reach for them. The screen you are on is highlighted in the bar.

| Screen | Holds | Reached by |
|---|---|---|
| **Welcome** | the icon, one line, two ways in | opens here; the name in the bar, or `Ctrl+0` |
| **Write by Voice** | three quiet settings, the clock, one button, your words | `Ctrl+1`, or **Write by Voice** on the welcome page |
| **Practise** | a sentence to read, one button, the result word by word | `Ctrl+2`, or **Practise** on the welcome page |
| **My Words** | everything you have said: its text and its recording together | `Ctrl+3` |

The bar is navigation and nothing else. Pressing **Write by Voice** goes to that
screen; it does not start listening. Starting is done there, with the settings
already in view.

## Plain words

Every label is written so that a ten-year-old, a seventy-year-old, or someone
reading English as a second language can follow it, which also makes it easier
to translate later. Technical names stay out of sight, in tooltips and the
footer:

| Instead of | It says |
|---|---|
| Transcripts, Speeches | **My Words** |
| Speed: base.en / Accuracy: small.en | **Faster** / **More accurate** |
| 15 seconds | **Up to 15 sec** |
| Keep audio files | **Keep the recording** |
| Start / Stop Recording | **Start Speaking** / **Stop** |
| Loading model..., Transcribing 1 of 3 | **Getting ready...**, **Writing it down: part 1 of 3** |
| Whisper heard: "..." | **We heard: "..."** |

Errors say what to do next ("Check that a microphone is plugged in"), never
only what failed.

## Welcome

The welcome page has no tab of its own; the name in the bar leads back to it.
It is deliberately almost empty, on white:

- the application icon
- one line: *Speak clearly, and keep what you say.*
- two buttons, each with its name and one line on what it does for you:
  **Write by Voice** (*Speak, and see it written*), the primary action, and
  **Practise** (*Hear which words need work*)
- one honest footnote: *Works without the internet. English only, for now.*
  Practice and dictation use English models, and saying so up front is kinder
  than letting someone find out by trying.

A new feature gets a tab in the bar. It earns a button here only if it becomes
one of the main reasons to open the application.

## One design for every screen

Every screen follows the welcome page:

- **White, and a centred column.** Each screen is capped (Practise 760 px,
  Write by Voice 860 px, My Words 1120 px), so a full-screen window does not
  stretch a line across 1920 px.
- **The content is the hero.** The sentence on Practise, the clock on Write by
  Voice, and your words on My Words.
- **One filled button per screen,** the pill in the accent blue: **Start
  Speaking** on both speaking screens, **Play** on My Words. While you speak it
  turns red and reads **Stop**, or **Stop and Check** on Practise.
- **Everything else is a quiet text button:** blue for actions, red for deleting.
  None of them has a border or a fill until it is hovered.
- **No titles or explanations on the page.** The tab already says where you are.
  What a control does is in its tooltip, and what the application is doing is
  in the footer.
- **Grey fills instead of bordered cards:** your words while you speak, the
  practice result, and your own practice sentence.

Navigation closes while you are speaking or while your words are being written
down. The controls for something still running must not be hidden behind
another screen.

## Where the actions live

Each screen owns what belongs to it, so nothing is hidden in a menu away from
the thing it affects:

| Action | Where |
|---|---|
| Choose, type, and read a practice sentence | Practise |
| Start, stop, cancel | the screen that started it |
| Copy, save your words as they are written | above them, on Write by Voice |
| Play, copy, save, or delete something you said | beside it, on My Words |
| Delete everything | under the list on My Words, which is what it empties |
| Privacy, About, Quit | `Help` |

`Help` is the only entry in the bar that opens a menu; the rest switch screens.

## Look

One light theme, applied to the whole application so dialogs match the window.
The Fusion style draws the same on Linux and Windows, which is what lets one
style sheet serve both. The palette is fixed rather than taken from the
desktop: the colours assume a light background, and a dark system palette would
put pale text on white.

- **Colours:** close to the macOS system palette, so the application reads as a
  calm Mac utility on any desktop: `#ffffff` pages, `#f5f5f7` fills, `#1d1d1f`
  text, `#6e6e73` secondary text, `#0066cc` accent, `#d70015` destructive,
  `#e5e5ea` hairlines.
- **Contrast:** every colour used for text reaches at least 4.5:1 against
  white, the level small text needs to stay readable for older eyes and on a
  cheap screen in sunlight. That is why the accent is `#0066cc` (5.6:1) rather
  than Apple's brighter `#007aff` (4.0:1), the red is `#d70015` rather than
  `#ff3b30`, and the grey of a word not heard in practice is `#6e6e73`. Only
  disabled controls are paler.
- **Size:** no text below 13 px, and no text button shorter than 32 px, for
  unsteady hands and touchscreens.
- **Dates and times** follow the computer's region: `Today, 2:32 PM` in the
  United States, `Today, 14:32` in most other places. Month and day names come
  from the region too.
- **Never colour alone.** The practice key is in words, the state dot has a
  tooltip and a spoken name, and every control has an accessible name.
- **Header:** the name (which also leads to the welcome page), the three
  screens, an **Offline** badge, and Help. The window will not narrow past the
  width the bar needs, so no label is ever cut short.
- **Footer:** what the screen is doing on the left, and one small outlined
  **Star us on GitHub** on the right that warms to yellow on hover. The
  version, who made it, and the GitHub and Instagram links are in
  **Help > About**. Links open in the browser only when clicked; see
  [PRIVACY.md](PRIVACY.md). The title bar carries the name alone.
- **Dropdowns:** rounded, with a drawn chevron shipped in normal and `@2x`
  sizes, so they stay crisp on high-density screens. The chevron is a PNG
  because an SVG would need the Qt SVG plugin, which the package does not
  depend on.
- **Lists:** a sidebar divided from its content by a hairline. The selected
  row is filled in the accent colour.

## Write by Voice

```text
   [Faster v] [Default microphone v] [Up to 1 min v]  [x] Keep the recording

                           00:23 / 01:00
                            ( ■  Stop )
                         * Listening. Speak now.
                            ----------
                                                          Copy  Save
+-------------------------------------------------------------------+
|  Your words will appear here.                                     |
+-------------------------------------------------------------------+
```

**Settings need no labels.** Each value says what it is (`Faster`,
`Up to 1 min`, the microphone's name), and each list is sized to its contents.
The model's real name is in the tooltip and the footer.

**One button.** It reads **Start Speaking**, then **Stop** (in red) while you
speak, then **Cancel** while the model loads or your words are being written
down. `Ctrl+R` and `Esc` do the same.

**Context lives in the footer.** The model, input device and output path are
one line, showing only what is actually known.

The progress bar is hidden unless something is running: an empty bar on an idle
window suggests stalled work. The live-text note under it appears only when the
live text has fallen a second or more behind you.

## My Words

Everything you have said, newest first, in one list. A session's text and its
recording are shown together, because that is how a person remembers it: "what
I said on Tuesday", not a transcript in one place and a WAV file in another.
Each row is named for when it was said, the way a person would say it:

```text
Today, 2:32 PM
Yesterday, 6:11 PM
Fri 9 Oct, 10:10 AM
```

That name and nothing else. What it says is one click away; repeating a fragment
of it on every row is noise. The file name on disk is in each row's tooltip.
The newest is selected and shown on arrival.

Selecting a row shows, on the right:

- its name and one line about it: `0:07 long`, `0:07 long, no text`, or
  `Text only. The recording was not kept.`
- a compact player (**Play**, the position, the time, **Stop**) when there is a
  recording, and nothing at all when there is not
- the text, or a short note when there is none

**What plays is the raw capture.** Each recording produces three files: the raw
audio, a noise-reduced copy, and the speech extracted from it. Only the
extracted speech is sent to Whisper. Playback deliberately gives back the raw
recording, because that is what was actually said; the processed versions exist
for transcription, not for listening. Playback stops when you leave the screen.

**Delete** asks which part to lose, because the text and the recording are
separate things: **Delete the Text...** keeps the recording, and **Delete the
Recording...** keeps the text (the recording's two processed copies go with
it). Each confirms first, naming the date. When one part is deleted, the session
stays, showing what is left; when nothing is left, it leaves the list and the
next one is shown. **Delete Everything...**, under the list, removes every
recording and all of their text.

Right-clicking a row offers **Copy**, **Save a Copy**, which audio files survive
for that session (or `not kept`), and the two deletes.

Nothing on this screen can start a recording, because none of the recording
controls are present. **My Words cannot be opened while you are speaking or
while your words are being written down**, and the recording shortcuts are
inert while it shows.

Reading is confined the same way writing is: a path resolving outside the
application's own directory, or one that is a symbolic link, is refused rather
than followed. See [FILE_STORAGE.md](FILE_STORAGE.md).

Audio is played through the vendored miniaudio, the same library used to
capture, so the application needs no media framework and the package still
depends only on `qt6-qtbase-gui`.

## Keyboard

Every screen's shortcut appears in its tooltip in the top bar, so it can be
found rather than memorised. The numbers follow the order of the bar.

| Keys | Action |
|---|---|
| `Ctrl+R` | Start speaking, or stop |
| `Esc` | Cancel whatever the worker is doing |
| `Ctrl+S` | Save your words to a file |
| `Ctrl+Shift+C` | Copy all of your words |
| `Ctrl+0` | Go to the welcome page |
| `Ctrl+1` | Go to Write by Voice |
| `Ctrl+2` | Go to Practise |
| `Ctrl+3` | Go to My Words |
| `Up` / `Down` | Move through My Words |
| `Ctrl+Q` | Quit |

`Ctrl+R` is one action rather than two, so the same key both starts and stops.
On Practise it starts a practice attempt; anywhere else it goes to Write by
Voice and starts there.
Plain `Ctrl+C` is left to the text for copying a selection, which is why
copying all of it takes the shifted form.

## State at a glance

A coloured dot beside the status line carries the current state: grey when idle,
red while recording, amber while working, green when finished, and red again on
a failure. Colour alone is never the only signal; the dot also carries a tooltip
and an accessible name naming the state, and the status line says the same thing
in words.

Disabled controls explain themselves. A greyed-out model selector says it cannot
be changed until the recording finishes; a greyed-out Save says it becomes
available once a transcription has finished. A control that is unavailable
without saying why is a dead end.

## Build and run

```bash
sudo dnf install qt6-qtbase-devel
cmake -S . -B build-release
cmake --build build-release --parallel
./build-release/eco_escrito
```

## States

Every control's enabled state is a function of one enum and nothing else:

```text
Ready → LoadingModel → Recording → Stopping → Processing → Completed
                                                         ↘ Error
```

| State | Meaning | Start | Stop | Model, limit, mic | Save, copy |
|---|---|:--:|:--:|:--:|:--:|
| `Ready` | nothing running | yes | no | yes | no |
| `LoadingModel` | worker starting, model being validated | no | no | no | no |
| `Recording` | capturing, partials arriving | no | yes | no | no |
| `Stopping` | stop requested, capture winding down | no | no | no | no |
| `Processing` | VAD and final transcription running | no | no | no | no |
| `Completed` | transcript written | yes | no | yes | yes |
| `Error` | reported failure | yes | no | yes | if text arrived |

`apply_state` is the only function that calls `setEnabled`. Previously
enablement was set in four places that could disagree.

### Transitions are driven by the worker

The worker announces its progress, so the interface reflects what is actually
happening rather than what was requested:

| Worker line | Transition |
|---|---|
| `READY\|` | `LoadingModel` → `Recording`, and the clock starts |
| `PROCESSING\|` | `Stopping` → `Processing` |
| `SAVED\|TRANSCRIPT\|...` | → `Completed` |
| `ERROR\|...` | → `Error` |

`READY|` is emitted at every prompt, including after a completed transcription.
Only a transition out of `LoadingModel` starts a recording, so the later one
cannot begin a phantom second recording.

## Two recordings at once

`start_recording` returns immediately unless the state is `Ready`, `Completed`
or `Error`. The button is disabled in every other state as well, so a double
click, a stray shortcut, or a timer cannot launch a second worker.

## Model and device changes

The model, duration and microphone selectors are disabled from `LoadingModel`
until the worker finishes. Changing them mid-run would not affect the worker
already running, so accepting the change would misreport what produced the
transcript. The microphone selector additionally stays disabled until
enumeration finishes.

## Responsiveness

Two blocking calls were removed:

- device enumeration ran `waitForFinished(4000)` during construction, freezing
  the window for as long as the audio backend took to answer. It now runs
  asynchronously and fills the dropdown when it completes.
- starting the worker ran `waitForStarted(1000)`. The state machine waits for
  `READY|` instead.

Whisper has always run in the worker process, so inference never blocked the
event loop.

## Elapsed time and progress

Elapsed time uses `QElapsedTimer`, which is monotonic: unaffected by system
clock changes or by crossing midnight. The previous implementation subtracted
two `QTime::currentTime()` values and would have gone negative at midnight.

The clock starts when the worker reports `READY|`, not when the button is
pressed, so it does not count model loading as recorded audio.

The display shows elapsed against the limit, with a progress bar that fills
toward it. During `LoadingModel`, `Stopping` and `Processing` the bar is
indeterminate, because the worker reports no completion fraction for those
phases.

## Closing during a recording

Closing while `LoadingModel`, `Recording`, `Stopping` or `Processing` asks for
confirmation, because the transcript would be lost. On confirmation the worker
is asked to quit, given three seconds, then killed.

## Long recordings

Limits are 15, 45 and 60 seconds, and 5 or 10 minutes for dictation. `base.en`
is the default model because it transcribes roughly 3.5 times faster than
`small.en`, which is what decides whether a ten minute recording takes about a
minute to transcribe or several.

The final pass is chunked, and the worker reports each chunk as it finishes:

```text
PROGRESS|7|20
```

The interface turns that into a filling bar and a line naming what is left:

```text
Transcribing 7 of 20 (41s elapsed, about 76s left)
```

The estimate is derived from chunks already finished, so it appears only once
there is something to base it on.

**Cancelling.** The Stop button becomes **Cancel** while the worker is loading,
stopping or transcribing. No phase is a dead end: a ten minute recording can
take minutes to transcribe, and a wait with no way out is indistinguishable from
a hang.

## Live text

Partial text arrives as `PARTIAL|latency_ms|queue_seconds|text` and is merged by
word-level overlap; see [STREAMING_TRANSCRIPTION.md](STREAMING_TRANSCRIPTION.md).
The final pass replaces it with the saved transcript.
