# Eco-escrito

**Offline speech-to-text and pronunciation practice for your desktop.**
Eco-escrito records your voice and transcribes it on your own computer with
[whisper.cpp](https://github.com/ggml-org/whisper.cpp). Nothing is uploaded, and
the program never opens a network connection.

![Version](https://img.shields.io/badge/version-1.2.0-blue)
![Platforms](https://img.shields.io/badge/platform-Linux%20%7C%20Windows-lightgrey)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C)
![Qt 6](https://img.shields.io/badge/Qt-6-41CD52)
![License: MIT](https://img.shields.io/badge/license-MIT-green)

## Table of contents

- [Features](#features)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick start](#quick-start)
- [Command-line interface](#command-line-interface)
- [Where your files go](#where-your-files-go)
- [Privacy](#privacy)
- [Performance](#performance)
- [Building from source](#building-from-source)
- [Development](#development)
- [Documentation](#documentation)
- [Known limitations](#known-limitations)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Features

- **Fully offline.** Audio is captured, analysed and transcribed locally. A
  network connection is only used once, to download a model, and the Windows
  installer ships with one built in.
- **Pronunciation practice.** Read a sentence aloud and each word is marked
  right, changed, missed or worth checking. A mispronunciation that Whisper
  quietly "corrected" is still flagged.
- **Live dictation.** Partial text appears while you speak, and the full
  transcript is written when you stop.
- **Transcript and recording library.** Read, play back and delete past
  sessions from the app.
- **Private by default.** Files are written only to a per-user directory that
  only you can read, with atomic writes and no log files.
- **Scriptable CLI** for streaming, benchmarking and comparing models.
- **Verified guarantees.** Every build checks that neither binary imports a
  network syscall.

## Requirements

| | Minimum |
|---|---|
| OS | Fedora Linux or Windows, x86-64 |
| CPU | Any x86-64 processor, 2+ cores (transcription is CPU only) |
| Memory | 1 GB free for `base.en`, 1.5 GB for `small.en` |
| Disk | ~20 MB for the app, plus 142 MB (`base.en`) or 466 MB (`small.en`) |
| Audio | Any microphone reachable through PipeWire, PulseAudio or ALSA, or any Windows input device |

Full hardware notes are in [docs/PERFORMANCE.md](docs/PERFORMANCE.md).

## Installation

### Fedora

```bash
sudo dnf install ./eco-escrito-*-Linux.rpm
eco-escrito-install-model base.en --user
```

The package does not include a speech model, because models are hundreds of
megabytes. Downloading the model is the only step that uses the network. The
package replaces an earlier `audio-to-text` install.

### Windows

Run `eco-escrito-<version>-win64.exe` and open **Eco-escrito** from the Start
Menu. The `base.en` model is included, so there is nothing more to download. A
portable ZIP is also available.

The installer is not code-signed yet, so SmartScreen will warn the first time it
runs.

Other models, upgrading and uninstalling are covered in [INSTALL.md](INSTALL.md).

## Quick start

Launch the app with `eco_escrito`, or from your desktop's application menu.

The app opens on **Home**, which has a tile for each feature and your most
recent transcripts. From there:

| Screen | What it does |
|---|---|
| **Practice** | Pick a sentence by the sound you want to work on, or type your own. Press **Start Speaking**, read it aloud, then press **Stop and Check**. Practice attempts are not saved. |
| **Recording** | Free dictation. Choose a model, microphone and time limit, then record. |
| **Transcripts** | Read or delete saved transcripts. |
| **Speeches** | Play back saved recordings. |

See [docs/PRACTICE.md](docs/PRACTICE.md) and
[docs/GRAPHICAL_INTERFACE.md](docs/GRAPHICAL_INTERFACE.md) for details and
keyboard shortcuts.

## Command-line interface

`eco_escrito_cli` runs the same pipeline without the interface.

```bash
eco_escrito_cli models/ggml-base.en.bin --stream     # live dictation
eco_escrito_cli --list-devices                       # show microphones
eco_escrito_cli models/ggml-base.en.bin --device 1   # use a specific one
eco_escrito_cli --benchmark --input speech.wav models/ggml-base.en.bin
eco_escrito_cli --compare models/ggml-base.en.bin models/ggml-small.en.bin
```

| Option | Description |
|---|---|
| `--stream` | Print partial text while recording |
| `--duration 15\|45\|60\|300\|600` | Recording limit in seconds |
| `--threads N` | Transcription threads |
| `--device INDEX` | Capture device, as listed by `--list-devices` |
| `--model-dir DIR` | Approve an extra directory to load models from |
| `--practice` | Literal transcription with per-word confidence |
| `--no-retain-audio`, `--no-retain-transcript` | Do not keep the recording or the transcript |
| `--benchmark --input FILE` | Time the pipeline on a 16 kHz mono 16-bit WAV |
| `--compare MODEL MODEL ...` | Transcribe the same input with several models |
| `--delete-recordings` | Delete all saved recordings |
| `--privacy` | Print what is stored and where |
| `--version`, `--help` | Version and build information, usage |

Models load only from `./models`, the application data directory, or a
directory approved with `--model-dir`.

## Where your files go

| Platform | Location |
|---|---|
| Linux | `~/.local/share/eco-escrito/` (or `$XDG_DATA_HOME/eco-escrito/`) |
| Windows | `%LOCALAPPDATA%\eco-escrito\` |

```text
eco-escrito/
├── recordings/    WAV audio
├── transcripts/   transcription text
└── models/        models installed with --user
```

On Linux the directories are `0700` and the files `0600`. On Windows they
inherit the per-user ACL of `%LOCALAPPDATA%`. If you used the app when it was
called Audio to Text, your existing `audio-to-text` folder is moved to the new
location the first time Eco-escrito starts. See
[docs/FILE_STORAGE.md](docs/FILE_STORAGE.md).

## Privacy

- No network code. Neither binary imports a network syscall.
- A full transcription completes inside a network namespace with no interfaces.
- Both checks run on every build (`tests/check_guarantees.sh`).

See [docs/PRIVACY.md](docs/PRIVACY.md).

## Performance

12 seconds of speech on a 12-thread machine:

| Model | Threads | Transcription time | Real-time factor |
|---|---:|---:|---:|
| `base.en` (default) | 8 | 1.0 s | 0.08 |
| `small.en` | 8 | 3.5 s | 0.29 |

A real-time factor below 1.0 means transcription runs faster than real time. To
measure on your own hardware, run
`eco_escrito_cli --benchmark --input speech.wav models/ggml-base.en.bin`.

## Building from source

```bash
git clone --recurse-submodules git@github.com:2musime/eco-escrito.git
cd eco-escrito
sudo dnf install gcc-c++ cmake make qt6-qtbase-devel
cmake -S . -B build-release
cmake --build build-release --parallel
bash scripts/install-model.sh base.en --dir models
./build-release/eco_escrito
```

| Output | Description |
|---|---|
| `build-release/eco_escrito` | Qt desktop app |
| `build-release/eco_escrito_cli` | Command-line worker |

Windows builds (MSVC and Qt 6) and packaging are described in
[docs/BUILDING.md](docs/BUILDING.md) and [docs/RELEASE.md](docs/RELEASE.md).

## Development

```bash
tests/run_gates.sh          # guarantees, style, warnings, tests, sanitizers
tests/run_gates.sh --quick  # skip the sanitizer build
```

### Project layout

```text
.
├── gui_main.cpp          Qt interface
├── main.cpp              CLI worker and transcription pipeline
├── *.h                   Header-only modules: audio, VAD, storage, practice, models
├── packaging/            Desktop entry, icons, Windows resources
├── scripts/              Model installers, icon generation
├── tests/                Unit, GUI and CLI tests, quality gates
├── docs/                 Design and operational documentation
└── third_party/          whisper.cpp and miniaudio (submodules)
```

## Documentation

| Topic | Document |
|---|---|
| Installing, upgrading, uninstalling | [INSTALL.md](INSTALL.md) |
| Building and dependencies | [docs/BUILDING.md](docs/BUILDING.md) |
| Interface and keyboard | [docs/GRAPHICAL_INTERFACE.md](docs/GRAPHICAL_INTERFACE.md) |
| Pronunciation practice | [docs/PRACTICE.md](docs/PRACTICE.md) |
| Streaming behaviour | [docs/STREAMING_TRANSCRIPTION.md](docs/STREAMING_TRANSCRIPTION.md) |
| Audio capture | [docs/AUDIO_CAPTURE.md](docs/AUDIO_CAPTURE.md) |
| Noise reduction | [docs/NOISE_REDUCTION.md](docs/NOISE_REDUCTION.md) |
| Models and checksums | [docs/MODEL_VALIDATION.md](docs/MODEL_VALIDATION.md) |
| Model comparison | [docs/MODEL_COMPARISON.md](docs/MODEL_COMPARISON.md) |
| File storage and confinement | [docs/FILE_STORAGE.md](docs/FILE_STORAGE.md) |
| Error handling | [docs/ERROR_HANDLING.md](docs/ERROR_HANDLING.md) |
| Privacy | [docs/PRIVACY.md](docs/PRIVACY.md) |
| Performance | [docs/PERFORMANCE.md](docs/PERFORMANCE.md) |
| Tests | [docs/TESTING.md](docs/TESTING.md) |
| Quality gates | [docs/QUALITY_GATES.md](docs/QUALITY_GATES.md) |
| Releasing | [docs/RELEASE.md](docs/RELEASE.md) |

## Known limitations

- x86-64 and CPU only, with no GPU backend.
- Packaged for Fedora (`.rpm`) and Windows (installer and ZIP). There is no
  `.deb` or Flatpak package.
- The Windows build is produced from the same source but has not yet been run
  on a Windows machine. The sanitizers and the CLI test suite do not run there.
- Only the English models (`base.en`, `small.en`) are tested. Multilingual
  models load, but they are untested.
- Practice compares words, not sounds, so a mispronounced word that Whisper
  still recognises with confidence is marked right.
- No automated test drives the interface.

## Contributing

1. Branch from `develop` (`feat/<name>`, `fix/<name>`).
2. Run `tests/run_gates.sh` before you push.
3. Open a pull request into `develop`. `main` is only updated at release
   boundaries.

Commit messages follow [Conventional Commits](https://www.conventionalcommits.org/).

## License

Released under the MIT License.

## Acknowledgements

- [whisper.cpp](https://github.com/ggml-org/whisper.cpp) for speech recognition
- [OpenAI Whisper](https://github.com/openai/whisper) for the models
- [miniaudio](https://miniaud.io/) for audio capture
- [Qt 6](https://www.qt.io/) for the interface
