# Lowline 0.1.1

Correctness and stability update; not feature parity with Profondo.

- Replaced the waveform hard clip as the normal overload handler with a stereo-linked peak limiter using 1 ms lookahead and 80 ms release. Reports its latency to the host. The final clamp remains only a safeguard, not the normal gain-control algorithm. This is a sample-peak limiter, not an oversampled true-peak limiter.
- Preserved declared latency during host bypass; internal bypass also passes through the limiter.
- Fixed removed models reappearing after a device/sample-rate reprepare. Models and IR processors are now retired on the loading worker when cleared.
- Disabled delay no longer captures fresh input that could reappear when the effect is switched on.
- Factory program recall resets sound settings consistently while preserving input/output calibration and channel selection. Out-of-range Program Change messages are ignored instead of wrapping modulo four.
- Rejected foreign XML preset roots instead of reporting a successful restore.
- State restoration clears previous assets before loading the requested files, so a missing asset cannot leave the last preset's amplifier active. Batched restore errors remain visible after later successful clears/loads.
- Prevented preset saving while files are still loading, to avoid saving an incomplete file selection.
- Made language state atomic and refreshed editor text after a host/session language restore.
- Folded stereo effects down when processing a genuinely mono output, instead of silently losing the right side.
- Increased the declared maximum tail from 30 to 90 seconds to cover long feedback plus room IR combinations.

## Still intentionally absent

MIDI learn, user-preset banks selected by Program Change, portable embedded presets, tuner, gate, crossover, tempo-sync, factory cabinet/room IRs, NAM metadata normalisation and CPU metering have not been added in this maintenance update. Presets still refer to absolute asset paths. There is no claim of Mac/Windows or real hardware verification.

Loading uses a short protected object swap with a nonblocking direct fallback if it overlaps the audio callback; capture replacement is still not guaranteed seamless. The fallback now also goes through the same limiter. Booster oversampling and automatic sample-rate conversion remain future work.

### Interfaccia: smeraldo e ambra

- Sfondo antracite, accenti smeraldo e ambra piu vividi, testi piu leggibili.
- Maggiore contrasto tra pulsanti attivi, spenti e al passaggio del mouse.
