# Lowline 0.2.0 — Bass NAM + IR rig

A free, open-source native bass rig for macOS (Apple Silicon + Intel), Windows x64 and Linux x64. Standalone and VST3; AU is also built for macOS. Original project for Massimo Danieli, not affiliated with NamRay, Neural Amp Modeler or TONE3000.

![Lowline rig](preview.png)

## New in 0.2

- Monophonic bass tuner (25–500 Hz), A4 reference and smooth mute; pitch analysis runs off the audio thread.
- Noise gate with threshold, release, hysteresis and hold.
- Five-band EQ with three adjustable mid frequencies, plus optional 12 dB/octave low-pass on the parallel clean branch.
- Optional NAM level compensation from loudness metadata, bounded to ±18 dB per model. Models without valid metadata retain their gain.
- Delay sync to host or manual BPM, six subdivisions, tap tempo and low cut.
- MIDI learn for parameters and tap tempo, persistent mappings and PC 0–127 assignments to user presets.
- Portable .lowline export containing the loaded NAM/IR assets.
- 50 ms crossfades for NAM and IR replacement. Engines are prepared and retired off the audio thread.
- Audio processing-load indicator and dedicated RIG, TOOLS and MIDI pages.

The rig also includes compression, saturation booster, NAM pedal and amp slots, cabinet IR, clean blend with polarity/alignment, stereo ping-pong delay, convolution reverb and a stereo-linked lookahead limiter.

![Tone tools](docs/tools-preview.png)

![MIDI learn and preset bank](docs/midi-preview.png)

## Download and quick start

Verified **0.2.0** packages ([successful build and tests](https://github.com/MassimoDanieli/lowline/actions/runs/39152945390)):

- [Download macOS universal — Apple Silicon + Intel](https://github.com/MassimoDanieli/lowline/actions/runs/39152945390/artifacts/12146091435)
- [Download Windows x64](https://github.com/MassimoDanieli/lowline/actions/runs/39152945390/artifacts/12146150976)
- [Download Linux x64](https://github.com/MassimoDanieli/lowline/actions/runs/39152945390/artifacts/12145338897)

GitHub sign-in may be required to download artifacts.

Native packages are built in [GitHub Actions](https://github.com/MassimoDanieli/lowline/actions). Choose a successful **Native builds** run and download **Lowline-macOS-universal**, **Lowline-Windows-x64** or **Lowline-Linux-x64** under Artifacts. Unpack the outer artifact ZIP, then the platform archive inside. Artifacts expire after 30 days.

On Mac, run the standalone app or install the AU/VST3 bundle in your user's Audio/Plug-Ins folders. The app is not Developer-ID signed or notarised. See the guide for installation and audio setup.

For Scarlett 2i2: enable INST, select the matching input and Scarlett output, disable direct monitoring, and start at 48 kHz / 128 or 256 samples. If the standalone app has “Mute audio input” enabled, disable it. Set the interface's physical gain without clipping, then adjust software INPUT and OUTPUT. A capture may require a different sample rate; follow its status message.

No NAM captures or IRs are bundled. TONE3000 opens the website; download compatible assets and load them into the appropriate slots.

## Guides / Guide

Click **GUIDA / HELP** for the built-in offline manual, with twelve indexed topics and language switching. The same files are published here:

- [Italiano — guida completa](docs/HELP_IT.md)
- [English — complete guide](docs/HELP_EN.md)
- [Validation and limitations](VALIDATION.md)
- [Changelog](CHANGELOG.md)

![Offline guide](docs/help-preview.png)

## Signal flow

Selected mono input → input gain → gate → compressor → split.

Processed branch: booster → NAM pedal → NAM amp → five-band EQ → cabinet IR.
Clean branch: optional low-pass → manual alignment → polarity.

Blend → stereo delay → convolution reverb → output gain → 1 ms lookahead limiter → tuner mute.

The clean branch includes gate/compression. Its low-pass does not high-pass the processed branch. Stereo input is summed or one channel is selected; stereo width comes from IRs and effects. Mono output folds both output channels together.

The limiter targets sample peaks at ±0.98, is stereo-linked and reports its latency to the host. It is not a true-peak limiter. INPUT changes how hard models are driven; OUTPUT changes final level. NAM normalisation is based on declared metadata, not measured LUFS matching or physical input calibration. Normalising a pedal also changes the drive into the amp.

## Presets and MIDI

**SAVE** stores controls, MIDI maps, PC assignments and absolute asset paths. **EXPORT** embeds the four slots' assets (64 MB per file / 192 MB total). **OPEN** accepts either format. Portable assets are extracted to a persistent user-data cache; subsequent ordinary saves reference that cache. Export again to transfer them. Only redistribute assets for which you have permission. PC-assigned preset files are separate and are not recursively embedded.

MIDI learn: select a target on MIDI, click LEARN, move the controller. CLEAR removes that target's assignment; RESET restores defaults. Values 0–63 disable toggles and 64–127 enable them. Continuous controls span their full parameter range. Tap reacts to a rising edge; transmit release too. All MIDI channels are accepted.

Default CCs: 20 compressor, 21 booster, 22 pedal, 23 amp, 24 cab, 25 delay, 26 reverb, 27 bypass, 11 blend, 7 output (−36 to +6 dB). Learned output uses the full −36 to +12 dB range. Calibrate expression pedals on the controller.

PC 0–3 recalls factory starting configurations unless a user file is assigned. Any PC 0–127 can recall an assigned .lowline file on the background worker. These recalls retain the current MIDI map/bank; manual OPEN restores the saved map/bank. SAVE the bank in a preset or host session to retain it.

Older presets load with neutral defaults for the new effects. Keep backups when moving between versions; 0.2 exports are not designed for older versions.

## Build

CMake 3.24+, C++20 compiler. Dependencies are pinned: JUCE 8.0.6 and NAM core `0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842` (A2 support). Repository builds fetch them; a local `third_party` checkout can be used offline.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Mac universal: add `'-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'` at configuration. Windows: Visual Studio 2022 Desktop C++ and CMake. Linux: development packages for ALSA, X11/Xext/Xinerama/Xrandr/Xcursor, FreeType, Fontconfig and OpenGL; see the workflow. Headless tests can use Xvfb.

## Practical limits

Crossfades replace abrupt engine swaps, but temporarily run two engines per transitioning slot. Phase differences and CPU overruns can still be audible. Complete preset recall applies controls and loads slots sequentially; it is not an atomic scene switch, and long IR tails are not retained past the fade. There is no automatic model sample-rate conversion.

The tuner is monophonic and can be confused by strong harmonics/noise. Gate, EQ and level settings require adjustment for the instrument. The booster is a simple host-rate saturation and can alias at high drive. The CPU display is callback time relative to its audio deadline, not total system utilisation.

Software tests do not establish Scarlett round-trip latency, performance with arbitrary neural captures, DAW validation or real-world listening quality. See VALIDATION.md for what was actually checked.

## License

Original Lowline code: AGPL-3.0-only. JUCE is used under AGPLv3; NAM core is MIT; dependencies retain their licences. See LICENSE and THIRD_PARTY_NOTICES.md. Community captures have their own redistribution terms.
