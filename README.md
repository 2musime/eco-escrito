# Audio to Text

Speech to text that runs entirely on your own computer. Nothing is uploaded, and
the program opens no network connection.

Linux and Windows, x86-64. Qt 6 interface,
[whisper.cpp](https://github.com/ggml-org/whisper.cpp) for transcription,
CPU only.

## Install

Fedora:

```bash
sudo dnf install ./audio-to-text-*-Linux.rpm
audio-to-text-install-model base.en --user
audio_to_text
```

Windows: run the installer and open **Audio to Text** from the Start Menu. The
`base.en` model is included, so there is nothing further to download.

On Fedora the model is a separate step, and it is the only step that uses the
network. See [INSTALL.md](INSTALL.md).

## Build

```bash
git clone --recurse-submodules git@github.com:2musime/offline-speech-to-text.git
cd offline-speech-to-text
sudo dnf install gcc-c++ cmake make qt6-qtbase-devel
cmake -S . -B build-release && cmake --build build-release --parallel
bash scripts/install-model.sh base.en --dir models
./build-release/audio_to_text
```

## Use

Four screens: **Practice**, **Recording**, **Transcripts**, **Speeches**.

**Practice** opens first. Pick a sentence by the sound you want to work on, or
type your own, press **Start Speaking**, read it, and press **Stop and Check**.
Each word is marked right, changed, missed, or worth checking, so a
mispronunciation that Whisper quietly "corrected" is still flagged. Practice
attempts are not saved. See [docs/PRACTICE.md](docs/PRACTICE.md).

**Recording** is free dictation. Choose a model, microphone and time limit,
record, and the transcript appears. Partial text arrives while you speak; the
full transcript is written when you stop. Past transcripts are readable and
deletable under **Transcripts**, and recordings can be played back under
**Speeches**.

There is also a command-line worker:

```bash
./build-release/audio_to_text_cli models/ggml-base.en.bin --stream
./build-release/audio_to_text_cli --list-devices
./build-release/audio_to_text_cli --privacy
./build-release/audio_to_text_cli --delete-recordings
./build-release/audio_to_text_cli --benchmark --input speech.wav --practice models/ggml-small.en.bin
```

## Where your files go

```text
~/.local/share/audio-to-text/recordings/    WAV audio
~/.local/share/audio-to-text/transcripts/   transcription text
```

On Windows, the same two directories under
`%LOCALAPPDATA%\audio-to-text\`.

Readable only by you: on Linux through explicit permissions (`0700` and
`0600`), on Windows through the per-user ACL that `%LOCALAPPDATA%` carries.
Nothing is written to a log file.

## Privacy

Audio is captured, analysed and transcribed locally. Neither binary imports a
network syscall, and a full transcription completes inside a network namespace
with no interfaces. The checks are in `tests/check_guarantees.sh` and run on
every build. See [docs/PRIVACY.md](docs/PRIVACY.md).

## Performance

12 seconds of speech on a 12-thread machine:

| Model | Threads | Transcribe | Real-time factor |
|---|---:|---:|---:|
| `base.en` | 8 | 1.0 s | 0.08 |
| `small.en` | 8 | 3.5 s | 0.29 |

`base.en` is the default. Measure your own with
`audio_to_text_cli --benchmark --input speech.wav models/ggml-base.en.bin`.
See [docs/PERFORMANCE.md](docs/PERFORMANCE.md) for hardware requirements.

## Development

```bash
tests/run_gates.sh          # guarantees, style, warnings, tests, sanitizers
tests/run_gates.sh --quick  # skip the sanitizer build
```

| Topic | Document |
|---|---|
| Building and dependencies | [docs/BUILDING.md](docs/BUILDING.md) |
| Interface and keyboard | [docs/GRAPHICAL_INTERFACE.md](docs/GRAPHICAL_INTERFACE.md) |
| Pronunciation practice | [docs/PRACTICE.md](docs/PRACTICE.md) |
| Tests | [docs/TESTING.md](docs/TESTING.md) |
| Quality gates | [docs/QUALITY_GATES.md](docs/QUALITY_GATES.md) |
| Where files go, and confinement | [docs/FILE_STORAGE.md](docs/FILE_STORAGE.md) |
| Models and checksums | [docs/MODEL_VALIDATION.md](docs/MODEL_VALIDATION.md) |
| Streaming behaviour | [docs/STREAMING_TRANSCRIPTION.md](docs/STREAMING_TRANSCRIPTION.md) |
| Releasing | [docs/RELEASE.md](docs/RELEASE.md) |

## Limitations

- x86-64 and CPU only; no GPU backend
- Packaged for Fedora (`.rpm`) and Windows (installer and ZIP); no `.deb`, no
  Flatpak
- Windows builds and packages from the same source, but has not yet been run on
  a Windows machine. The sanitizers and the command-line suite do not run there,
  and the installer is unsigned, so SmartScreen will warn on first run
- English models (`base.en`, `small.en`); multilingual models load but are untested
- Practice compares words, not sounds: a mispronounced word that Whisper still
  recognises confidently is marked right
- No automated test drives the interface
