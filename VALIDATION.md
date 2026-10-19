# Validation — Lowline 0.3.0

Verified source: `e6866c64e7505d48c91050501b9eab242e8d0356`. [Native builds run 39188625927](https://github.com/MassimoDanieli/lowline/actions/runs/39188625927) passed compilation and regression tests on macOS universal, Windows x64 and Linux x64. The local screenshot run passed 170 assertions, including navigation/render checks.

## Verification scope

The regression executable uses the actual JUCE processor and NAM core, with synthetic audio and fixture captures. Local Release builds cover Linux standalone, VST3 and tests. GitHub's Native builds workflow builds/tests macOS universal, Windows x64 and Linux x64; the 0.3 release job depends on all three succeeding. Mac architecture checks verify arm64 and x86_64 in the standalone executable; tests run on the runner's own architecture.

New 0.3 checks cover:

- Offline WSOLA duration and preserved 220 Hz pitch at 0.5, 0.75, 1.25 and 1.5 speeds, with bounded samples. This establishes fixture pitch behaviour, not quality on arbitrary music.
- Worker decoding, advancing playback, bounded loop wrap, and metronome output/beat count.
- Closing and reading both recorded files: matching sample counts, mono DI/stereo FX, 32-bit float, separate expected signal values and refusal to overwrite an existing take.
- A/B capture/recall, RMS reference compensation, silence rejection, five-second measured processor output and temporary trim.
- Dirty-state fingerprint, parameter undo and preset metadata roundtrip.
- Outgoing room convolution energy continuing beyond the 50 ms engine transition.
- Native workspace navigation and renders for Library, Compare, Practice, Stage and Audio/App.
- Eighteen offline help topics in both languages. The installer/signing shell scripts are syntax-checked; workflow YAML is parsed.

Existing 0.2 regression checks remain: tuner low B through A4, silence/DC rejection, noise gate, five-band response, clean low-pass, host/manual delay sync, MIDI learn and persistence, PC-assigned user presets, four-slot portable import after source deletion, malformed portable rejection, NAM metadata normalisation, capture crossfade continuity and concurrent loads, old preset defaults, limiter ceiling/stereo balance/latency, mono folding, real NAM known-gain processing, cabinet convolution and missing-file handling.

## Runtime boundaries

No test certifies Scarlett round-trip latency, DAW/AU host validation, arbitrary model performance, subjective sound quality or stage readiness. Public Mac builds remain unsigned by Developer ID and unnotarised. The optional signing script needs an owner's valid certificate and stored notarytool profile; notarisation has not been performed here.

Preset assets are prepared as a group, then published under a short lock that the audio thread only tries (never waits for). The lock covers adoption, not DSP processing. Controls still apply before preparation completes, and existing fades may defer a slot. This is not an atomic scene switch. At most one previous room tail is retained for its IR duration, with a final 100 ms ramp; another replacement can truncate it. Room disable/clear/mix changes can mask the tail. Delay settings and the old complete rig are not cloned. Crossfades/tails temporarily increase CPU and level.

Backing is decoded and stretched on a worker. The audio thread uses a try-lock for backing publication and recording-writer access; a missed lock can skip a backing block or count a dropped recording block. WSOLA is a basic correlation/overlap algorithm with possible transient smearing. Loading/rendering can use significant memory; files are capped at 10 minutes AND 16M samples/channel. Practice settings and A/B snapshots are temporary, not DAW/preset state. Metronome tempo is independent of the host and backing. Host bypass does not advance the practice/recording path.

Recording uses buffered background disk writers. DI is the selected input before gain/effects; FX is processed bass plus match trim, before the monitor limiter/tuner mute. Backing and click are excluded. Float WAVs may exceed 0 dBFS. A non-zero dropped-block count invalidates assumptions about gap-free or aligned recording. Closing the instance/audio reconfiguration stops and flushes recording. This is not a crash-proof recorder or a physical latency measurement.

A/B level matching is left-channel RMS from five seconds of played material, not LUFS. Use identical input material/gain. Silence below -60 dB RMS is rejected; gain is bounded to ±12 dB. Match gain is temporary and excluded from presets. NAM normalisation remains a separate metadata-based process.

Undo covers parameter-tree/editorial changes, not file loading, transport, recording or catalogue changes. The library stores paths and local favourites/tags; simultaneous edits in multiple instances use last-save-wins semantics. Removing a catalogue row does not delete the source file. No cloud catalogue/API integration or automatic updater has been added: TONE3000 and update checks open explicit browser links.

## Reproduce

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
build/LowlineTests_artefacts/Release/LowlineTests --screenshot
```

On Windows append `.exe`; headless Linux may need `xvfb-run -a`. Source/dependency provenance remains in THIRD_PARTY_NOTICES.md. Native screenshots are copied into docs/ after visual review. CI artifacts include test logs and screenshots; published downloads include SHA256SUMS.txt.
