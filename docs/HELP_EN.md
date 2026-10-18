# Lowline — User guide

## 01 · Start here
Lowline is a bass rig combining a compressor, booster, two NAM models, cabinet impulse response (IR), EQ, parallel clean signal, stereo delay and IR reverb. It runs as a standalone app or VST3 plugin; AU is also available on Mac.

For your first session:
1. Connect your bass to the Scarlett, enable INST on that input and turn down your headphone volume.
2. In the standalone app open Options / Audio settings and choose Scarlett for both input and output. If “Mute audio input” is shown, disable it to hear your instrument.
3. Select Input 1 / Left or Input 2 / Right in Lowline to match the socket you use.
4. Start at 48 kHz with a 128- or 256-sample buffer. If your NAM model requires another sample rate, select that rate and reload it.
5. Turn off Scarlett direct monitoring to hear only the processed signal.
6. Select Clean foundation. You can play without loading any models: empty slots pass the signal through.
7. Load an amp .nam model and, if needed, a cabinet IR. Enable AMP ON and CAB ON, then adjust AMP / CLEAN and OUTPUT.

Green means a button is active; a dark button is off. The word “ON” is a fixed label: use the colour to identify the state. An active button does not mean a file has been loaded into that slot.

Open HELP from the main screen. Choose a topic from the index; IT / EN changes this guide's language. The guide is built into the app and works offline.

## 02 · Signal flow and interface
The signal path is:
Selected input → INPUT → compressor → split into two branches.

Processed branch: BOOST → NAM pedal → NAM amp → EQ → cabinet IR.
Clean branch: CLEAN ALIGN → CLEAN PHASE.

AMP / CLEAN combines the branches; next come stereo delay → IR reverb → OUTPUT → final limiter.

The clean branch starts AFTER the compressor. “Clean” means without booster, NAM, EQ or cabinet, but it can still be compressed. Delay and reverb process the combined signal.

Input 1 / Left uses the first channel; Input 2 / Right uses the second when available. Stereo > Mono averages the two inputs: it does not preserve a stereo source. The rig starts with a mono instrument; stereo effects and IRs can create width at the output.

Drag a knob to change it; double-click to restore the parameter's default value. You can also edit the number below a knob and press Enter.

SAVE and OPEN manage .lowline presets. The upper-right menu offers four factory starting configurations. IT / EN on the main screen changes the interface language; technical control names remain in English.

## 03 · Input, output and meters
INPUT: −36 to +24 dB. Adjusts the signal before the compressor and models. Increasing it can drive a NAM capture harder; it does more than increase final loudness.

OUTPUT: −36 to +12 dB. Adjusts level after the effects and before the limiter. Start at −6 dB and compare sounds at similar loudness.

Set the Scarlett's physical gain first: hard playing should not clip its input. Turning down Lowline OUTPUT cannot repair distortion already caused at the audio interface input.

IN and OUT show audio-block peaks in dBFS, not perceived loudness. IN refers to the incoming signal before software input gain. Values near −180 dB indicate numerical silence. GR shows compressor gain reduction in dB.

LIMIT means the output limiter is reducing level. Lower OUTPUT, MAKEUP or upstream levels if it activates frequently. The limiter links both stereo channels, adds approximately 1 ms of lookahead and targets peaks of ±0.98. It is not a true-peak limiter and does not replace sensible gain settings.

BYPASS skips the rig chain while retaining input selection, INPUT, OUTPUT and final protection. It does not automatically match comparison levels. The host's plugin bypass preserves the declared latency.

## 04 · Compressor: a more even bass sound
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

Models must be mono-in/mono-out .nam files. The core supports A2, but compatibility and CPU demand depend on the capture. If the required sample rate differs from the device rate, follow the status message, change the rate and reload the model. Lowline does not automatically normalise capture loudness.

TONE3000 opens the website in your browser: download a file yourself, then load it into the correct slot. There is no integrated downloader. No models or IRs are included.

## 06 · EQ and parallel clean signal
BASS / 100 Hz, MID / 700 Hz and TREBLE / 3.5 kHz control three bell filters from −12 to +12 dB. Their frequencies are fixed. EQ affects only the processed branch, before the cabinet IR.

AMP / CLEAN is the percentage of the processed branch:
• 0% = clean branch only.
• 100% = processed branch only.
• 70% = 70% processed and 30% clean.

AMP / CLEAN does not affect just the amp model: the processed branch includes booster, pedal, amp, EQ and cabinet. Other effects in that branch can work even without an amp capture.

CLEAN PHASE reverses the clean branch's polarity. If blending the branches makes the bass sound thin, compare both settings and keep the fuller one.

CLEAN ALIGN delays the clean signal by 0–20 ms to compensate for timing offsets in captures or IRs. Start at 0, temporarily set blend near 50% and increase alignment in small steps while listening to low notes. This is manual alignment, not automatic measurement; do not use it to compensate for the entire interface latency. There is no crossover separating low frequencies from the rest of the clean signal.

## 07 · Stereo delay
DELAY ON makes repeats audible and sends new notes into the delay.

TIME (30…1500 ms): spacing between repeats. Lower settings give short echoes; longer settings separate the notes clearly.
FEEDBACK (0…85%): how much echo returns to the delay. Higher values create more repeats; use restraint with busy parts.
HIGH CUT (800…10000 Hz): darkens the feedback path and therefore subsequent repeats. Lower settings soften the tail.
DELAY LEVEL (0…60%): amount of echo added to the direct signal; it is not a crossfade that removes the original signal.
PING PONG: alternates repeats between left and right. Use headphones or stereo output to hear it clearly.

Starting point: TIME 380 ms, FEEDBACK 27%, DELAY LEVEL 18%, HIGH CUT 3500 Hz.

The delay does not automatically synchronise to DAW tempo and has no tap tempo. For a quarter note use TIME = 60000 / BPM; at 120 BPM this is 500 ms. Halve that for an eighth note; use 75% of the quarter-note value for a dotted eighth.

Switching delay off stops new notes entering it and fades its output. It is not a guaranteed tail-preserving preset system: previous delay memory may remain if you turn it back on quickly.

## 08 · Convolution reverb
Load a room IR in CONVOLUTION ROOM, then enable REVERB ON. Mono/stereo WAV or AIFF files up to 20 seconds are accepted.

REVERB MIX (0…70%) blends the post-delay signal with the room response. Increasing it raises the reverberated contribution and lowers the direct contribution. Start at 8–15% to keep bass articulation clear.

Enabling REVERB ON without a loaded IR does not create a reverb. The file determines the character and tail length; there are no separate decay or room-size controls.

IR normalisation is handled by the convolution engine; this does not automatically match NAM model loudness. The IR's initial timing is preserved. Longer IRs may use more resources: try a shorter IR or a larger buffer if playback breaks up.

## 09 · Presets, files and drag-and-drop
SAVE creates a .lowline preset; OPEN recalls one. Presets and DAW sessions store controls, language and absolute NAM/IR file paths. They do not embed models or IRs.

To preserve a sound, keep assets in a stable folder, save the preset and note which files it uses. To transfer it to another computer, copy the assets too, reload them through the slots on the new system and save the preset again. Relocation is not automatic.

The “x” clears a slot. Errors can indicate a missing file, incompatible format or wrong sample rate: read the status bar at the bottom. While Loading is displayed, wait for completion before judging the sound. Replacing captures can cause a discontinuity: do it between phrases.

Drag-and-drop:
• Dropping a .nam anywhere in the window loads AMP; use PEDAL's own button for a pedal capture.
• Dropping an IR in the upper area loads CAB; the lower effects row loads ROOM.
• Dropping a .lowline opens that preset.

The four factory configurations are starting points: Clean foundation, Warm edge, Wide ambience and Direct / DI. They do not load assets, and retain input selection and INPUT/OUTPUT calibration. Direct / DI uses the clean branch but retains compression; Wide ambience needs an IR for its reverb. Recalling a configuration can overwrite sound settings: save your work first.

## 10 · MIDI and foot controllers
In the standalone app enable your controller in Audio/MIDI settings. For the plugin, route MIDI to Lowline in your DAW; the procedure depends on the host. Lowline accepts all MIDI channels.

Program Change 0…3 recalls the four factory presets in order. Some controllers display these as 1…4: check the actual transmitted message. It does not recall saved .lowline files; other program numbers are ignored.

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

An expression pedal transmitting CC 11 can control blend, but Lowline has no MIDI learn, remapping, pedal calibration or tap tempo. Messages are applied at audio-block boundaries rather than sample-accurately.

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

Crackles or excessive delay: increase buffer size to reduce interruptions; lower it for less latency if the computer can cope. Try lighter captures and shorter IRs. The limiter adds approximately 1 ms, on top of device, buffer and other plugin latency. Lowline has no CPU load display.

Changed preset sound or missing files: keep original paths or reload the assets and save again. No models are embedded in the preset.

NAM fails to load: read the status bar, check mono model compatibility and the required sample rate. AMP ON alone does not load an amplifier.

Not yet available: tuner, gate, model loudness normalisation, five-band EQ, clean crossover, delay sync, MIDI learn, presets with embedded assets and seamless capture changes. Use your host's tuner and tools when needed. Automated builds verify software, not real Scarlett latency or listening results.
