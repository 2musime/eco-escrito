# Contributing

Thanks for helping with Eco-escrito. This page covers how work is organised and
what has to pass before it is merged.

## Workflow

1. Branch from `develop`: `feat/<name>`, `fix/<name>`, `docs/<name>` or
   `chore/<name>`.
2. Run the quality gates before you push (see below).
3. Open a pull request into `develop`. `main` only changes at a release,
   through a pull request from `develop`.

Commit messages follow [Conventional Commits](https://www.conventionalcommits.org/):
a short `type(scope): summary` line, then bullet points.

## Quality gates

```bash
tests/run_gates.sh          # guarantees, style, warnings, tests, sanitizers
tests/run_gates.sh --quick  # skip the sanitizer build
```

There is no hosted pipeline: everything runs locally, and the script names
whatever failed. The privacy guarantees (no network code, no oversized or
private files in the repository) are part of it, so a change that breaks them
cannot pass. See [docs/QUALITY_GATES.md](docs/QUALITY_GATES.md) and
[docs/TESTING.md](docs/TESTING.md).

No automated test clicks through the interface, so if you change it, use the
app yourself: start speaking, stop, and check the text.

## Building

```bash
git clone --recurse-submodules git@github.com:2musime/eco-escrito.git
cd eco-escrito
sudo dnf install gcc-c++ cmake make qt6-qtbase-devel
cmake -S . -B build-release
cmake --build build-release --parallel
bash scripts/install-model.sh base.en --dir models
./build-release/eco_escrito
```

This produces `build-release/eco_escrito` (the desktop app) and
`build-release/eco_escrito_cli` (the worker it drives). Windows builds (MSVC
and Qt 6, including Visual Studio presets) are in
[docs/BUILDING.md](docs/BUILDING.md).

## Project layout

```text
.
├── gui_main.cpp          Qt interface
├── main.cpp              CLI worker and transcription pipeline
├── *.h                   Header-only modules: audio, VAD, storage, practice, models
├── packaging/            Desktop entry, icons, Windows resources
├── scripts/              Model installers, icon generation
├── tests/                Unit, GUI and CLI tests, quality gates
├── docs/                 Design and operational documentation
└── third_party/          whisper.cpp (submodule) and miniaudio
```

## Interface changes

The interface follows a few firm rules: plain words, one main button per
screen, and text contrast of at least 4.5:1. Read
[docs/GRAPHICAL_INTERFACE.md](docs/GRAPHICAL_INTERFACE.md) before changing a
screen.

## Releasing

See [docs/RELEASE.md](docs/RELEASE.md).
