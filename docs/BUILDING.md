# Building

## Getting the source

Whisper is a submodule, so clone with it:

```bash
git clone --recurse-submodules git@github.com:2musime/eco-escrito.git
```

In an existing checkout that predates the submodule being registered:

```bash
git submodule update --init --recursive
```

Without this, `third_party/whisper.cpp` is empty and CMake fails at
`add_subdirectory`.

## Dependencies

On Fedora, install the compiler, CMake, and Qt6 development package:

```bash
sudo dnf install gcc-c++ cmake make qt6-qtbase-devel
```

The Whisper source and miniaudio header are kept in `third_party/`.

## Dependencies on Windows

- Visual Studio 2022 or newer with the **Desktop development with C++**
  workload, or the standalone Build Tools. The compiler is MSVC; MinGW is not
  tested here. Visual Studio brings its own CMake and Ninja.
- Git with submodule support.
- Qt 6 for MSVC (64-bit). The official installer works, or, without a Qt
  account, [aqtinstall](https://github.com/miurahr/aqtinstall), whose
  standalone `aqt.exe` needs no Python:

  ```bat
  aqt install-qt windows desktop 6.8.3 win64_msvc2022_64 --outputdir C:\Qt
  ```

  Note the path you install it to; CMake needs it.
- NSIS, only to build the installer. The portable ZIP from the NSIS site is
  enough; put its directory on `PATH`.

Verified with Visual Studio 2026 Community (MSVC 14.51) and Qt 6.8.3 on
Windows 11.

## Production build

From the project root, use a fresh build directory:

```bash
cmake -S . -B build-release
cmake --build build-release --parallel
```

When no build type is supplied, CMake configures this project as `Release`.
The build produces:

```text
build-release/eco_escrito       Qt GUI
build-release/eco_escrito_cli  console worker
```

Run the GUI:

```bash
./build-release/eco_escrito
```

Run the CLI worker directly:

```bash
./build-release/eco_escrito_cli models/ggml-small.en.bin --threads 4
```

Recordings are limited to 15 seconds by default. Supported limits are 15, 45
and 60 seconds, and 300 or 600 seconds for dictation:

```bash
./build-release/eco_escrito_cli models/ggml-small.en.bin \
  --duration 45 --threads 4
```

The limit is enforced for both normal and streaming capture. Empty recordings,
unsupported duration values, buffer overflow, and truncated recordings produce
clear errors instead of unbounded memory growth.

## Windows build

### In Visual Studio

Open the project folder with **File -> Open -> Folder**. `CMakePresets.json`
offers **x64 Debug** and **x64 Release**, which build with Ninja into
`out\build\<preset>\bin` and expect Qt at `C:/Qt/6.8.3/msvc2022_64`. If yours
is elsewhere, create a `CMakeUserPresets.json` (ignored by git) that inherits
`windows-base` and sets `CMAKE_PREFIX_PATH`. The presets put Qt's `bin` on
`PATH`, so the interface starts from the debugger; choose **eco_escrito.exe**
as the startup item. Use **x64 Release** to try transcription: a Debug Whisper
is many times slower.

The same presets work from a Developer Command Prompt:

```bat
cmake --preset x64-release
cmake --build --preset x64-release
ctest --preset x64-release
```

### From the command line

Configure from a Developer Command Prompt, pointing CMake at Qt:

```bat
cmake -S . -B build ^
  -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"
cmake --build build --config Release --parallel
```

Everything lands in one directory, because Windows resolves a DLL from the
folder holding the executable and there is no RPATH to send it elsewhere:

```text
build\bin\Release\eco_escrito.exe       Qt interface
build\bin\Release\eco_escrito_cli.exe   console worker
build\bin\Release\whisper.dll             and the ggml libraries
```

Qt's own libraries are not there yet, so the interface will not start from the
build tree until either Qt's `bin` is on `PATH` or `windeployqt` has been run
against it. Installing does that for you:

```bat
cmake --install build --config Release --prefix C:\opt\eco-escrito
```

The install step invokes `windeployqt` on the installed executable, which
copies the Qt libraries and the platform plugin beside it. If `windeployqt`
cannot be found, CMake says so at configure time and the installed application
will not start.

To produce the release artifacts, with the model inside them:

```bat
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" ^
  -DECO_ESCRITO_BUNDLE_MODEL=ON
cmake --build build --config Release --parallel
cpack -C Release
```

`ECO_ESCRITO_BUNDLE_MODEL` downloads `base.en` once per build tree, about
142 MB, and checks it against the hash pinned in `CMakeLists.txt`. It lands in
`models` beside the executables, which the worker accepts as a model directory.
Leave the option off for ordinary development builds; nobody compiling a change
should have to wait for a download.

`cpack` produces the NSIS installer when `makensis` is on PATH, and the ZIP
either way. The installer adds Start Menu and desktop shortcuts, carries the
application icon, and quietly runs the Visual C++ runtime installer that
`windeployqt` places beside the executables, so a machine without the runtime
can still start the program. `scripts/install-model.ps1` is installed beside
the executables.

### What is not there yet

- **Unsigned binaries.** SmartScreen will warn on anything downloaded from the
  internet until the installer is code-signed.
- **The command-line test suite does not run**, and neither do the sanitizers.
  See [TESTING.md](TESTING.md) and [QUALITY_GATES.md](QUALITY_GATES.md).
- **Live microphone capture is unverified on Windows.** The build, the unit
  and interface tests, transcription of a recorded file, the network-free
  imports, and the installer have been checked on Windows 11; recording from a
  real microphone has not.

## Checks before pushing

Everything is checked locally; there is no hosted pipeline to wait on:

```bash
tests/run_gates.sh          # every gate
tests/run_gates.sh --quick  # skip the sanitizer build
```

That runs the guarantee and formatting checks, builds with
`-DECO_ESCRITO_WARNINGS_AS_ERRORS=ON`, runs the tests, and runs the
sanitizers. See [QUALITY_GATES.md](QUALITY_GATES.md).

## Sanitizer build

Use sanitizers during development. This build is for testing, not production
performance:

```bash
cmake -S . -B build-sanitize \
  -DCMAKE_BUILD_TYPE=Debug \
  -DECO_ESCRITO_ENABLE_SANITIZERS=ON
cmake --build build-sanitize --parallel
```

Run the CLI worker under AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
ASAN_OPTIONS=detect_leaks=1 ./build-sanitize/eco_escrito_cli \
  models/ggml-small.en.bin --threads 2
```

## Compiler warnings

Project targets use:

```text
-Wall -Wextra -Wpedantic
```

On MSVC the equivalent is:

```text
/W4 /permissive- /utf-8
```

`/permissive-` matters most: it rejects the Microsoft extensions that let
non-portable code compile quietly, which is the failure mode this project
cares about.

Warnings from third-party Whisper code may still appear because that dependency
is built as part of the project. They are not changed by this project baseline.