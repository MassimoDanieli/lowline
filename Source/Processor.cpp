#include "Processor.h"
#include "Editor.h"
using namespace juce;
AudioProcessorValueTreeState::ParameterLayout LowlineProcessor::parameters()
{
    AudioProcessorValueTreeState::ParameterLayout l;
    auto f = [&](const char *id, const char *name, float lo, float hi, float def, float step = .01f)
    {
        l.add(std::make_unique<AudioParameterFloat>(ParameterID{id, 1}, name,
                                                    NormalisableRange<float>{lo, hi, step}, def));
    };
    auto b = [&](const char *id, const char *name, bool def)
    { l.add(std::make_unique<AudioParameterBool>(ParameterID{id, 1}, name, def)); };
    f("input", "Input dB", -36, 24, 0);
    f("output", "Output dB", -36, 12, -6);
    l.add(std::make_unique<AudioParameterChoice>(ParameterID{"source", 1}, "Input channel",
                                                 StringArray{"Left / Mono", "Right", "Stereo sum"}, 0));
    b("compOn", "Compressor", true);
    f("threshold", "Threshold dB", -60, 0, -22);
    f("ratio", "Ratio", 1, 12, 4);
    f("attack", "Attack ms", .5, 100, 15);
    f("release", "Release ms", 10, 800, 120);
    f("makeup", "Makeup dB", 0, 24, 0);
    b("driveOn", "Booster", false);
    f("drive", "Drive", 0, 24, 6);
    b("pedalOn", "NAM pedal", true);
    b("ampOn", "NAM amp", true);
    b("cabOn", "Cab IR", true);
    f("bass", "Bass dB", -12, 12, 0);
    f("mid", "Mid dB", -12, 12, 0);
    f("treble", "Treble dB", -12, 12, 0);
    f("blend", "Amp blend %", 0, 100, 70);
    b("invert", "Clean polarity", false);
    f("align", "Clean delay ms", 0, 20, 0);
    b("delayOn", "Delay", false);
    f("time", "Delay ms", 30, 1500, 380, 1);
    f("feedback", "Feedback %", 0, 85, 27);
    f("delayMix", "Delay level %", 0, 60, 18);
    f("highcut", "Delay high cut Hz", 800, 10000, 3500, 1);
    b("ping", "Ping pong", true);
    b("reverbOn", "IR reverb", false);
    f("reverbMix", "Reverb mix %", 0, 70, 15);
    b("bypass", "Bypass", false);
    b("gateOn", "Noise gate", false);
    f("gateThreshold", "Gate threshold dB", -80, -10, -55);
    f("gateRelease", "Gate release ms", 20, 1000, 150);
    f("lowMid", "Low mid dB", -12, 12, 0);
    f("highMid", "High mid dB", -12, 12, 0);
    f("lowMidFreq", "Low mid Hz", 80, 800, 250, 1);
    f("midFreq", "Mid Hz", 150, 2500, 700, 1);
    f("highMidFreq", "High mid Hz", 500, 6000, 1500, 1);
    b("crossOn", "Clean low-pass", false);
    f("crossFreq", "Clean crossover Hz", 50, 1000, 180, 1);
    b("normOn", "NAM level normalisation", false);
    f("normTarget", "NAM target dB", -30, -6, -18);
    b("tunerOn", "Tuner", false);
    b("tunerMute", "Tuner mute", false);
    f("tunerRef", "Tuner A4 Hz", 430, 450, 440, .1f);
    b("sync", "Delay tempo sync", false);
    b("hostSync", "Use host tempo", true);
    f("tempo", "Tempo BPM", 30, 240, 120, .1f);
    l.add(std::make_unique<AudioParameterChoice>(ParameterID{"division", 1}, "Delay division",
        StringArray{"1/16", "1/8", "1/8 dotted", "1/4", "1/4 dotted", "1/2"}, 3));
    f("lowcut", "Delay low cut Hz", 0, 1000, 0, 1);
    return l;
}
LowlineProcessor::LowlineProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::stereo(), true)
                         .withOutput("Output", AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Lowline", parameters())
{
    for (auto *parameter : getParameters()) parameter->addListener(this);
    for (auto *parameter : getParameters())
        if (auto *ranged = dynamic_cast<RangedAudioParameter *>(parameter)) midiTargets.push_back(ranged);
    defaultMidiMappings();
    loadMidiSettings();
    worker = std::thread([this] { workerLoop(); });
    markClean();
}
LowlineProcessor::~LowlineProcessor()
{
    {
        std::lock_guard g(jobsMutex);
        stopping = true;
        jobs.clear();
    }
    jobsReady.notify_one();
    if (worker.joinable())
        worker.join();
    cancelPendingUpdate();
}
bool LowlineProcessor::isBusesLayoutSupported(const BusesLayout &l) const
{
    auto i = l.getMainInputChannelSet(), o = l.getMainOutputChannelSet();
    return (i == AudioChannelSet::mono() || i == AudioChannelSet::stereo()) &&
           (o == AudioChannelSet::stereo() || o == AudioChannelSet::mono());
}
void LowlineProcessor::prepareToPlay(double sr, int n)
{
    const SpinLock::ScopedLockType guard(modelLock);
    rate = sr;
    maxBlock = std::max(1, n);
    work.setSize(2, maxBlock);
    dry.setSize(2, maxBlock);
    raw.setSize(1, maxBlock);
    bypassMix.prepare(sr, p("bypass") > .5f ? 1.f : 0.f);
    driveMix.prepare(sr, p("driveOn") > .5f ? 1.f : 0.f);
    saturator.reset();
    limiter.prepare(sr);
    studio.prepare(sr, maxBlock);
    compareGain.prepare(sr, 1);
    compressor.prepare(sr);
    delay.prepare(sr);
    dryDelay.prepare(sr);
    for (auto &e : eq)
        e.reset();
    updateEq(true);
    inputGain.prepare(sr, lowline::db(p("input")));
    outputGain.prepare(sr, lowline::db(p("output")));
    blend.prepare(sr, p("blend") * .01f);
    drive.prepare(sr, lowline::db(p("drive")));
    delayMix.prepare(sr, p("delayOn") ? p("delayMix") * .01f : 0);
    reverbMix.prepare(sr, p("reverbOn") ? p("reverbMix") * .01f : 0);
    bool compatible = true;
    for (auto &capture : captures) compatible = capture.prepare(sr, maxBlock) && compatible;
    if (!compatible) setStatus("NAM bypassed: match the device sample rate to the model, then reload.");
    gate.prepare(sr);
    crossover.reset();
    crossMix.prepare(sr, p("crossOn"));
    muteGain.prepare(sr, p("tunerOn") && p("tunerMute") ? 0.f : 1.f);
    tuner.prepare(sr);
    setLatencySamples(limiter.delaySamples);
}
void LowlineProcessor::set(const char *id, float v)
{
    if (auto *x = state.getParameter(id))
        x->setValueNotifyingHost(x->convertTo0to1(v));
}
void LowlineProcessor::resetMidiMappings()
{
    defaultMidiMappings();
    saveMidiSettings();
}
void LowlineProcessor::defaultMidiMappings()
{
    for (auto &mapping : midiMap) mapping = -1;
    const char *ids[] = {"compOn", "driveOn", "pedalOn", "ampOn", "cabOn", "delayOn", "reverbOn", "bypass"};
    for (size_t i = 0; i < midiTargets.size(); ++i)
    {
        for (int cc = 0; cc < 8; ++cc)
            if (midiTargets[i]->paramID == ids[cc]) midiMap[size_t(20 + cc)] = int(i);
        if (midiTargets[i]->paramID == "blend") midiMap[11] = int(i);
    }
    midiMap[7] = -2; // Preserve the original CC7 range (-36 to +6 dB).
    learnTarget = -1;
}
void LowlineProcessor::clearMidiTarget(int target)
{
    for (auto &mapping : midiMap)
        if (mapping.load() == target || (mapping.load() == -2 && target >= 0 && target < int(midiTargets.size()) && midiTargets[size_t(target)]->paramID == "output")) mapping = -1;
    learnTarget = -1;
    saveMidiSettings();
}
File &LowlineProcessor::settingsOverride()
{
    static File override;
    return override;
}
File LowlineProcessor::midiSettingsFile()
{
    const auto folder = settingsOverride() != File{} ? settingsOverride()
        : File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Lowline");
    return folder.getChildFile("midi-controller.xml");
}
ValueTree LowlineProcessor::midiSettingsTree() const
{
    ValueTree root("LowlineMidi");
    ValueTree mappings("MidiMappings");
    for (int cc = 0; cc < 128; ++cc) mappings.setProperty("cc" + String(cc), midiMap[size_t(cc)].load(), nullptr);
    mappings.setProperty("momentary", momentarySwitches.load(), nullptr);
    root.addChild(mappings, -1, nullptr);
    ValueTree bank("ProgramFiles");
    {
        std::lock_guard guard(metadataMutex);
        for (int pc = 0; pc < 128; ++pc)
            if (programFiles[size_t(pc)].isNotEmpty()) bank.setProperty("pc" + String(pc), programFiles[size_t(pc)], nullptr);
    }
    root.addChild(bank, -1, nullptr);
    return root;
}
void LowlineProcessor::applyMidiSettings(const ValueTree &tree)
{
    auto mappings = tree.getChildWithName("MidiMappings");
    if (mappings.isValid())
    {
        for (int cc = 0; cc < 128; ++cc)
        {
            const int target = int(mappings.getProperty("cc" + String(cc), -1));
            midiMap[size_t(cc)] = target >= -2 && target <= int(midiTargets.size()) ? target : -1;
        }
        momentarySwitches = bool(mappings.getProperty("momentary", false));
    }
    auto bank = tree.getChildWithName("ProgramFiles");
    if (bank.isValid())
    {
        std::lock_guard guard(metadataMutex);
        for (int pc = 0; pc < 128; ++pc)
        {
            programFiles[size_t(pc)] = bank.getProperty("pc" + String(pc)).toString();
            programAssigned[size_t(pc)] = programFiles[size_t(pc)].isNotEmpty();
        }
    }
}
void LowlineProcessor::loadMidiSettings()
{
    // The MIDI map and the Program Change bank belong to the player's controller, not to a sound:
    // one file shared by the app and every plugin instance.
    const auto file = midiSettingsFile();
    hasMidiSettings = file.existsAsFile();
    if (!hasMidiSettings) return;
    if (auto xml = XmlDocument::parse(file); xml && xml->hasTagName("LowlineMidi"))
        applyMidiSettings(ValueTree::fromXml(*xml));
}
void LowlineProcessor::saveMidiSettings()
{
    if (!MessageManager::existsAndIsCurrentThread())
    {
        midiSettingsDirty = true;
        triggerAsyncUpdate();
        return;
    }
    midiSettingsDirty = false;
    const auto file = midiSettingsFile();
    if (file.getParentDirectory().createDirectory().failed()) return;
    if (auto xml = midiSettingsTree().createXml())
    {
        TemporaryFile temporary(file);
        if (xml->writeTo(temporary.getFile()) && temporary.overwriteTargetFileWithTemporary()) hasMidiSettings = true;
    }
}
void LowlineProcessor::setMomentarySwitches(bool on)
{
    momentarySwitches = on;
    saveMidiSettings();
}
StringArray LowlineProcessor::midiTargetNames() const
{
    StringArray names;
    for (auto *parameter : midiTargets) names.add(parameter->getName(80));
    names.add("Tap tempo");
    return names;
}
String LowlineProcessor::midiDescription() const
{
    String result;
    auto names = midiTargetNames();
    for (int cc = 0; cc < 128; ++cc)
    {
        const int target = midiMap[size_t(cc)];
        if (target == -2) result += "CC 7  -> Output (-36 / +6 dB)\n";
        else if (target >= 0 && target < names.size())
            result += "CC " + String(cc) + "  -> " + names[target] + "\n";
    }
    return result;
}
void LowlineProcessor::tapTempoAt(double now)
{
    float bpm = 0;
    {
        const SpinLock::ScopedLockType guard(tapLock);
        const double interval = now - lastTap.exchange(now);
        if (interval < 250 || interval > 2000) { tapCount = 0; return; }
        const int n = std::min(tapCount, int(tapIntervals.size()));
        double sum = 0;
        for (int i = 0; i < n; ++i) sum += tapIntervals[size_t(i)];
        // A tap far from the running average starts a new tempo instead of being averaged in.
        if (n > 0 && std::abs(interval - sum / n) > .3 * sum / n) tapCount = 0;
        tapIntervals[size_t(tapCount % int(tapIntervals.size()))] = interval;
        ++tapCount;
        const int used = std::min(tapCount, int(tapIntervals.size()));
        sum = 0;
        for (int i = 0; i < used; ++i) sum += tapIntervals[size_t(i)];
        bpm = float(60000.0 * used / sum);
    }
    set("tempo", bpm);
    // Without tempo sync TAP sets the delay time directly, like a delay pedal.
    if (p("sync") < .5f)
        set("time", std::clamp(lowline::tempoDelay(bpm, int(p("division"))), 30.f, 1500.f));
}
void LowlineProcessor::assignProgram(int pc, const File &file)
{
    if (pc < 0 || pc > 127) return;
    {
    std::lock_guard lock(metadataMutex);
    programFiles[size_t(pc)] = file == File{} ? String() : file.getFullPathName();
    programAssigned[size_t(pc)] = file != File{};
    }
    saveMidiSettings();
}
String LowlineProcessor::programAssignment(int pc) const
{
    std::lock_guard lock(metadataMutex);
    return pc >= 0 && pc < 128 ? programFiles[size_t(pc)] : String();
}
void LowlineProcessor::midi(const MidiBuffer &m)
{
    for (auto item : m)
    {
        auto msg = item.getMessage();
        if (msg.isProgramChange())
        {
            const int pc = msg.getProgramChangeNumber();
            if (programAssigned[size_t(pc)]) requestedProgram = pc;
            else setCurrentProgram(pc);
        }
        if (!msg.isController()) continue;
        const int cc = msg.getControllerNumber(), value = msg.getControllerValue();
        const int learning = learnTarget.exchange(-1);
        if (learning >= 0 && learning <= int(midiTargets.size()))
        {
            for (auto &mapping : midiMap)
                if (mapping.load() == learning || (mapping.load() == -2 && learning < int(midiTargets.size()) && midiTargets[size_t(learning)]->paramID == "output")) mapping = -1;
            midiMap[size_t(cc)] = learning;
            saveMidiSettings(); // deferred to the message thread
        }
        const int target = midiMap[size_t(cc)];
        const bool high = value >= 64;
        if (target == int(midiTargets.size()))
        {
            if (high && !ccHigh[size_t(cc)]) tapTempo();
        }
        else if (target == -2) set("output", -36 + float(value) * 42 / 127);
        else if (target >= 0 && target < int(midiTargets.size()))
        {
            auto *parameter = midiTargets[size_t(target)];
            if (momentarySwitches && dynamic_cast<AudioParameterBool *>(parameter) != nullptr)
            {
                if (high && !ccHigh[size_t(cc)]) parameter->setValueNotifyingHost(parameter->getValue() > .5f ? 0.f : 1.f);
            }
            else
                parameter->setValueNotifyingHost(float(value) / 127);
        }
        ccHigh[size_t(cc)] = high;
    }
}
void LowlineProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &m)
{
    ScopedNoDenormals noDenormals;
    midi(m);
    m.clear();
    float peak = 0;
    for (int c = 0; c < getTotalNumInputChannels(); ++c)
        peak = std::max(peak, buffer.getMagnitude(c, 0, buffer.getNumSamples()));
    inputPeak = peak;
    const double started = Time::getMillisecondCounterHiRes();
    if (measureAfterLoad && !busy) { measureAfterLoad = false; measureRequested = true; }
    if (measureRequested.exchange(false)) { measureStartVersion = stateVersion.load(); measureEnergy = 0; measureCount = 0; measureRemaining = int(rate.load()*5); measuredRms = -100; }
    float bpm = p("tempo");
    bool host = false;
    if (p("sync") && p("hostSync"))
        if (auto *playhead = getPlayHead())
            if (auto position = playhead->getPosition())
                if (auto tempo = position->getBpm(); tempo && std::isfinite(*tempo) && *tempo >= 30 && *tempo <= 300)
                { bpm = float(*tempo); host = true; }
    effectiveTempo = bpm;
    usingHostTempo = host;
    effectiveDelay = p("sync") ? lowline::tempoDelay(bpm, int(p("division"))) : p("time");
    for (int off = 0; off < buffer.getNumSamples(); off += maxBlock)
    {
        const int n = std::min(maxBlock.load(), buffer.getNumSamples() - off);
        studio.captureInput(buffer, off, n, int(p("source")), getTotalNumInputChannels());
        processChunk(buffer, off, n);
        for (int i = off; i < off+n; ++i)
        {
            const float gain = compareGain.next(lowline::db(compareTrim));
            if (measureRemaining > 0)
            {
                const float x = buffer.getSample(0,i);
                measureEnergy += x*x; ++measureCount;
                if (--measureRemaining == 0)
                {
                    measuredRms = lowline::toDb(float(std::sqrt(measureEnergy/std::max(1,measureCount)))); measureEnergy = 0; measureCount = 0;
                    // Valid only if nothing changed while measuring.
                    const bool fresh = stateVersion.load() == measureStartVersion.load();
                    measuredVersion = fresh ? measureStartVersion.load() : 0xffffffffu;
                    const int target = measureFor.exchange(-1);
                    if (fresh && target >= 0 && target < 2) comparisonRms[size_t(target)] = measuredRms.load();
                }
            }
            for (int c = 0; c < getTotalNumOutputChannels(); ++c) buffer.setSample(c,i,buffer.getSample(c,i)*gain);
        }
        studio.record(buffer, off, n, getTotalNumOutputChannels());
        studio.mix(buffer, off, n, getTotalNumOutputChannels());
    }
    limitOutput(buffer);
    const double budget = 1000.0 * buffer.getNumSamples() / rate.load();
    if (budget > 0) cpuLoad = .9f * cpuLoad.load() + .1f * float((Time::getMillisecondCounterHiRes() - started) / budget * 100);
}
void LowlineProcessor::processBlockBypassed(AudioBuffer<float> &buffer, MidiBuffer &messages)
{
    ScopedNoDenormals guard;
    messages.clear();
    if (getTotalNumOutputChannels() == 1 && getTotalNumInputChannels() > 1)
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buffer.setSample(0, i, .5f * (buffer.getSample(0, i) + buffer.getSample(1, i)));
    else if (getTotalNumInputChannels() == 1 && getTotalNumOutputChannels() > 1)
        buffer.copyFrom(1, 0, buffer, 0, 0, buffer.getNumSamples());
    // Preserve declared latency on host bypass, rather than moving the track by 1 ms.
    // Host bypass must never mute, even if the tuner mute was left on.
    limitOutput(buffer, false);
}
void LowlineProcessor::limitOutput(AudioBuffer<float> &buffer, bool allowMute)
{
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float l = buffer.getSample(0, i), r = getTotalNumOutputChannels() > 1 ? buffer.getSample(1, i) : l;
        limiter.tick(l, r);
        const float mute = muteGain.next(allowMute && p("tunerOn") && p("tunerMute") ? 0.f : 1.f);
        l *= mute; r *= mute;
        if (limiter.gain < .999f)
            clipped = true;
        buffer.setSample(0, i, lowline::safety(l));
        if (getTotalNumOutputChannels() > 1)
            buffer.setSample(1, i, lowline::safety(r));
    }
    outputPeak = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    if (getTotalNumOutputChannels() > 1)
        outputPeak = std::max(outputPeak.load(), buffer.getMagnitude(1, 0, buffer.getNumSamples()));
}
void LowlineProcessor::updateEq(bool snap)
{
    // Band gains and frequencies glide (about 20 ms) instead of stepping once per block.
    const float target[5][2] = {{p("bass"), 100}, {p("lowMid"), p("lowMidFreq")}, {p("mid"), p("midFreq")},
                                {p("highMid"), p("highMidFreq")}, {p("treble"), 3500}};
    const float k = snap ? 1.f : .2f; // called every 32 samples
    for (size_t b = 0; b < 5; ++b)
    {
        eqGainNow[b] += k * (target[b][0] - eqGainNow[b]);
        eqFreqNow[b] = eqFreqNow[b] <= 0 || snap ? target[b][1] : eqFreqNow[b] * std::pow(target[b][1] / eqFreqNow[b], k);
    }
    eq[0].lowShelf(rate, eqFreqNow[0], eqGainNow[0]);
    for (size_t b = 1; b < 4; ++b) eq[b].peak(rate, eqFreqNow[b], eqGainNow[b]);
    eq[4].highShelf(rate, eqFreqNow[4], eqGainNow[4]);
}
void LowlineProcessor::processChunk(AudioBuffer<float> &out, int off, int n)
{
    const float inTarget = lowline::db(p("input")), outTarget = lowline::db(p("output"));
    const bool bypass = p("bypass") > .5f;
    // The rig keeps running while the bypass crossfade (15 ms) is in progress; only a fully
    // bypassed rig skips the processing. Delay and reverb tails still fade out on bypass.
    const bool process = !bypass || bypassMix.value < .9999f;
    {
        const SpinLock::ScopedTryLockType publish(publicationLock);
        if (publish.isLocked()) for (auto &capture : captures) capture.beginChunk();
    }
    const int source = int(p("source")), inputs = getTotalNumInputChannels();
    for (int i = 0; i < n; ++i)
    {
        float x = out.getSample(source == 1 && inputs > 1 ? 1 : 0, off + i);
        if (source == 2 && inputs > 1)
            x = .5f * (out.getSample(0, off + i) + out.getSample(1, off + i));
        if (!std::isfinite(x))
            x = 0;
        if (p("tunerOn")) tuner.push(x);
        x *= inputGain.next(inTarget);
        raw.setSample(0, i, x);
        x = gate.tick(x, p("gateOn") && process, p("gateThreshold"), p("gateRelease"));
        if (p("compOn") > .5f && process)
            x = compressor.tick(x, p("threshold"), p("ratio"), p("attack"), p("release"), p("makeup"));
        dry.setSample(0, i, x);
        work.setSample(0, i, x);
    }
    gainReduction = p("compOn") > .5f && !bypass ? compressor.reduction : 0;
    if (process)
    {
        const float driveTarget = p("driveOn") > .5f ? 1.f : 0.f;
        if (driveTarget > 0 || driveMix.value > 1.e-4f)
            for (int i = 0; i < n; ++i)
            {
                const float g = drive.next(lowline::db(p("drive")));
                const float m = driveMix.next(driveTarget), x = work.getSample(0, i);
                work.setSample(0, i, x + m * (saturator.tick(x * g) - x));
            }
        else
        {
            driveMix.value = 0;
            saturator.reset();
        }
        work.copyFrom(1, 0, work, 0, 0, n);
        for (int stage = 0; stage < 2; ++stage)
            captures[size_t(stage)].process(work, n, p(stage == 0 ? "pedalOn" : "ampOn") > .5f,
                                           p("normOn") > .5f, p("normTarget"), false);
        for (int i = 0; i < n; ++i)
        {
            if ((i & 31) == 0) updateEq(false);
            float x = work.getSample(0, i);
            if (!std::isfinite(x))
                x = 0;
            for (auto &e : eq)
                x = e.tick(x);
            work.setSample(0, i, x);
        }
    }
    work.copyFrom(1, 0, work, 0, 0, n);
    if (process) captures[Cab].process(work, n, p("cabOn") > .5f, false, 0, false);
    const float polarity = p("invert") > .5f ? -1.f : 1.f;
    for (int i = 0; i < n; ++i)
    {
        const float original = dry.getSample(0, i);
        const float low = crossover.low(original, rate, p("crossFreq"));
        const float amount = crossMix.next(p("crossOn"));
        const float clean = (1 - amount) * original + amount * low;
        float d = dryDelay.tick(clean, p("align") * .001f * float(rate.load())) * polarity;
        float wet = blend.next(p("blend") * .01f);
        const float b = bypassMix.next(bypass ? 1.f : 0.f);
        for (int ch = 0; ch < 2; ++ch)
        {
            const float rig = process ? (1 - wet) * d + wet * work.getSample(ch, i) : 0.f;
            work.setSample(ch, i, b * raw.getSample(0, i) + (1 - b) * rig);
        }
    }
    for (int i = 0; i < n; ++i)
    {
        float l = work.getSample(0, i), r = work.getSample(1, i);
        float mix = delayMix.next(!bypass && p("delayOn") > .5f ? p("delayMix") * .01f : 0);
        delay.tick(l, r, effectiveDelay.load(), p("feedback") * .01f, mix, p("ping") > .5f, p("highcut"),
                   !bypass && p("delayOn") > .5f ? 1.f : 0.f, p("lowcut"));
        work.setSample(0, i, l);
        work.setSample(1, i, r);
    }
    dry.copyFrom(0, 0, work, 0, 0, n);
    dry.copyFrom(1, 0, work, 1, 0, n);
    captures[Room].process(work, n, !bypass && loaded[Room] && p("reverbOn") > .5f, false, 0, false);
    for (int i = 0; i < n; ++i)
    {
        float rm = reverbMix.next(!bypass && loaded[Room] && p("reverbOn") > .5f ? p("reverbMix") * .01f : 0);
        float g = outputGain.next(outTarget);
        for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
        {
            const float direct = getTotalNumOutputChannels() == 1
                                     ? .5f * (dry.getSample(0, i) + dry.getSample(1, i))
                                     : dry.getSample(ch, i);
            const float reverberated = getTotalNumOutputChannels() == 1
                                           ? .5f * (work.getSample(0, i) + work.getSample(1, i))
                                           : work.getSample(ch, i);
            float x = g * ((1 - rm) * direct + rm * reverberated);
            if (std::abs(x) > .98f || !std::isfinite(x))
                clipped = true;
            out.setSample(ch, off + i, std::isfinite(x) ? x : 0);
        }
    }
}
void LowlineProcessor::setStatus(const String &s)
{
    std::lock_guard g(metadataMutex);
    message = s;
}
String LowlineProcessor::status() const
{
    std::lock_guard g(metadataMutex);
    return message;
}
String LowlineProcessor::slotName(Slot s) const
{
    std::lock_guard g(metadataMutex);
    return paths[s].isEmpty() ? "(empty)" : File(paths[s]).getFileName();
}
void LowlineProcessor::loadFile(Slot s, const File &f)
{
    {
        std::lock_guard g(jobsMutex);
        jobs.push_back({s, f});
        busy = true;
    }
    jobsReady.notify_one();
}
void LowlineProcessor::clearSlot(Slot s)
{
    loadFile(s, File{});
}
void LowlineProcessor::workerLoop()
{
    String batchError;
    for (;;)
    {
        Job job;
        {
            std::unique_lock g(jobsMutex);
            jobsReady.wait_for(g, std::chrono::milliseconds(30), [this] { return stopping || !jobs.empty(); });
            if (stopping)
                return;
            if (jobs.empty())
            {
                g.unlock();
                for (auto &capture : captures) capture.collect();
                const int pc = requestedProgram.exchange(-1);
                if (pc >= 0)
                {
                    const String path = programAssignment(pc).isEmpty() ? String()
                        : lowline::platform::relocate(File(programAssignment(pc))).getFullPathName();
                    MemoryBlock bytes;
                    if (path.isNotEmpty() && buildPreset(File(path), bytes))
                    {
                        { std::lock_guard g(presetMutex); pendingPreset = std::move(bytes); }
                        triggerAsyncUpdate();
                    }
                }
                continue;
            }
            job = jobs.front();
            jobs.pop_front();
        }
        try
        {
            if (job.scene)
            {
                const double sr = rate; const int bs = maxBlock;
                std::array<std::unique_ptr<LowlineCapture>, 4> prepared;
                {
                    // A slot that already holds the same unmodified file keeps playing untouched:
                    // no re-parse, no crossfade and no doubled room-tail rendering on PC / A-B switches.
                    std::lock_guard g(metadataMutex);
                    for (int i = 0; i < 4; ++i)
                    {
                        const auto &f = job.files[size_t(i)];
                        const auto path = f == File{} ? String() : f.getFullPathName();
                        job.keep[size_t(i)] = paths[size_t(i)] == path &&
                            (path.isEmpty() || f.getLastModificationTime() == modified[size_t(i)]);
                    }
                }
                for (int i = 0; i < 4; ++i)
                    if (!job.keep[size_t(i)]) prepared[size_t(i)] = prepareCapture(Slot(i), job.files[size_t(i)], sr, bs);
                const SpinLock::ScopedLockType setup(modelLock);
                if (rate != sr || maxBlock != bs) throw std::runtime_error("Audio setup changed; recall preset again.");
                const SpinLock::ScopedLockType publish(publicationLock);
                for (int i = 0; i < 4; ++i)
                    if (!job.keep[size_t(i)]) publishCapture(Slot(i), job.files[size_t(i)], std::move(prepared[size_t(i)]));
                if (job.warning.isNotEmpty()) throw std::runtime_error(job.warning.toStdString());
            }
            else performLoad(job.slot, job.file);
        }
        catch (const std::exception &e)
        {
            batchError = "Load failed: " + String(e.what());
            setStatus(batchError);
        }
        {
            std::lock_guard g(jobsMutex);
            if (jobs.empty() && batchError.isNotEmpty())
            {
                setStatus(batchError);
                batchError.clear();
            }
            busy = !jobs.empty();
        }
    }
}
std::unique_ptr<LowlineCapture> LowlineProcessor::prepareCapture(Slot slot, const File &file, double sr, int bs)
{
    auto capture = std::make_unique<LowlineCapture>();
    if (file != File{})
    {
        if (!file.existsAsFile()) throw std::runtime_error("File not found: " + file.getFileName().toStdString());
        if (slot < Cab)
        {
            capture->model = nam::get_dsp(std::filesystem::u8path(file.getFullPathName().toStdString()));
            if (capture->model->NumInputChannels() != 1 || capture->model->NumOutputChannels() != 1)
                throw std::runtime_error("Use a mono NAM model.");
            const double expected = capture->model->GetExpectedSampleRate();
            if (expected > 0 && !capture->model->SupportsArbitrarySampleRate() && std::abs(expected - sr) > 1)
                throw std::runtime_error("Set device/DAW sample rate to " + String(expected, 0).toStdString() + " Hz, then reload.");
            capture->hasLoudness = capture->model->HasLoudness();
            if (capture->hasLoudness)
            {
                capture->loudness = float(capture->model->GetLoudness());
                capture->hasLoudness = std::isfinite(capture->loudness);
            }
        }
        else
        {
            AudioFormatManager formats; formats.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> reader(formats.createReaderFor(file));
            if (!reader || reader->lengthInSamples <= 0 || reader->numChannels > 2)
                throw std::runtime_error("Use a mono/stereo WAV or AIFF impulse response.");
            if (double(reader->lengthInSamples) / reader->sampleRate > (slot == Cab ? 2.0 : 20.0))
                throw std::runtime_error("IR too long (cab 2 s / room 20 s maximum).");
            capture->tailSeconds = slot == Room ? double(reader->lengthInSamples) / reader->sampleRate : 0;
            // The cab input is mono: a mono IR needs one convolution, not two identical ones.
            capture->monoIr = slot == Cab && reader->numChannels == 1;
            capture->ir = std::make_unique<dsp::Convolution>();
            capture->ir->loadImpulseResponse(file, capture->monoIr ? dsp::Convolution::Stereo::no : dsp::Convolution::Stereo::yes, dsp::Convolution::Trim::no, 0,
                                             dsp::Convolution::Normalise::yes);
        }
    }
    capture->prepare(sr, bs);
    return capture;
}
void LowlineProcessor::performLoad(Slot slot, const File &file)
{
    const double sr = rate; const int bs = maxBlock;
    auto capture = prepareCapture(slot,file,sr,bs);
    const SpinLock::ScopedLockType guard(modelLock);
    if (rate != sr || maxBlock != bs) throw std::runtime_error("Audio setup changed; reload the file.");
    const SpinLock::ScopedLockType publish(publicationLock);
    publishCapture(slot,file,std::move(capture));
}
void LowlineProcessor::publishCapture(Slot slot, const File &file, std::unique_ptr<LowlineCapture> capture)
{
    if (slot < Cab)
    {
        normalisationAvailable[size_t(slot)] = capture->hasLoudness;
        modelLoudness[size_t(slot)] = capture->loudness;
    }
    captures[size_t(slot)].publish(std::move(capture));
    ++stateVersion;
    loaded[size_t(slot)] = file != File{};
    {
        std::lock_guard g(metadataMutex);
        paths[slot] = file == File{} ? String() : file.getFullPathName();
        modified[slot] = file == File{} ? Time() : file.getLastModificationTime();
        message = file == File{} ? "Slot cleared / Rimosso" : "Loaded / Caricato: " + file.getFileName();
    }
}
const String LowlineProcessor::getProgramName(int i)
{
    const char *names[] = {"Clean foundation", "Warm edge", "Wide ambience", "Direct / DI"};
    return names[jlimit(0, 3, i)];
}
void LowlineProcessor::setCurrentProgram(int i)
{
    if (i < 0 || i >= 4)
        return;
    if (MessageManager::existsAndIsCurrentThread())
    {
        auto before = parameterSnapshot();
        applyProgram(i);
        state.state.setProperty("presetName", getProgramName(i), nullptr);
        recordUndo(getProgramName(i), std::move(before));
    }
    else
    {
        // MIDI Program Change on the audio thread: never touch the ValueTree here.
        applyProgram(i);
        programNamePending = i;
        triggerAsyncUpdate();
    }
}
void LowlineProcessor::applyProgram(int i)
{
    program = i;
    // Deterministic sound settings; preserve interface selection and input/output calibration.
    set("gateOn", 0); set("gateThreshold", -55); set("gateRelease", 150);
    set("lowMid", 0); set("highMid", 0);
    set("lowMidFreq", 250); set("midFreq", 700); set("highMidFreq", 1500);
    set("crossOn", 0); set("crossFreq", 180);
    set("sync", 0); set("lowcut", 0); set("normOn", 0);
    set("attack", 15);
    set("release", 120);
    set("invert", 0);
    set("align", 0);
    set("pedalOn", 1);
    set("ampOn", 1);
    set("cabOn", 1);
    set("time", 380);
    set("feedback", 27);
    set("highcut", 3500);
    set("ping", 1);
    set("compOn", 1);
    set("threshold", -22);
    set("ratio", 4);
    set("makeup", 0);
    set("bass", 0);
    set("mid", 0);
    set("treble", 0);
    set("bypass", 0);
    set("driveOn", i == 1 ? 1 : 0);
    set("drive", i == 1 ? 9 : 3);
    set("blend", i == 3 ? 0 : i == 1 ? 55 : 70);
    set("delayOn", i == 2 ? 1 : 0);
    set("reverbOn", i == 2 ? 1 : 0);
    set("delayMix", 18);
    set("reverbMix", 15);
}
void LowlineProcessor::getStateInformation(MemoryBlock &dest)
{
    auto tree = state.copyState();
    {
        std::lock_guard g(metadataMutex);
        for (int i = 0; i < 4; ++i)
            tree.setProperty("file" + String(i), paths[i], nullptr);
    }
    // MIDI map and Program Change bank are global (see loadMidiSettings), not part of a sound.
    tree.removeChild(tree.getChildWithName("MidiMappings"), nullptr);
    tree.removeChild(tree.getChildWithName("ProgramFiles"), nullptr);
    tree.setProperty("schema", 3, nullptr);
    tree.setProperty("language", getLanguage(), nullptr);
    tree.setProperty("program", program.load(), nullptr);
    auto xml = tree.createXml();
    copyXmlToBinary(*xml, dest);
}
void LowlineProcessor::setStateInformation(const void *data, int size)
{
    restoreState(data, size, false);
}
void LowlineProcessor::restoreState(const void *data, int size, bool keepUndo)
{
    auto xml = getXmlFromBinary(data, size);
    if (!xml || !xml->hasTagName("Lowline"))
        return;
    auto tree = ValueTree::fromXml(*xml);
    // Old presets receive defaults for newly introduced parameters.
    for (auto *parameter : midiTargets)
        if (!tree.getChildWithProperty("id", parameter->paramID).isValid())
        {
            ValueTree child("PARAM"); child.setProperty("id", parameter->paramID, nullptr);
            child.setProperty("value", parameter->convertFrom0to1(parameter->getDefaultValue()), nullptr);
            tree.addChild(child, -1, nullptr);
        }
    state.replaceState(tree);
    // APVTS skips parameters whose stored value already equals the restored one, which can leave
    // an automated in-between value (e.g. a switch at 0.77) in place: write every value explicitly.
    for (auto *parameter : midiTargets)
    {
        const auto child = tree.getChildWithProperty("id", parameter->paramID);
        if (!child.isValid()) continue;
        const float target = parameter->convertTo0to1(float(child.getProperty("value")));
        if (std::abs(parameter->getValue() - target) > 1.e-6f) parameter->setValueNotifyingHost(target);
    }
    if (!keepUndo)
    {
        // A loaded preset or host session starts a fresh history (UndoManager: message thread only).
        if (MessageManager::existsAndIsCurrentThread()) undo.clearUndoHistory();
        else { clearUndoPending = true; triggerAsyncUpdate(); }
    }
    // Older presets and sessions carry a MIDI map: it is adopted once, only while no global
    // controller settings exist yet (upgrade from 0.3). Afterwards presets never change it.
    if (!hasMidiSettings && (tree.getChildWithName("MidiMappings").isValid() || tree.getChildWithName("ProgramFiles").isValid()))
    {
        applyMidiSettings(tree);
        saveMidiSettings();
    }
    setLanguage(tree.getProperty("language", "IT").toString());
    program = jlimit(0, 3, int(tree.getProperty("program", 0)));
    {
        std::lock_guard guard(jobsMutex);
        Job scene; scene.slot = Pedal; scene.scene = true;
        for (int i = 0; i < 4; ++i)
        {
            auto path = tree.getProperty("file" + String(i)).toString();
            if (path.isNotEmpty()) path = lowline::platform::relocate(File(path)).getFullPathName(); // iOS container moved
            scene.files[size_t(i)] = path.isNotEmpty() && File(path).existsAsFile() ? File(path) : File{};
            if (path.isNotEmpty() && !File(path).existsAsFile()) scene.warning += "File not found: " + File(path).getFileName() + "; ";
        }
        jobs.push_back(std::move(scene));
        busy = true;
    }
    jobsReady.notify_one();
}
void LowlineProcessor::savePreset(const File &f, bool portable)
{
    if (busy)
    {
        setStatus("Wait for files to finish loading before saving.");
        return;
    }
    MemoryBlock b;
    getStateInformation(b);
    if (portable)
    {
        auto xml = getXmlFromBinary(b.getData(), int(b.getSize()));
        auto tree = ValueTree::fromXml(*xml);
        int64 total = 0;
        for (int slot = 0; slot < 4; ++slot)
        {
            const String path = tree.getProperty("file" + String(slot)).toString();
            if (path.isEmpty()) continue;
            File asset(path);
            const auto size = asset.getSize(); total += size;
            if (!asset.existsAsFile() || size <= 0 || size > 64 * 1024 * 1024 || total > 192 * 1024 * 1024)
            { setStatus("Portable save failed: missing or oversized asset (64 MB/file, 192 MB total)."); return; }
            MemoryBlock bytes;
            if (!asset.loadFileAsData(bytes)) { setStatus("Cannot read asset for portable preset."); return; }
            tree.setProperty("asset" + String(slot), bytes.toBase64Encoding(), nullptr);
            tree.setProperty("ext" + String(slot), asset.getFileExtension(), nullptr);
        }
        auto portableXml = tree.createXml(); b.reset(); copyXmlToBinary(*portableXml, b);
    }
    TemporaryFile temporary(f);
    if (!temporary.getFile().replaceWithData(b.getData(), b.getSize()) || !temporary.overwriteTargetFileWithTemporary())
        setStatus("Cannot save preset");
    else
    { markClean(); setStatus("Preset saved / Salvato"); }
}
bool LowlineProcessor::buildPreset(const File &f, MemoryBlock &out)
{
    MemoryBlock b;
    if (f.getSize() > 300LL * 1024 * 1024 || !f.loadFileAsData(b))
    {
        setStatus("Cannot read preset");
        return false;
    }
    auto xml = getXmlFromBinary(b.getData(), (int)b.getSize());
    if (!xml || !xml->hasTagName("Lowline"))
    {
        setStatus("Invalid Lowline preset");
        return false;
    }
    auto tree = ValueTree::fromXml(*xml);
    int64 total = 0;
    for (int slot = 0; slot < 4; ++slot)
    {
        const String encoded = tree.getProperty("asset" + String(slot)).toString();
        if (encoded.isEmpty()) continue;
        MemoryBlock bytes;
        if (!bytes.fromBase64Encoding(encoded) || bytes.getSize() > 64 * 1024 * 1024)
        { setStatus("Invalid portable asset."); return false; }
        total += int64(bytes.getSize());
        const String ext = tree.getProperty("ext" + String(slot)).toString().toLowerCase();
        if (total > 192LL * 1024 * 1024 || (slot < 2 ? ext != ".nam" : !StringArray{".wav", ".aif", ".aiff"}.contains(ext)))
        { setStatus("Invalid portable asset type or size."); return false; }
        auto cache = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Lowline/Assets");
        if (cache.createDirectory().failed()) { setStatus("Cannot create asset cache."); return false; }
        // The name is the content hash: an existing file of the same size is already that asset,
        // so Program Change switching does not re-hash large IRs every time.
        auto asset = cache.getChildFile(SHA256(bytes).toHexString() + ext);
        if (!asset.existsAsFile() || asset.getSize() != int64(bytes.getSize()))
        {
            TemporaryFile temporary(asset);
            if (!temporary.getFile().replaceWithData(bytes.getData(), bytes.getSize()) ||
                !temporary.overwriteTargetFileWithTemporary())
            { setStatus("Cannot extract portable asset."); return false; }
        }
        tree.setProperty("file" + String(slot), asset.getFullPathName(), nullptr);
        tree.removeProperty("asset" + String(slot), nullptr);
        tree.removeProperty("ext" + String(slot), nullptr);
    }
    auto extracted = tree.createXml(); out.reset(); copyXmlToBinary(*extracted, out);
    return true;
}
void LowlineProcessor::openPreset(const File &f)
{
    MemoryBlock bytes;
    if (!buildPreset(f, bytes)) return;
    setStateInformation(bytes.getData(), (int)bytes.getSize());
    cleanWhenReady = true;
    // Loader errors must remain visible instead of being overwritten by a success message.
}
void LowlineProcessor::handleAsyncUpdate()
{
    if (clearUndoPending.exchange(false)) undo.clearUndoHistory();
    if (midiSettingsDirty) saveMidiSettings();
    if (const int named = programNamePending.exchange(-1); named >= 0)
        state.state.setProperty("presetName", getProgramName(named), nullptr);
    MemoryBlock bytes;
    { std::lock_guard g(presetMutex); bytes.swapWith(pendingPreset); }
    if (bytes.getSize() == 0) return;
    setStateInformation(bytes.getData(), (int)bytes.getSize());
    cleanWhenReady = true;
}

AudioProcessorEditor *LowlineProcessor::createEditor()
{
    return new LowlineEditor(*this);
}
AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new LowlineProcessor();
}

String LowlineProcessor::presetName() const { return state.state.getProperty("presetName", "Clean foundation").toString(); }
String LowlineProcessor::presetTags() const { return state.state.getProperty("presetTags").toString(); }
String LowlineProcessor::presetNotes() const { return state.state.getProperty("presetNotes").toString(); }
void LowlineProcessor::setPresetDetails(const String &name, const String &tags, const String &notes)
{
    undo.beginNewTransaction("Preset details");
    state.state.setProperty("presetName", name.substring(0,120), &undo);
    state.state.setProperty("presetTags", tags.substring(0,240), &undo);
    state.state.setProperty("presetNotes", notes.substring(0,2000), &undo);
}
void LowlineProcessor::markClean()
{
    // Parameters and editorial metadata; asset jobs are checked separately by the UI.
    MemoryBlock bytes; getStateInformation(bytes); cleanSignature = SHA256(bytes).toHexString();
}
bool LowlineProcessor::isDirty()
{
    if (cleanWhenReady && !busy) { cleanWhenReady = false; markClean(); }
    MemoryBlock bytes; getStateInformation(bytes);
    return cleanSignature.isEmpty() || cleanSignature != SHA256(bytes).toHexString();
}
void LowlineProcessor::storeComparison(int index)
{
    if (index < 0 || index > 1 || busy) return;
    getStateInformation(comparison[size_t(index)]);
    const String name = index == 0 ? "A" : "B";
    if (measurementFresh())
    {
        comparisonRms[size_t(index)] = measuredRms.load();
        setStatus("Stored / Memorizzato " + name);
    }
    else
    {
        // No valid measurement of this exact sound: measure it now and attach the result.
        comparisonRms[size_t(index)] = -100.f;
        measureFor = index;
        measureLevel();
        setStatus("Stored " + name + ": play for 5 s to measure / Memorizzato " + name + ": suona per 5 s per misurarlo");
    }
}
bool LowlineProcessor::recallComparison(int index)
{
    if (!hasComparison(index) || busy) return false;
    auto before = parameterSnapshot();
    auto &b = comparison[size_t(index)];
    restoreState(b.getData(), int(b.getSize()), true); // A/B keeps the undo history
    recordUndo(index == 0 ? "Recall A" : "Recall B", std::move(before));
    compareTrim = 0;
    measureAfterLoad = true; // MATCH needs a measurement of the recalled sound
    return true;
}
void LowlineProcessor::matchComparison(int reference)
{
    if (reference < 0 || reference > 1 || comparisonRms[size_t(reference)] < -60 || measuredRms < -60)
    { setStatus("Measure both sounds above -60 dB RMS / Misura entrambi i suoni"); return; }
    if (!measurementFresh())
    { setStatus("Measure the current sound first / Misura prima il suono attuale"); return; }
    compareTrim = juce::jlimit(-12.f, 12.f, comparisonRms[size_t(reference)] - measuredRms.load());
}

namespace
{
struct SnapshotAction : UndoableAction
{
    SnapshotAction(LowlineProcessor &p, std::vector<float> b, std::vector<float> a)
        : processor(p), before(std::move(b)), after(std::move(a)) {}
    bool perform() override
    {
        if (first) { first = false; return true; } // already applied when recorded
        processor.applySnapshot(after);
        return true;
    }
    bool undo() override { processor.applySnapshot(before); return true; }
    int getSizeInUnits() override { return int(before.size() * 2 * sizeof(float)) / 64 + 1; }
    LowlineProcessor &processor;
    std::vector<float> before, after;
    bool first = true;
};
} // namespace
std::vector<float> LowlineProcessor::parameterSnapshot() const
{
    std::vector<float> values;
    for (auto *parameter : getParameters()) values.push_back(parameter->getValue());
    return values;
}
void LowlineProcessor::applySnapshot(const std::vector<float> &values)
{
    const auto &parameters = getParameters();
    applyingUndo = true;
    for (size_t i = 0; i < values.size() && i < size_t(parameters.size()); ++i)
        if (std::abs(parameters[int(i)]->getValue() - values[i]) > 1.e-7f)
        {
            parameters[int(i)]->beginChangeGesture();
            parameters[int(i)]->setValueNotifyingHost(values[i]);
            parameters[int(i)]->endChangeGesture();
        }
    applyingUndo = false;
}
void LowlineProcessor::recordUndo(const String &name, std::vector<float> before)
{
    if (!MessageManager::existsAndIsCurrentThread()) return;
    auto after = parameterSnapshot();
    if (after == before) return;
    undo.beginNewTransaction(name);
    undo.perform(new SnapshotAction(*this, std::move(before), std::move(after)), name);
}
void LowlineProcessor::parameterGestureChanged(int, bool starting)
{
    // Gestures come from the editor's attachments (message thread); MIDI, automation and tap
    // tempo change values without gestures, so they never enter the history.
    if (applyingUndo || !MessageManager::existsAndIsCurrentThread()) return;
    if (starting)
    {
        if (gestureDepth++ == 0) gestureBefore = parameterSnapshot();
    }
    else if (gestureDepth > 0 && --gestureDepth == 0)
        recordUndo("Edit", std::move(gestureBefore));
}
