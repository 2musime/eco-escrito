# Installing

Offline speech to text for Linux and Windows. Audio is captured, analysed and
transcribed on your own machine; nothing is uploaded. See
[docs/PRIVACY.md](docs/PRIVACY.md).

## From a package (Fedora)

```bash
sudo dnf install ./eco-escrito-*-Linux.rpm
```

The package pulls in `qt6-qtbase-gui` and **includes both speech models**, in
`/usr/share/eco-escrito/models`. There is nothing else to download: choose
either one in the application.

| In the application | Model | Size | Use |
|---|---|---|---|
| **Faster** | `base.en` | 142 MB | the default; fast, good accuracy |
| **More accurate** | `small.en` | 466 MB | about 3.5 times slower, better accuracy |

Then check it works:

```bash
eco_escrito_cli --version
eco_escrito_cli --list-devices
eco_escrito          # the graphical interface
```

The GUI also appears in the desktop menu under Sound &amp; Video.

## From a package (Windows)

Run `eco-escrito-<version>-win64.exe` and accept the default location. The
installer places the program, the Whisper libraries, the Qt libraries **and both
speech models** (`base.en` for **Faster**, `small.en` for **More accurate**) in
one directory, installs the Microsoft Visual C++
runtime if the machine lacks it, and adds **Eco-escrito** to the Start Menu and
the desktop. There is nothing else to download and nothing to configure: open
it and record.

Windows will warn before it runs: the installer is not code-signed, so
SmartScreen shows *"Windows protected your PC"*. Choose **More info**, then
**Run anyway**. There is no way around this short of buying a code-signing
certificate, and you should be suspicious of any download that asks you to
bypass the warning -- verify the file came from this project's releases page.

The ZIP is the same files, models included, with no installer and no Start Menu
entry. Unpack it anywhere and run `bin\eco_escrito.exe`. Keep the `models`
folder beside the executable; that is where it is found.

Check the installation:

```powershell
& "$env:ProgramFiles\Eco-escrito\bin\eco_escrito_cli.exe" --version
& "$env:ProgramFiles\Eco-escrito\bin\eco_escrito_cli.exe" --list-devices
```

If `--list-devices` shows nothing, or recording produces silence, check
**Settings -> Privacy & security -> Microphone** and confirm that *"Let desktop
apps access your microphone"* is on. Windows blocks the device without telling
the application why, so this looks like a broken microphone rather than a
permission.

## From source

See [docs/BUILDING.md](docs/BUILDING.md). In short:

```bash
git clone --recurse-submodules git@github.com:2musime/eco-escrito.git
cd eco-escrito
sudo dnf install gcc-c++ cmake make qt6-qtbase-devel
cmake -S . -B build-release
cmake --build build-release --parallel
bash scripts/install-model.sh base.en --dir models
./build-release/eco_escrito
```

## Requirements

| | Minimum |
|---|---|
| OS | Fedora Linux or Windows, x86-64 |
| CPU | Any x86-64 processor, 2+ cores (transcription is CPU only) |
| Memory | 1 GB free for `base.en`, 1.5 GB for `small.en` |
| Disk | ~20 MB for the app, plus 142 MB (`base.en`) or 466 MB (`small.en`) |
| Audio | Any microphone reachable through PipeWire, PulseAudio or ALSA, or any Windows input device |
| Network | None after the model is in place |

Full detail in [docs/PERFORMANCE.md](docs/PERFORMANCE.md).

Windows support is built and packaged from the same source as the Linux build.
The build, tests, transcription and installer have been verified on Windows 11;
live microphone capture has not yet been tried there, so report what breaks.

## Where your files go

```text
~/.local/share/eco-escrito/recordings/     WAV audio
~/.local/share/eco-escrito/transcripts/    transcription text
~/.local/share/eco-escrito/models/         models installed with --user
```

On Windows the same three directories sit under
`%LOCALAPPDATA%\eco-escrito\`.

On Linux `$XDG_DATA_HOME/eco-escrito/` is used instead when that variable is set.

They are readable only by you: on Linux through explicit permissions
(directories `0700`, files `0600`), on Windows through the per-user ACL that
`%LOCALAPPDATA%` already carries. If you used the app when it was called Audio
to Text, your `audio-to-text` folder is moved here the first time Eco-escrito
starts. See [docs/FILE_STORAGE.md](docs/FILE_STORAGE.md).

Delete everything stored with:

```bash
eco_escrito_cli --delete-recordings
```

or **Delete Everything...** in **My Words**.

## Uninstalling

Fedora:

```bash
sudo dnf remove eco-escrito
rm -rf ~/.local/share/eco-escrito
```

Windows: uninstall **Eco-escrito** from *Settings -> Apps*, then remove your
data if you want it gone:

```powershell
Remove-Item -Recurse -Force "$env:LOCALAPPDATA\eco-escrito"
```

On both systems the uninstaller removes only the program. The second command
removes your recordings, transcripts and any model you installed -- which is
why it is separate, and deliberately not something an uninstaller decides for
you. Nothing is left elsewhere.

## Known limitations

- x86-64 and CPU only, with no GPU backend.
- English only: the `base.en` and `small.en` models are the ones offered and
  tested.
  Multilingual models load, but they are untested.
- Packaged for Fedora (`.rpm`) and Windows (installer). There is no `.deb` or
  Flatpak package.
- On Windows, the build, tests, transcription and installer are verified on
  Windows 11; live microphone capture has not been tried yet, and the
  installer is unsigned, so SmartScreen warns on first run.
- Practice compares words, not sounds, so a mispronounced word that Whisper
  still recognises with confidence is marked clear.
