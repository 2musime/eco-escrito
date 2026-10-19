# Release

## Versioning

The version lives in one place, `project(... VERSION x.y.z)` in `CMakeLists.txt`.
CMake generates `version.h` from it and records the short git commit, so a build
can always identify itself:

```bash
eco_escrito_cli --version
# eco_escrito 1.4.0 (f9d338a, Release)
```

The GUI shows the same under **Help > About**.

## Checklist

Work top to bottom. Anything that fails stops the release.

**1. The tree is clean**

```bash
git status --porcelain          # empty
git log --oneline develop..HEAD # nothing unmerged
```

**2. Every gate passes**

```bash
tests/run_gates.sh
```

Guarantees, formatting, a build with warnings as errors, the full test suite,
and the sanitizers. It names whatever failed. See
[QUALITY_GATES.md](QUALITY_GATES.md).

**3. The interface has been used by a person**

Start a recording, speak, stop, and confirm the transcript. No automated test
clicks a button, so this step is the only thing standing between a broken
interface and a release.

**4. The version is bumped** in `CMakeLists.txt`, and `--version` reports it.
Nothing else needs editing: the install commands in `README.md`, `INSTALL.md`
and this file match any version on purpose. Confirm with:

```bash
grep -rn "eco-escrito-[0-9]" README.md INSTALL.md docs/ | grep -v '\*'
```

Anything it prints is a hard-coded version that will go stale. The one
deliberate exception is the version badge at the top of `README.md`, which
must be bumped by hand.

**5. A fresh clone builds**

```bash
git clone --recurse-submodules <url> /tmp/release-check
cmake -S /tmp/release-check -B /tmp/release-check/build
cmake --build /tmp/release-check/build --parallel
```

**6. Packages build**

```bash
cmake --preset linux-package
cmake --build --preset linux-package
cd build-package && cpack
```

Produces `eco-escrito-<version>-Linux.rpm` and a `.tar.gz`, each carrying
`small.en` in `/usr/share/eco-escrito/models`. The preset turns on
`ECO_ESCRITO_BUNDLE_MODEL`; a package built without it has no model.

**7. The package installs on a clean system** with no build tools present, and
the installed binaries run. See below.

**8. Performance has not regressed**

```bash
./build-release/eco_escrito_cli --benchmark --input speech.wav \
  models/ggml-base.en.bin models/ggml-small.en.bin
```

Compare against [PERFORMANCE.md](PERFORMANCE.md). Run-to-run variance is about
10%; do not chase differences smaller than that.

**9. The Windows artifacts were built and attached**

On a Windows machine, from the same commit:

```bat
cmake -S . -B build-release -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" ^
  -DECO_ESCRITO_BUNDLE_MODEL=ON
cmake --build build-release --config Release --parallel
cpack -C Release
```

Produces `eco-escrito-<version>-win64.exe` when `makensis` is installed, and
a ZIP either way. Run `tests/run_gates.sh --quick` under Git Bash there too; it
reports which gates do not run on Windows rather than claiming a clean sweep.

This step needs a machine this project does not otherwise require, so it is the
one most likely to be skipped. Skipping it is a decision, not an oversight: if
no Windows artifact ships, say so on the release page instead of leaving people
to guess whether the version applies to them. A release whose Linux and Windows
artifacts are different versions is worse than one that ships Linux only.

**10. Tag and publish**

```bash
git tag -a v.1.4.0 -m "1.4.0"
git push origin v.1.4.0
```

Attach every artifact to one GitHub release, so a single version number covers
them all. Publishing a release page is not GitHub Actions and works on this
repository's plan; only automated building does not.

## Building packages

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
cmake --build build-release --target package
```

`rpm-build` must be installed for the RPM generator, and `makensis` for the
Windows installer. Both are optional: without them CPack produces the archive
formats and says which generator it skipped.

## What ships, and what does not

| Component | Packaged | Why |
|---|---|---|
| `eco_escrito`, `eco_escrito_cli` | yes | the application |
| `libwhisper`, `libggml`, `libggml-base`, `libggml-cpu` | yes, privately | built from the pinned submodule; installed to `/usr/lib64/eco-escrito` rather than competing with a system Whisper |
| Qt 6 | no, required | comes from the distribution as `qt6-qtbase-gui` |
| Desktop entry and icons (Linux) | yes | so the GUI appears in the menu with its icon |
| Start Menu shortcut (Windows) | yes | created by the NSIS installer |
| Qt 6 (Windows) | yes | no distribution to provide it; `windeployqt` copies it in at install time |
| `eco-escrito-install-model`, `install-model.ps1` | yes | fetches a model after installation |
| Whisper model | yes | `small.en`, the one the interface uses, so installing the application is all it takes; checked against the hash pinned in `CMakeLists.txt` |

The model makes each package about 450 MB. That is the price of a program that
works the moment it is installed, offline, with nothing further to fetch.

## Verifying a clean installation

This step is not optional, and it cannot be done on a development machine.

A machine that built the software already has every library the build produced,
so an installed binary can resolve them by accident even when the package is
wrong. The first package built here looked correct, listed the right files, and
declared the right dependency, but the installed CLI would not start:

```text
eco_escrito_cli: error while loading shared libraries: libwhisper.so.1
```

The runtime path had been built from `CMAKE_INSTALL_PREFIX`, which is
`/usr/local` by default, while CPack stages the package under `/usr`. The binary
searched a directory that did not exist. Only a machine without the build tree
exposes that. The path is now `$ORIGIN`-relative, so it resolves against the
binary's own location instead of a prefix fixed at configure time.

Run it in a throwaway container, which needs no spare machine:

```bash
podman run --rm -v "$PWD/build-release:/pkg:Z" fedora:41 bash -c '
  dnf -y install /pkg/eco-escrito-*.rpm &&
  eco_escrito_cli --version &&
  ldd /usr/bin/eco_escrito_cli | grep "not found" && echo BROKEN || echo OK'
```

Confirm before shipping:

- no library reports `not found` for either binary
- `eco_escrito_cli --version` prints the expected version
- the GUI starts under `QT_QPA_PLATFORM=offscreen`
- `desktop-file-validate` accepts the desktop entry
- `eco-escrito-install-model --help` runs



```bash
sudo dnf install ./eco-escrito-*-Linux.rpm
eco_escrito_cli --version
ls /usr/share/eco-escrito/models    # ggml-small.en.bin
eco_escrito_cli --list-devices
```

The installed binaries carry an RPATH pointing at `/usr/lib64/eco-escrito`, so
they resolve the private Whisper libraries without `LD_LIBRARY_PATH`.
