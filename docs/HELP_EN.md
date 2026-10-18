# Lowline 0.2 — User guide

## 01 · Start here
Lowline is a bass rig combining a gate, compressor, booster, two NAM models, cabinet impulse response (IR), EQ, parallel clean signal, stereo delay and IR reverb. It runs as a standalone app or VST3 plugin; AU is also available on Mac.

For your first session:
1. Connect your bass to the Scarlett, enable INST on that input and turn down your headphone volume.
2. In the standalone app open Options / Audio settings and choose Scarlett for both input and output. If “Mute audio input” is shown, disable it to hear your instrument.
3. Select Input 1 / Left or Input 2 / Right in Lowline to match the socket you use.
4. Start at 48 kHz with a 128- or 256-sample buffer. If your NAM model requires another sample rate, select that rate and reload it.
5. Turn off Scarlett direct monitoring to hear only the processed signal.
6. Select Clean foundation. You can play without loading any models: empty slots pass the signal through.
7. Load an amp .nam model and, if needed, a cabinet IR. Enable AMP ON and CAB ON, then adjust AMP / CLEAN and OUTPUT.

Green means a button is active; a dark button is off. The word “ON” is a fixed label: use the colour to identify the state. An active button does not mean a file has been loaded into that slot.

The RIG, TOOLS and MIDI tabs separate the signal chain, sound tools and controller setup. TOOLS contains the tuner, gate, EQ, clean filter, normalisation and delay timing.

Open HELP from the main screen. Choose a topic from the index; ENGLISH / ITALIANO changes this guide's language. The guide is built into the app and works offline.

## 02 · Signal flow and interface
The signal path is:
Selected input → INPUT → gate → compressor → split into two branches.

Processed branch: BOOST → NAM pedal → NAM amp → EQ → cabinet IR.
Clean branch: optional CLEAN LP filter → CLEAN ALIGN → CLEAN PHASE.

AMP / CLEAN combines the branches; next come stereo delay → IR reverb → OUTPUT → final limiter.

The clean branch starts AFTER the gate and compressor. “Clean” means without booster, NAM, EQ or cabinet, but it can still be compressed. Delay and reverb process the combined signal.

Input 1 / Left uses the first channel; Input 2 / Right uses the second when available. Stereo > Mono averages the two inputs: it does not preserve a stereo source. The rig starts with a mono instrument; stereo effects and IRs can create width at the output.

Drag a knob to change it; double-click to restore the parameter's default value. You can also edit the number below a knob and press Enter.

SAVE and OPEN manage .lowline presets. EXPORT creates a portable preset containing the loaded models and IRs. The upper-right menu offers four factory starting configurations. IT / EN on the main screen changes the interface language; technical control names remain in English.

## 03 · Levels, CPU and tuner
INPUT: −36 to +24 dB. Adjusts the signal before the compressor and models. Increasing it can drive a NAM capture harder; it does more than increase final loudness.

OUTPUT: −36 to +12 dB. Adjusts level after the effects and before the limiter. Start at −6 dB and compare sounds at similar loudness.

Set the Scarlett's physical gain first: hard playing should not clip its input. Turning down Lowline OUTPUT cannot repair distortion already caused at the audio interface input.

IN and OUT show audio-block peaks in dBFS, not perceived loudness. IN refers to the incoming signal before software input gain. Values near −180 dB indicate numerical silence. GR shows compressor gain reduction in dB.

LIMIT means the output limiter is reducing level. Lower OUTPUT, MAKEUP or upstream levels if it activates frequently. The limiter links both stereo channels, adds approximately 1 ms of lookahead and targets peaks of ±0.98. It is not a true-peak limiter and does not replace sensible gain settings.

BYPASS skips the rig chain while retaining input selection, INPUT, OUTPUT and final protection. It does not automatically match comparison levels. The host's plugin bypass preserves the declared latency.

CPU shows smoothed audio-callback processing time as a percentage of the time available for each block. It is not total system CPU usage: spikes near or above 100% can cause drop-outs. Capture changes temporarily process two engines.

TUNER (TOOLS tab): enable TUNER and play one string at a time. It displays note, octave, frequency and cents deviation; ±3 cents is highlighted green. A4 REFERENCE adjusts concert pitch from 430 to 450 Hz (default 440). Fundamental detection covers 25–500 Hz, including a five-string bass's low B. Detection is monophonic: strong harmonics, noise and chords may confuse it. Allow a fraction of a second for the note to settle.

MUTE gradually silences the output only while TUNER is enabled. Switching TUNER off restores sound even if MUTE remains selected. The tuner reads the input before the gate and effects; pitch analysis runs on a separate thread.

## 04 · Gate and compressor: a more even bass sound
GATE ON in TOOLS attenuates background noise during pauses, before the compressor. GATE THRESHOLD (−80…−10 dB) sets the opening threshold; GATE RELEASE (20…1000 ms) controls gradual closure. Opening takes approximately 2 ms, with a 40 ms hold and 3 dB hysteresis to avoid chatter near the threshold.

Start at −55 dB and 150 ms. Stop playing and raise the threshold gradually until noise is reduced; then check that quiet notes and sustain are preserved. The gate defaults to off for compatibility with older presets.

COMP ON enables the compressor before the clean/processed split.

THRESHOLD (−60…0 dB): the level above which compression increases. Lower it to compress more notes.
RATIO (1:1…12:1): compression strength; 1:1 applies no dynamic reduction, while 4:1 is an audible but manageable starting point.
ATTACK (0.5…100 ms): response to the start of a note. Shorter settings control the transient; longer settings let more pluck through.
RELEASE (10…800 ms): recovery after a note. Too short can sound unstable; too long can suppress subsequent notes.
MAKEUP (0…24 dB): restores level after compression. It is not automatic and can drive the output limiter.

Try RATIO 4:1, ATTACK 15 ms and RELEASE 120 ms. Play normally and adjust THRESHOLD until GR shows a few dB on stronger notes. Compare on/off, adjusting MAKEUP so that you do not simply prefer whichever sound is louder.

## 05 · Booster, NAM models and cabinet
BOOST ON enables simple saturation before the models. BOOST (0…24 dB) increases drive and distortion; it is not a transparent volume control. Start low, especially with an active bass.

The NAM slots run in series:
• PEDAL: a pedal capture, before the amp.
• AMP: an amplifier capture.

Click the .nam button on the appropriate row and choose a file. PEDAL ON / AMP ON enables or bypasses that model. The “x” removes the file from its slot; the ON button retains the file and changes only the effect state. An empty model slot passes the signal through without NAM processing.

CAB accepts a mono or stereo cabinet IR in WAV or AIFF format, up to 2 seconds long. Load it with the IR button and enable CAB ON. An IR reproduces acoustic filtering rather than an amplifier's dynamic saturation.

A capture may already include a cabinet and microphone. In that case try CAB off to avoid filtering the sound twice. Check the capture author's description.

Models must be mono-in/mono-out .nam files. The core supports A2, but compatibility and CPU demand depend on the capture. If the required sample rate differs from the device rate, follow the status message, change the rate and reload the model. NORMALISE in TOOLS enables level compensation based on the NAM model's loudness metadata. NAM TARGET sets the reference from −30 to −6 dB; correction is limited to ±18 dB per model and applied gradually. The default target is −18 dB.

PEDAL and AMP display the declared loudness; “no metadata” means the slot is empty or lacks a valid value. In that case model gain stays unchanged: Lowline does not invent a measurement or automatically analyse perceived loudness. PEDAL compensation also changes the signal entering AMP and can affect its distortion. Physical input calibration in dBu remains manual.

NAM/IR changes use a 50 ms crossfade between the previous engine and the new engine, prepared outside the audio callback. Rapid requests retain the most recent pending engine. CPU demand temporarily increases; a crossfade cannot guarantee that every combination of captures, phase and CPU load is inaudible. Long IR tails are not retained beyond the transition.

TONE3000 opens the website in your browser: download a file yourself, then load it into the correct slot. There is no integrated downloader. No models or IRs are included.

## 06 · EQ and parallel clean signal
EQ lives in TOOLS. Five bell filters provide −12 to +12 dB gain: BASS (100 Hz), LOW MID (default 250 Hz), MID (default 700 Hz), HIGH MID (default 1500 Hz) and TREBLE (3500 Hz).

LOW MID FREQ ranges from 80 to 800 Hz; MID FREQ from 150 to 2500 Hz; HIGH MID FREQ from 500 to 6000 Hz. Bands can overlap: make small changes and compare loudness. The two additional bands default to 0 dB, retaining older presets' original settings. EQ affects only the processed branch, before the cabinet IR.

AMP / CLEAN is the percentage of the processed branch:
• 0% = clean branch only.
• 100% = processed branch only.
• 70% = 70% processed and 30% clean.

AMP / CLEAN does not affect just the amp model: the processed branch includes booster, pedal, amp, EQ and cabinet. Other effects in that branch can work even without an amp capture.

CLEAN PHASE reverses the clean branch's polarity. If blending the branches makes the bass sound thin, compare both settings and keep the fuller one.

CLEAN ALIGN delays the clean signal by 0–20 ms to compensate for timing offsets in captures or IRs. Start at 0, temporarily set blend near 50% and increase alignment in small steps while listening to low notes. This is manual alignment, not automatic measurement; do not use it to compensate for the entire interface latency. CLEAN LP enables a 12 dB/octave low-pass on the clean branch only. CLEAN LOW PASS sets the cutoff from 50 to 1000 Hz (default 180). Use it to keep a clean low end while the processed branch supplies character and harmonics. The processed branch stays full-range: this is not a complementary frequency split. At 0% blend with CLEAN LP enabled you hear only filtered clean signal.

## 07 · Stereo delay
DELAY ON makes repeats audible and sends new notes into the delay.

TIME (30…1500 ms): spacing between repeats. Lower settings give short echoes; longer settings separate the notes clearly.
FEEDBACK (0…85%): how much echo returns to the delay. Higher values create more repeats; use restraint with busy parts.
HIGH CUT (800…10000 Hz): darkens the feedback path and therefore subsequent repeats. Lower settings soften the tail.
DELAY LEVEL (0…60%): amount of echo added to the direct signal; it is not a crossfade that removes the original signal.
PING PONG: alternates repeats between left and right. Use headphones or stereo output to hear it clearly.

Starting point: TIME 380 ms, FEEDBACK 27%, DELAY LEVEL 18%, HIGH CUT 3500 Hz.

SYNC in TOOLS calculates delay from TEMPO and a subdivision: 1/16, 1/8, dotted 1/8, 1/4, dotted 1/4 or 1/2. At 120 BPM a quarter note is 500 ms and a dotted eighth is 375 ms. TIME is ignored while SYNC is enabled; the effective delay appears below the selector.

HOST BPM uses DAW tempo when supplied and within 30–300 BPM. Standalone or an unavailable/invalid host tempo falls back to TEMPO (30–240 BPM). The DAW/Manual label identifies the source. TAP derives tempo from two presses 250–2000 ms apart: press regularly at least twice. Disable HOST BPM to use taps when the host is providing a valid tempo. Tap tempo is also available through MIDI learn.

DELAY LOW CUT (0…1000 Hz) reduces low frequencies in the repeats and feedback; 0 disables it. Try 100–200 Hz if repeats muddy the bass. HIGH CUT remains in RIG. Synchronised delay can exceed the manual TIME control's 1500 ms maximum.

Switching delay off stops new notes entering it and fades its output. It is not a guaranteed tail-preserving preset system: previous delay memory may remain if you turn it back on quickly.

## 08 · Convolution reverb
Load a room IR in CONVOLUTION ROOM, then enable REVERB ON. Mono/stereo WAV or AIFF files up to 20 seconds are accepted.

REVERB MIX (0…70%) blends the post-delay signal with the room response. Increasing it raises the reverberated contribution and lowers the direct contribution. Start at 8–15% to keep bass articulation clear.

Enabling REVERB ON without a loaded IR does not create a reverb. The file determines the character and tail length; there are no separate decay or room-size controls.

IR normalisation is handled by the convolution engine; this does not automatically match NAM model loudness. The IR's initial timing is preserved. Longer IRs may use more resources: try a shorter IR or a larger buffer if playback breaks up.

## 09 · Presets, files and drag-and-drop
SAVE creates a lightweight .lowline preset with controls, language, MIDI mappings, Program Change assignments and absolute NAM/IR file paths. DAW sessions retain the same state. OPEN reads both ordinary and portable presets.

EXPORT creates a portable .lowline containing copies of the models and IRs loaded in all four slots. Limits are 64 MB per asset and 192 MB total; missing or oversized assets cancel export. Wait for Loading to finish before saving. Embed and share only assets you have permission to redistribute.

On opening, embedded assets are extracted to a persistent Lowline/Assets cache in your user application-data folder. Original paths are no longer required. Subsequent ordinary saves refer to the cache: use EXPORT again to move them to another computer. Paths to other presets assigned to Program Changes are not recursively embedded; transfer those presets separately and reassign them if needed.

To preserve a sound, keep assets in a stable folder, save the preset and note which files it uses. To transfer an ordinary preset, copy and reload its assets too; alternatively use EXPORT to create a single portable file.

The “x” clears a slot. Errors can indicate a missing file, incompatible format or wrong sample rate: read the status bar at the bottom. While Loading is displayed, wait for completion before judging the sound. Capture changes use a 50 ms crossfade. A complete preset applies controls and loads slots sequentially: it is not an atomic scene switch with every tail preserved.

Drag-and-drop:
• Dropping a .nam anywhere in the window loads AMP; use PEDAL's own button for a pedal capture.
• Dropping an IR in the upper area loads CAB; the lower effects row loads ROOM.
• Dropping a .lowline opens that preset.

The four factory configurations are starting points: Clean foundation, Warm edge, Wide ambience and Direct / DI. They do not load assets, and retain input selection and INPUT/OUTPUT calibration. Direct / DI uses the clean branch but retains compression; Wide ambience needs an IR for its reverb. Recalling a configuration can overwrite sound settings: save your work first.

## 10 · MIDI and foot controllers
In the standalone app enable your controller in Audio/MIDI settings. For the plugin, route MIDI to Lowline in your DAW; the procedure depends on the host. Lowline accepts all MIDI channels.

Program Change 0…3 recalls the four factory presets in order. Some controllers display these as 1…4: check the actual transmitted message. In MIDI you can assign a .lowline file to any PC 0…127: choose the number, click ASSIGN PRESET and select the file. CLEAR PC removes an assignment. User assignments override factory presets; unassigned numbers above 3 are ignored. Files load on the worker, including the time needed to read and prepare assets.

SAVE retains the bank in the current preset/session. PC recalls preserve current bank assignments and MIDI learn, so a loaded preset does not erase your foot-controller setup. Manual OPEN restores the mappings saved in that file.

Control Change (CC):
20 = compressor
21 = booster
22 = NAM pedal
23 = NAM amp
24 = cabinet IR
25 = delay
26 = reverb
27 = bypass
11 = blend: 0 fully clean, 127 fully processed
7 = OUTPUT: 0 corresponds to −36 dB, 127 to +6 dB

For CC 20…27, values 0…63 switch off and 64…127 switch on. Configure a latching switch that alternates 0 and 127. A momentary switch may enable an effect only while held, depending on its programming.

MIDI LEARN: choose a parameter, click LEARN and move the controller. The next received CC is assigned to that parameter; its previous assignment is removed. CANCEL stops listening, CLEAR removes the selected parameter's assignment and RESET restores the initial map above. The list shows assigned CCs.

Continuous controls use the parameter's full range across values 0…127, so learned OUTPUT reaches +12 dB while default CC 7 keeps its original +6 dB ceiling. Buttons use 0…63 off and 64…127 on. Tap tempo triggers on a transition from low to high: the controller must also transmit release.

An expression pedal can control blend, output or other parameters. Calibrate its range on the controller: Lowline does not yet offer min/max calibration or custom curves. Assignments accept every channel; messages apply at audio-block boundaries rather than sample-accurately.

## 11 · Plugin use on Mac, Windows and Linux
Standalone is the independent app. VST3 is the plugin format for a compatible host; AU is the Mac format used by hosts such as GarageBand and Logic. Install only the formats you need.

On Mac the usual user folders are:
~/Library/Audio/Plug-Ins/Components/ for Lowline.component (AU)
~/Library/Audio/Plug-Ins/VST3/ for Lowline.vst3
Library is inside your Home folder; in Finder use Go → Go to Folder to reach it.

After copying the bundle, restart the host or rescan plugins. Insert Lowline on a bass audio track, choose the correct Scarlett input and enable track monitoring. When using the plugin, set audio device and buffer in the DAW. Avoid monitoring through the standalone app, direct monitoring and DAW simultaneously.

For ping-pong and stereo IRs use a stereo output. Mono output folds the channels together and loses stereo width.

On Windows use a VST3 host and the audio interface manufacturer's driver when available. On Linux use a compatible VST3 host and configure the audio system in the app or host.

The Mac package supports Apple Silicon and Intel but is not Developer-ID signed or notarised. If macOS blocks it, review the warning and Privacy & Security options only after verifying the download's origin.

## 12 · Troubleshooting and current limits
No sound: check cable and INST, selected input, input/output device, headphone level, app mute and track monitoring. Disable BYPASS to hear effects. For a basic check remove models, disable effects and EQ, and use the clean branch.

Doubled or hollow bass: disable direct monitoring and check that standalone and DAW are not both sounding. If it happens only when blending, try CLEAN PHASE and CLEAN ALIGN.

Distortion: first check Scarlett input clipping; then reduce INPUT, BOOST, MAKEUP and OUTPUT as appropriate to the stage that distorts. A NAM model may intentionally distort. LIMIT indicates output limiting rather than physical input clipping.

Crackles or excessive delay: increase buffer size to reduce interruptions; lower it for less latency if the computer can cope. Try lighter captures and shorter IRs. The limiter adds approximately 1 ms, on top of device, buffer and other plugin latency. Check CPU as well: it measures audio processing time against the block budget.

Changed preset sound or missing files: keep original paths or reload the assets and save again. SAVE retains paths; EXPORT embeds models and IRs.

NAM fails to load: read the status bar, check mono model compatibility and the required sample rate. AMP ON alone does not load an amplifier.

Current limits: NAM normalisation requires valid loudness metadata; input and expression-pedal calibration remain manual; no guarantee of atomic scene changes or complete tail preservation. The 50 ms crossfade avoids an abrupt engine swap but temporarily increases CPU demand. The tuner is monophonic. No models or IRs are supplied.

Automated builds check software using synthetic signals and test models. Latency, drop-outs, accuracy on real basses and listening results need verification with your Scarlett and host. Start with a 256-sample buffer and low volume; try 128 if the system remains stable.
