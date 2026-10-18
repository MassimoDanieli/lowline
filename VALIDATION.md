# Validation — Lowline 0.2.0

## Local verification

Configured and compiled the native Linux standalone, VST3 and test executable in Release mode with GCC 13.3, JUCE 8.0.6 and the pinned NAM core. The expanded regression suite exercises actual processor and model loading paths, not a mock audio engine.

New checks cover:

- Tuner fundamentals from low B (30.8677 Hz) through 440 Hz with added harmonics, within four cents on these synthetic fixtures; silence and DC rejection.
- Noise suppression below the gate threshold and reopening on a played-level signal.
- Gain response of all five EQ bands and low-frequency retention / high-frequency rejection of the clean low-pass.
- Dotted/half-note delay calculations, actual host tempo following and manual tempo sync.
- MIDI learn, replacement of previous mappings, expression control, save/restore and Program Change recall of user presets without losing the current bank.
- Portable export containing models and cabinet/room IRs, then import after deleting the source files; restored NAM processing, and rejection of an invalid embedded asset type before changing controls.
- Tuner output mute; NAM loudness metadata compensation; neutral defaults when older presets omit new parameters.
- Known-gain NAM replacements with a maximum per-sample transition step below 0.001 on the constant-input fixture; concurrent rapid loading while audio processing continues, reaching the last requested model.
- Rejection/bypass of an incompatible fixed-rate capture after device configuration changes. Linear NAM supports its own rate conversion, so the fixed-rate case uses NAM's base DSP contract rather than falsely treating Linear as fixed-rate.
- Native editor construction, twelve indexed help topics in each language, UTF-8 accents and language switching. RIG, TOOLS, MIDI and help snapshots are rendered from JUCE and visually inspected.

Existing regression checks remain: compression, ping-pong repeats, disabled-delay input rejection, clean path and polarity, real NAM loading against a Linear gain fixture, cabinet convolution, invalid-load preservation, state roundtrip, oversized audio blocks, nonfinite input recovery, factory recall, stale-model clearing, missing-asset errors, mono folding of stereo IRs, limiter peak containment/stereo balance and exact 1 ms reported latency including host bypass.

## Cross-platform builds

The Native builds workflow compiles and runs the suite on Linux x64, Windows x64 and macOS. Mac binaries contain arm64 and x86_64 slices; tests execute on the runner's architecture. The workflow checks the standalone universal architectures before packaging Standalone, VST3 and AU.

Verified source commit: `9efdb3be8156738f816bcf9968ae0ffc5d46eafc`. [Native builds run 39152945390](https://github.com/MassimoDanieli/lowline/actions/runs/39152945390) completed successfully on all three platforms: macOS universal, Windows x64 and Linux x64. Each job compiled and ran the regression suite. Direct package links are in README; CI artifacts also include regression logs and native UI previews.

## Practical limits

These are software and synthetic-signal checks. They do not establish Scarlett round-trip latency, real-time performance with arbitrary community captures, tuner accuracy on every bass/pickup/playing style, DAW plugin validation or subjective sound quality. No Apple Developer-ID signing or notarisation has been performed.

A 50 ms capture crossfade temporarily processes two engines per transitioning slot. It removes an abrupt pointer swap but does not guarantee an inaudible change for phase-mismatched models or an overloaded CPU. Full preset recall applies parameters and prepares slots sequentially; it is not an atomic scene switch and does not preserve long IR tails beyond the fade. Rapid pending capture requests may be coalesced to the latest prepared engine.

NAM level compensation uses valid loudness metadata and a bounded gain, not runtime LUFS measurement or physical input calibration. The clean crossover is a low-pass on the clean branch, with the processed branch remaining full-range. MIDI is applied at block boundaries and expression-pedal calibration remains external. CPU is smoothed callback time relative to the block budget, not total system utilisation.

Portable presets include loaded slot assets, but not other preset files referenced by Program Change assignments. Imported assets are stored in a persistent per-user cache; ordinary subsequent saves refer to those paths.

## Reproduce

Build with CMake, then run:

```sh
ctest --test-dir build -C Release --output-on-failure
build/LowlineTests_artefacts/Release/LowlineTests --screenshot
```

On Windows append `.exe`; headless Linux may need `xvfb-run -a`. Screenshots are written into the working directory. Source and dependency provenance are in THIRD_PARTY_NOTICES.md.
