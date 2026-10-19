# 0.3.0

- Add searchable personal NAM/IR/preset library, favourites, recents and tags.
- Add temporary A/B rig snapshots and measured RMS level matching.
- Add backing-track player, loops, offline WSOLA speed adjustment and accented metronome.
- Add dual DI/FX 32-bit float WAV recording with background disk writers.
- Add preset name/tags/description, dirty state and parameter/metadata undo/redo.
- Add stage view, keyboard shortcuts, grouped capture preparation and room-tail carry-over.
- Add audio setup checklist, diagnostic copy and on-demand release link.
- Package a macOS DMG and per-user installation helper; prepare optional developer signing script.
- Extend both offline guides to eighteen topics and add native workspace previews/regression checks.

# Lowline 0.2.0

- Added tuner with mute and A4 calibration, gate, five-band EQ and clean low-pass.
- Added NAM loudness-metadata normalisation, tempo-sync delay, tap tempo, low cut and audio CPU timing.
- Added persistent MIDI learn, user preset Program Change assignments and portable preset export.
- Replaced capture swaps with prepared-engine mailboxes and 50 ms crossfades; retired engines are deleted off the audio thread.
- Added TOOLS and MIDI pages; updated both offline guides and the Italian announcement.
- Older presets retain parameter IDs and receive neutral defaults for added controls.

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

### Guida bilingue integrata

- Pulsante GUIDA / HELP nell'app e nei plugin, con finestra ridimensionabile e indice di dodici argomenti.
- Manuali offline in italiano e inglese: prima configurazione, controlli, NAM/IR, preset, MIDI, plugin e risoluzione dei problemi.
- I manuali online in docs/ sono anche le risorse incorporate nell'app.
