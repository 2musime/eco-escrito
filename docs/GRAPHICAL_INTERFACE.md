# Graphical interface

The Qt6 interface is built as `eco_escrito`. The recorder and Whisper pipeline
live in `eco_escrito_cli`; the GUI drives that worker with `QProcess` and
never loads Whisper itself, so the event loop stays free.

## Screens

A welcome page and four screens, one job each, chosen from the bar across the
top. The screen you are on is highlighted in the bar.

| Screen | Holds | Reached by |
|---|---|---|
| **Welcome** | the icon, one line, two ways in | opens here; the name in the bar, or `Ctrl+0` |
| **Practice** | a sentence to read, one button, the result word by word | `Practice`, `Ctrl+1`, or **Practise Pronunciation** |
| **Recording** | settings, the record panel, the live transcript | `Recording`, `Ctrl+2`, or **New Recording** |
| **Transcripts** | the saved list and a reader | `Transcripts`, or `Ctrl+H` |
| **Speeches** | recordings, and a player for them | `Speeches`, or `Ctrl+P` |

The bar is navigation and nothing else. Pressing **Recording** goes to the
recording screen; it does not start a recording. Starting is done there, with
the model, microphone and limit already in view.

## Welcome

The welcome page has no tab of its own; the name in the bar leads back to it.
It is deliberately almost empty, on white, because every screen is already one
click away in the bar:

- the application icon
- one line: *Your voice, in writing. On your own computer.*
- **New Recording**, the primary action, and **Practise Pronunciation** beside it

A new feature gets a tab in the bar. It earns a button here only if it becomes
one of the main reasons to open the application.

## One design for every screen

Every screen follows the welcome page:

- **White, and a centred column.** Each screen is capped (Practice 760 px,
  Recording 860 px, the two library screens 1120 px), so a full-screen window
  does not stretch a line across 1920 px.
- **The content is the hero.** The sentence on Practice, the clock on Recording,
  the transcript on Transcripts, and the player on Speeches.
- **One filled button per screen,** the pill in the accent blue: **Start
  Speaking**, **Start Recording**, **Play**. It turns red and reads **Stop** while
  recording.
- **Everything else is a quiet text button:** blue for actions, red for deleting.
  None of them has a border or a fill until it is hovered.
- **No titles or explanations on the page.** The tab already says where you are.
  What a control does is in its tooltip, and what the application is doing is
  in the footer.
- **Grey fills instead of bordered cards:** the live transcript, the practice
  result, and your own practice sentence.

Navigation closes while a recording or transcription is running. The controls
for something still running must not be hidden behind another screen.

## Where the actions live

Each screen owns what belongs to it, so nothing is hidden in a menu away from
the thing it affects:

| Action | Where |
|---|---|
| Choose, type, and read a practice sentence | the practice screen |
| Start, stop, cancel | the screen that started it |
| Save, copy the live transcript | beside that transcript |
| Save, copy, delete one saved transcript | beside the transcript being read |
| Delete all recordings | the transcripts screen, which is what it empties |
| Privacy, About, Quit | `Help` |

`Help` is the only entry in the bar that opens a menu; the rest switch screens.

## Look

One light theme, applied to the whole application so dialogs match the window.
The Fusion style draws the same on Linux and Windows, which is what lets one
style sheet serve both. The palette is fixed rather than taken from the
desktop: the colours assume a light background, and a dark system palette would
put pale text on white.

- **Colours:** the macOS system palette, so the application reads as a calm
  Mac utility on any desktop: `#ffffff` pages, `#f5f5f7` fills, `#1d1d1f`
  text, `#6e6e73` secondary text, `#007aff` accent, `#ff3b30` destructive,
  `#e5e5ea` hairlines.
- **Header:** the name (which also leads to the welcome page), the four screens, an
  **Offline** badge, and Help. The window will not narrow past the width the
  bar needs, so no label is ever cut short. The badge keeps the privacy
  promise visible on every screen without repeating it.
- **Footer:** what the screen is doing on the left; **Privacy** and the version
  on the right. The title bar carries the name alone.
- **Dropdowns:** rounded, with a drawn chevron shipped in normal and `@2x`
  sizes, so they stay crisp on high-density screens. The chevron is a PNG
  because an SVG would need the Qt SVG plugin, which the package does not
  depend on.
- **Lists:** a sidebar divided from its content by a hairline. The selected
  row is filled in the accent colour.

## Recording screen

```text
      [Speed: base.en v] [System default v] [15 seconds v]  [x] Keep audio

                           00:23 / 01:00
                       ( ■  Stop Recording )
                          * Recording. Speak now.
                            ----------
                                                          Copy  Save
+-------------------------------------------------------------------+
|  Your words will appear here.                                     |
+-------------------------------------------------------------------+
```

**Settings need no labels.** Each value says what it is (`Speed: base.en`,
`15 seconds`, the microphone's name), and each list is sized to its contents.

**One button.** It reads **Start Recording**, then **Stop Recording** (in red)
while recording, then **Cancel** while the model loads or the audio is being
transcribed. `Ctrl+R` and `Esc` do the same.

**Context lives in the status bar.** The model, input device and output path
were four stacked labels reading `Model: not loaded`, `Input: not selected`,
`Saving to: not known yet` before anything had happened, which looks like a list
of failures. They are now one line, showing only what is actually known.

The progress bar is hidden unless something is running: an empty bar on an idle
window suggests stalled work.

## Saved transcripts

`Transcripts` in the top bar, or `Ctrl+H`, shows the library: a list of every
transcript the application has written, newest first, each named for when it was
made, the way a person would say it:

```text
Today, 14:32
Yesterday, 18:11
Fri 9 Oct, 10:10
```

That name and nothing else, matching how recordings are named on the Speeches
screen. The file name on disk is in each row's tooltip. The newest one is
selected and shown on arrival. What a transcript says is one click away; repeating a fragment of it on
every row is noise.

It is a separate screen rather than a panel beside the recorder. Nothing on it
can start a recording, because none of the recording controls are present: there
is no model selector, no limit, no record button and no progress bar. A control
that cannot be reached needs no rule about when it may be used.

Selecting a row reads that transcript into the pane on the right, headed with
the same name, whose tooltip says when the recording it came from is gone. The reader has its own view, so nothing about browsing disturbs a live
transcription.

**The library cannot be opened while a recording or transcription is running.**
The whole navigation bar greys out until the worker finishes, and the recording
shortcuts are inert while the library is showing.

**Delete** sits beside **Copy** and **Save a Copy**, above the transcript it would
remove, in red text. It is enabled only while a transcript is on screen, and
asks for confirmation naming the date first.

The recording the transcript came from is kept.
**Delete All Recordings**, under the list, remains the only way to remove audio.

After a deletion the reader moves to a neighbouring transcript rather than
going blank: the row that takes the deleted one's place, or the row above when
the deleted one was last. Only when nothing is left does the reader empty and
the list show `No transcripts yet`.

Right-clicking a row additionally offers:

- **Copy** and **Save a Copy** of the transcript being read
- **Audio:** which recordings survive for that session, or `not kept` when
  retention was off or they have been deleted
- **Delete This Transcript** — the same action as the button

The list is rebuilt when a new transcript is saved and after deleting
everything. With nothing stored it reads `No transcripts yet` rather than
showing an empty box.

Reading is confined the same way writing is: a path resolving outside the
application's own directory, or one that is a symbolic link, is refused rather
than followed. See [FILE_STORAGE.md](FILE_STORAGE.md).

## Speeches

`Speeches`, or `Ctrl+P`, lists every recording newest first, each named for when
it was made, as transcripts are (`Today, 14:32`).

That name and nothing else. Length, size and whether it was transcribed belong to
the recording you have selected, not to every row, and appear under its name in
the player once one is chosen: `0:07 · 0.2 MB · Transcribed`. The player is
centred: name, a large clock, the position slider, then **Play** and a quiet
**Stop**. Selecting a row loads it; **Play**, **Stop** and the position
slider work as they do anywhere else.

**What plays is the raw capture.** Each recording produces three files: the raw
audio, a noise-reduced copy, and the speech extracted from it. Only the
extracted speech is sent to Whisper. Playback deliberately gives back the raw
recording, because that is what was actually said; the processed versions exist
for transcription, not for listening.

Playback stops when you leave the screen, so audio never continues behind a
recording.

**Delete Recording**, in red text at the bottom right, removes the raw capture and its two processed copies. The
transcript is kept: text is small, and losing it along with the audio would be a
surprise. Deleting moves to a neighbouring recording rather than going blank.

Audio is played through the vendored miniaudio, the same library used to
capture, so the application needs no media framework and the package still
depends only on `qt6-qtbase-gui`.

## Keyboard

Every screen's shortcut appears in its tooltip in the top bar, so it can be
found rather than memorised.

| Keys | Action |
|---|---|
| `Ctrl+R` | Start speaking or recording, or stop one in progress |
| `Esc` | Cancel whatever the worker is doing |
| `Ctrl+S` | Save the transcript to a file |
| `Ctrl+Shift+C` | Copy the whole transcript |
| `Ctrl+0` | Go to the welcome page |
| `Ctrl+1` | Go to Practice |
| `Ctrl+2` | Go to Recording |
| `Ctrl+H` | Go to saved transcripts |
| `Ctrl+P` | Go to recordings |
| `Up` / `Down` | Move through the saved transcripts |
| `Ctrl+Q` | Quit |

`Ctrl+R` is one action rather than two, so the same key both starts and stops.
On the practice screen it starts a practice attempt; anywhere else it goes to
Recording and starts there.
Plain `Ctrl+C` is left to the transcript for copying a selection, which is why
copying the whole transcript takes the shifted form.

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
