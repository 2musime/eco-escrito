# Installing

Offline speech to text for Linux and Windows. Audio is captured, analysed and
transcribed on your own machine; nothing is uploaded. See
[docs/PRIVACY.md](docs/PRIVACY.md).

## From a package (Fedora)

```bash
sudo dnf install ./eco-escrito-*-Linux.rpm
```

The package pulls in `qt6-qtbase-gui`. It does **not** include a speech model:
models are hundreds of megabytes and downloading one is the only step that ever
touches the network.

Install a model after the application:

```bash
eco-escrito-install-model base.en --user
```

| Model | Size | Use |
|---|---|---|
| `base.en` | 142 MB | the default; fast, good accuracy |
| `small.en` | 466 MB | slower, better accuracy |
| `tiny.en` | 75 MB | fastest, least accurate |

Then check it works:

```bash
eco_escrito_cli --version
eco_escrito_cli --list-devices
eco_escrito          # the graphical interface
```

The GUI also appears in the desktop menu under Sound &amp; Video.

## From a package (Windows)

Run `eco-escrito-<version>-win64.exe` and accept the default location. The
installer places the program, the Whisper libraries, the Qt libraries **and the
`base.en` speech model** in one directory, installs the Microsoft Visual C++
runtime if the machine lacks it, and adds **Eco-escrito** to the Start Menu and
the desktop. There is nothing else to download and nothing to configure: open
it and record.

That is the only difference from the Fedora package, where the model is fetched
separately. Windows has no distribution to provide one, and a first run that
ends in a PowerShell command is a poor way to meet an application.

Windows will warn before it runs: the installer is not code-signed, so
SmartScreen shows *"Windows protected your PC"*. Choose **More info**, then
**Run anyway**. There is no way around this short of buying a code-signing
certificate, and you should be suspicious of any download that asks you to
bypass the warning -- verify the file came from this project's releases page.

The ZIP is the same files, model included, with no installer and no Start Menu
entry. Unpack it anywhere and run `bin\eco_escrito.exe`. Keep the `models`
folder beside the executable; that is where it is found.

To use a different model, `small.en` for better accuracy:

```powershell
& "$env:ProgramFiles\Eco-escrito\bin\install-model.ps1" small.en
```

It downloads into `%LOCALAPPDATA%\eco-escrito\models` and prints the file's
SHA-256. Then check the installation:

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

x86-64 Linux or Windows, about 1 GB free memory for `base.en`, a microphone, and
no network connection at all after the model is in place. Full detail in
[docs/PERFORMANCE.md](docs/PERFORMANCE.md).

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

They are readable only by you: on Linux through explicit permissions, on
Windows through the per-user ACL that `%LOCALAPPDATA%` already carries. Delete
everything stored with:

```bash
eco_escrito_cli --delete-recordings
```

or the **Delete recordings** button in the interface.

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
