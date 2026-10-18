# Lowline 0.1.1 — Bass NAM + IR rig

A native C++ audio application and plugin project for Massimo Danieli.
See `CHANGELOG.md` for the 0.1.1 maintenance fixes.

Original interface and signal-chain implementation, inspired by the idea of a compact amp-and-pedals workspace. Not affiliated with NamRay, Neural Amp Modeler or TONE3000.

**Development version 0.1.1.** Ready-to-run packages are published in [GitHub Actions](https://github.com/MassimoDanieli/lowline/actions/runs/39129426200): Mac universal (Apple Silicon + Intel), Windows x64 and Linux x64. The source ZIP itself contains no binaries. To compile locally, use the included scripts. Build and software tests have run on all three platforms; see `VALIDATION.md` for the limits of that verification.

![Lowline interface](preview.png)

## User guide / Guida utente

Click **GUIDA / HELP** in the app or plugin for the built-in offline guide: twelve topics, an index and Italian/English switching. Closing the guide leaves the rig running.

Read it online: [Italiano](docs/HELP_IT.md) · [English](docs/HELP_EN.md).

## What it does

- Standalone app, VST3 plugin on macOS/Windows/Linux; AU on macOS.
- Instrument input selection: input 1, input 2 or stereo sum.
- Soft-knee compressor: threshold, ratio, attack, release, manual makeup; gain-reduction meter.
- Simple saturating booster; separate mono NAM pedal and amp loaders, including the pinned core's A2 engine.
- Three-band post-amp EQ and stereo cabinet IR loader.
- Parallel clean/amp blend **after compressor, before delay and room**. At 0% the signal is clean; at 100% it is the processed amp/cab path.
- Clean polarity inversion and 0–20 ms manual alignment.
- Tape-style low-pass feedback delay, with real alternating stereo ping-pong.
- Stereo convolution room/reverb, from your WAV/AIFF IR.
- Four starting presets, user preset save/open, host state persistence.
- MIDI program changes and fixed CC mappings (below).
- Italian/English interface guidance, smoothed level/blend controls, input/output/GR meters and lookahead output limiter.
- Background model/IR loading; malformed loads preserve the previous working slot.

No captured amp sounds or third-party cabinet/reverb files are bundled. With no captures loaded, the compressor, booster, EQ and delay still work. Start with the Direct / DI preset, or turn the blend up to hear the booster/EQ.

## Mac — quick build

1. Install Apple's command line tools if needed: `xcode-select --install`.
2. Install [CMake](https://cmake.org/download/) and make it available in Terminal, or use `brew install cmake` if you already use Homebrew.
3. Unzip the project. In Terminal, go to the `lowline` directory and run:

   ```sh
   bash scripts/build-mac.command
   ```

The script builds for the Mac you run it on (Apple Silicon or Intel), runs the audio tests and opens `build/Lowline_artefacts/Release`.

- Open `Standalone/Lowline.app` to use it without a DAW or Logic licence.
- To use in GarageBand or another AU host, copy `AU/Lowline.component` to `~/Library/Audio/Plug-Ins/Components/` and restart the host.
- VST3 goes into `~/Library/Audio/Plug-Ins/VST3/`.

This is a local development build, not an Apple-notarised installer. macOS may ask you to allow the app and microphone access. The build workflow does not sign or notarise release installers.

### Scarlett 2i2 setup

Connect bass → Scarlett instrument input; enable INST and select the matching input in Lowline. In the standalone window's Options/Audio settings select Scarlett for input and output. If the JUCE standalone input is muted by default, disable “Mute audio input”. Listen through the Scarlett headphones or outputs; turn Scarlett direct monitoring off to hear only the software mix. Begin at 48 kHz, 128 or 256 samples, headphones low. A NAM model trained at another rate may require that rate; the loader tells you.

Set hardware gain so normal playing stays below clipping, then adjust Lowline input for the capture. Captures vary in calibration and level: input gain changes how hard they are driven. Lowline does not automatically apply model loudness/input calibration metadata.

## Download builds from GitHub Actions

Open the repository's **Actions** tab, select a successful **Native builds** run, and download its **Lowline-macOS-universal**, **Lowline-Windows-x64** or **Lowline-Linux-x64** artifact. Direct packages for the verified 0.1.1 build: [Mac universal](https://github.com/MassimoDanieli/lowline/actions/runs/39129426200/artifacts/12138015372), [Windows x64](https://github.com/MassimoDanieli/lowline/actions/runs/39129426200/artifacts/12137069663), [Linux x64](https://github.com/MassimoDanieli/lowline/actions/runs/39129426200/artifacts/12137571185).

The outer artifact ZIP contains a platform archive; unpack that archive to find the standalone app and plugins. The Mac archive preserves bundle permissions and is checked for both arm64 and x86_64. It is not Developer-ID signed or notarised. A successful CI test is not a Scarlett or listening test. Artifacts are retained for 30 days.

## Other platforms

Windows: Visual Studio 2022 with Desktop C++ workload and CMake.

```sh
cmake -S . -B build
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Linux: CMake, C++20 compiler, pkg-config and development packages for ALSA, X11, Xext, Xinerama, Xrandr, Xcursor, FreeType, Fontconfig and OpenGL. See `.github/workflows/build.yml` for Ubuntu package names. Then:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

For a headless Linux test runner, use `xvfb-run -a ctest --test-dir build --output-on-failure`.

The downloadable source ZIP includes JUCE and NAM under `third_party` for offline configuration. Git excludes this directory; repository builds fetch the exact pinned dependencies automatically. NAM is pinned to commit `0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842`; JUCE to 8.0.6.

## Controls and routing

Input selection and gain feed the compressor. The signal then splits into the clean branch (alignment and polarity) and the amp branch (booster, NAM pedal, NAM amp, EQ and cab). Blend combines them before stereo delay, room IR and output gain.

The source is treated as a mono instrument; stereo is created by the cabinet IR and effects. A stereo-sum source does not preserve the original stereo field. The “Bypass” switch skips the rig but retains input/output gain, mono input selection and the final safety ceiling; it is not bit-transparent host bypass.

Cab IRs: mono/stereo WAV or AIFF, up to 2 seconds. Room IRs: mono/stereo, up to 20 seconds. JUCE normalises IR level; trim is disabled so captured timing is preserved. Manual clean alignment can compensate for delay contained in a capture or IR. There is no automatic capture phase/delay measurement.

Models: mono input/output `.nam` only. The pinned NAM core supports A2; compatibility and real-time CPU performance for arbitrary community models are not guaranteed. Neural models that do not support arbitrary sample rates are rejected at mismatched rates instead of being played at the wrong speed. Changing device/host rate may require reloading the model.

A stereo-linked peak limiter uses 1 ms lookahead and an 80 ms release, targeting ±0.98. The added latency is reported to the host and retained on host bypass. It is a sample-peak limiter, not a true-peak limiter. A final clamp remains as a safeguard. If LIMIT lights, reduce output or upstream levels. The booster is simple tanh saturation at the host rate, with no oversampling; high-gain sounds can alias. The delay is an approximate tape-inspired effect, not a physical tape model.

### MIDI

Select the MIDI controller in standalone Audio/MIDI settings, or route MIDI to the plugin in your host. Any MIDI channel is accepted. Events are applied at block boundaries, not sample-accurately.

| Message | Action |
| --- | --- |
| PC 0–3 | Clean foundation, Warm edge, Wide ambience, Direct / DI |
| CC 20–27 | Compressor, booster, NAM pedal, NAM amp, cab, delay, reverb, bypass |
| CC 11 | Clean/amp blend, 0–100% |
| CC 7 | Output level, −36 to +6 dB |

For CC 20–27, values 0–63 mean off, 64–127 mean on. Use a footswitch that sends alternating 0/127 values. There is no MIDI learn, tap tempo or expression-pedal calibration yet.

### Presets and files

`.lowline` presets and DAW sessions retain control values and **absolute paths** to models/IRs, not copies of those assets. Keep files in a stable folder. Move a preset to another machine only with its assets, then reload those files. The four built-in starting presets change selected controls and do not load captures.

Load NAM/IR using the slot buttons. Drop a `.nam` onto the window to load the amp; drop an IR above the effects row for the cab or onto the lower row for the room; use the pedal button for a pedal capture. Open `.lowline` files with OPEN or drag and drop.

### TONE3000

The button opens the real site in your browser; download the `.nam`/IR and load it into the correct slot. Embedded search/download, API credentials and automatic level matching are **not implemented**. No account credentials are stored by Lowline.

## Development limitations

Audio processing never waits for the loader's model swap. In the rare case the swap lock overlaps an audio callback, that block takes a safe direct fallback. Loading/replacing captures may cause a discontinuity; switch between phrases. This prototype does not yet provide seamless scenes, tail-preserving preset changes, CPU metering or automatic model resampling. Hardware latency, dropout behaviour and AU validation must be checked on the target system.

## License

Original Lowline code: AGPL-3.0-only; see LICENSE. JUCE is used under AGPLv3, NAM core under MIT, Eigen and other bundled dependencies retain their own licences. See `THIRD_PARTY_NOTICES.md`. No licence or redistribution rights are granted to community captures you download separately.
