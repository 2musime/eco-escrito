# Command-line interface

`eco_escrito_cli` runs the same pipeline as the desktop app, without the
interface. The app itself drives it in the background, so everything the app
does can also be scripted.

```bash
eco_escrito_cli models/ggml-base.en.bin --stream     # live dictation
eco_escrito_cli --list-devices                       # show microphones
eco_escrito_cli models/ggml-base.en.bin --device 1   # use a specific one
eco_escrito_cli --benchmark --input speech.wav models/ggml-base.en.bin
eco_escrito_cli --compare models/ggml-base.en.bin models/ggml-small.en.bin
```

## Options

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
| `--delete-recordings` | Delete all saved recordings and their text |
| `--privacy` | Print what is stored and where |
| `--version`, `--help` | Version and build information, usage |

Models load only from `./models`, the application data directory, or a
directory approved with `--model-dir`. See
[MODEL_VALIDATION.md](MODEL_VALIDATION.md) and
[FILE_STORAGE.md](FILE_STORAGE.md).

## Related

- [STREAMING_TRANSCRIPTION.md](STREAMING_TRANSCRIPTION.md): what `--stream`
  prints and when
- [PERFORMANCE.md](PERFORMANCE.md): reading `--benchmark` output
- [MODEL_COMPARISON.md](MODEL_COMPARISON.md): using `--compare`
- [PRACTICE.md](PRACTICE.md): what `--practice` changes
