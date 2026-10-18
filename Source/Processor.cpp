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
    return l;
}
LowlineProcessor::LowlineProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", AudioChannelSet::stereo(), true)
                         .withOutput("Output", AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "Lowline", parameters()), worker([this] { workerLoop(); })
{
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
    limiter.prepare(sr);
    compressor.prepare(sr);
    delay.prepare(sr);
    dryDelay.prepare(sr);
    for (auto &e : eq)
        e.reset();
    inputGain.prepare(sr, lowline::db(p("input")));
    outputGain.prepare(sr, lowline::db(p("output")));
    blend.prepare(sr, p("blend") * .01f);
    drive.prepare(sr, lowline::db(p("drive")));
    delayMix.prepare(sr, p("delayOn") ? p("delayMix") * .01f : 0);
    reverbMix.prepare(sr, p("reverbOn") ? p("reverbMix") * .01f : 0);
    dsp::ProcessSpec spec{sr, (uint32)maxBlock.load(), 2};
    if (cab)
        cab->prepare(spec);
    if (room)
        room->prepare(spec);
    for (int i = 0; i < 2; ++i)
        if (models[i])
        {
            auto expected = models[i]->GetExpectedSampleRate();
            bool ok = expected < 0 || models[i]->SupportsArbitrarySampleRate() || std::abs(expected - sr) < 1;
            loaded[i] = ok;
            if (ok)
                models[i]->Reset(sr, maxBlock);
            else
                setStatus("NAM bypassed: match the device sample rate to the model, then reload.");
        }
    setLatencySamples(limiter.delaySamples);
}
void LowlineProcessor::set(const char *id, float v)
{
    if (auto *x = state.getParameter(id))
        x->setValueNotifyingHost(x->convertTo0to1(v));
}
void LowlineProcessor::midi(const MidiBuffer &m)
{
    for (auto item : m)
    {
        auto msg = item.getMessage();
        if (msg.isProgramChange())
            setCurrentProgram(msg.getProgramChangeNumber());
        if (msg.isController())
        {
            int cc = msg.getControllerNumber();
            float v = float(msg.getControllerValue());
            const char *ids[] = {"compOn", "driveOn", "pedalOn",  "ampOn",
                                 "cabOn",  "delayOn", "reverbOn", "bypass"};
            if (cc >= 20 && cc <= 27)
                set(ids[cc - 20], v >= 64 ? 1.f : 0.f);
            if (cc == 11)
                set("blend", v * 100 / 127);
            if (cc == 7)
                set("output", -36 + v * 42 / 127);
        }
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
    // Never wait for a file loader on the audio thread. Its commit only swaps prepared objects.
    const SpinLock::ScopedTryLockType guard(modelLock);
    if (!guard.isLocked())
    {
        const float gain = lowline::db(p("input") + p("output"));
        const int src = int(p("source")), inputs = getTotalNumInputChannels();
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float x = buffer.getSample(src == 1 && inputs > 1 ? 1 : 0, i);
            if (src == 2 && inputs > 1)
                x = .5f * (buffer.getSample(0, i) + buffer.getSample(1, i));
            x = std::isfinite(x) ? x * gain : 0;
            for (int c = 0; c < getTotalNumOutputChannels(); ++c)
                buffer.setSample(c, i, x);
        }
        limitOutput(buffer);
        return;
    }
    for (int off = 0; off < buffer.getNumSamples(); off += maxBlock)
        processChunk(buffer, off, std::min(maxBlock.load(), buffer.getNumSamples() - off));
    limitOutput(buffer);
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
    limitOutput(buffer);
}
void LowlineProcessor::limitOutput(AudioBuffer<float> &buffer)
{
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float l = buffer.getSample(0, i), r = getTotalNumOutputChannels() > 1 ? buffer.getSample(1, i) : l;
        limiter.tick(l, r);
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
void LowlineProcessor::processChunk(AudioBuffer<float> &out, int off, int n)
{
    const float inTarget = lowline::db(p("input")), outTarget = lowline::db(p("output"));
    const bool bypass = p("bypass") > .5f;
    const int source = int(p("source")), inputs = getTotalNumInputChannels();
    for (int i = 0; i < n; ++i)
    {
        float x = out.getSample(source == 1 && inputs > 1 ? 1 : 0, off + i);
        if (source == 2 && inputs > 1)
            x = .5f * (out.getSample(0, off + i) + out.getSample(1, off + i));
        if (!std::isfinite(x))
            x = 0;
        x *= inputGain.next(inTarget);
        if (p("compOn") > .5f && !bypass)
            x = compressor.tick(x, p("threshold"), p("ratio"), p("attack"), p("release"), p("makeup"));
        dry.setSample(0, i, x);
        work.setSample(0, i, x);
    }
    gainReduction = p("compOn") > .5f && !bypass ? compressor.reduction : 0;
    if (!bypass)
    {
        if (p("driveOn") > .5f)
            for (int i = 0; i < n; ++i)
            {
                float g = drive.next(lowline::db(p("drive")));
                work.setSample(0, i, std::tanh(work.getSample(0, i) * g));
            }
        for (int s = 0; s < 2; ++s)
            if (loaded[s] && p(s == 0 ? "pedalOn" : "ampOn") > .5f && models[s])
            {
                float *in[] = {work.getWritePointer(0)};
                float *dst[] = {work.getWritePointer(1)};
                models[s]->process(in, dst, n);
                work.copyFrom(0, 0, work, 1, 0, n);
            }
        eq[0].peak(rate, 100, p("bass"));
        eq[1].peak(rate, 700, p("mid"));
        eq[2].peak(rate, 3500, p("treble"));
        for (int i = 0; i < n; ++i)
        {
            float x = work.getSample(0, i);
            if (!std::isfinite(x))
                x = 0;
            for (auto &e : eq)
                x = e.tick(x);
            work.setSample(0, i, x);
        }
    }
    work.copyFrom(1, 0, work, 0, 0, n);
    auto block = dsp::AudioBlock<float>(work).getSubBlock(0, (size_t)n);
    if (!bypass && loaded[Cab] && p("cabOn") > .5f && cab)
    {
        dsp::ProcessContextReplacing<float> c(block);
        cab->process(c);
    }
    const float polarity = p("invert") > .5f ? -1.f : 1.f;
    for (int i = 0; i < n; ++i)
    {
        float d = dryDelay.tick(dry.getSample(0, i), p("align") * .001f * float(rate.load())) * polarity;
        float wet = blend.next(p("blend") * .01f);
        for (int ch = 0; ch < 2; ++ch)
            work.setSample(ch, i, bypass ? dry.getSample(0, i) : (1 - wet) * d + wet * work.getSample(ch, i));
    }
    for (int i = 0; i < n; ++i)
    {
        float l = work.getSample(0, i), r = work.getSample(1, i);
        float mix = delayMix.next(!bypass && p("delayOn") > .5f ? p("delayMix") * .01f : 0);
        delay.tick(l, r, p("time"), p("feedback") * .01f, mix, p("ping") > .5f, p("highcut"),
                   !bypass && p("delayOn") > .5f ? 1.f : 0.f);
        work.setSample(0, i, l);
        work.setSample(1, i, r);
    }
    dry.copyFrom(0, 0, work, 0, 0, n);
    dry.copyFrom(1, 0, work, 1, 0, n);
    if (loaded[Room] && room)
    {
        dsp::ProcessContextReplacing<float> c(block);
        room->process(c);
    }
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
            jobsReady.wait(g, [this] { return stopping || !jobs.empty(); });
            if (stopping)
                return;
            job = jobs.front();
            jobs.pop_front();
        }
        try
        {
            performLoad(job.slot, job.file);
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
void LowlineProcessor::performLoad(Slot slot, const File &file)
{
    if (file == File{})
    {
        // Retire objects on this worker, never destroy a neural model on the audio callback.
        std::unique_ptr<nam::DSP> oldModel;
        std::unique_ptr<dsp::Convolution> oldIR;
        {
            const SpinLock::ScopedLockType guard(modelLock);
            loaded[slot] = false;
            if (slot < Cab)
                oldModel.swap(models[slot]);
            else
                oldIR.swap(slot == Cab ? cab : room);
            std::lock_guard g(metadataMutex);
            paths[slot].clear();
            message = "Slot cleared / Rimosso";
        }
        return;
    }
    if (!file.existsAsFile())
        throw std::runtime_error("File not found: " + file.getFileName().toStdString());
    double sr = rate;
    int bs = maxBlock;
    if (slot == Pedal || slot == Amp)
    {
        auto model = nam::get_dsp(std::filesystem::u8path(file.getFullPathName().toStdString()));
        if (model->NumInputChannels() != 1 || model->NumOutputChannels() != 1)
            throw std::runtime_error("This version accepts mono NAM models only.");
        double expected = model->GetExpectedSampleRate();
        if (expected > 0 && !model->SupportsArbitrarySampleRate() && std::abs(expected - sr) > 1)
            throw std::runtime_error("Set audio device/DAW to " + String(expected, 0).toStdString() +
                                     " Hz for this NAM model.");
        model->Reset(sr, bs);
        {
            const SpinLock::ScopedLockType guard(modelLock);
            if (rate != sr || maxBlock != bs)
                throw std::runtime_error("Audio setup changed; please reload the model.");
            models[slot].swap(model);
            loaded[slot] = true;
        }
    }
    else
    {
        AudioFormatManager formats;
        formats.registerBasicFormats();
        std::unique_ptr<AudioFormatReader> reader(formats.createReaderFor(file));
        if (!reader || reader->lengthInSamples <= 0 || reader->numChannels > 2)
            throw std::runtime_error("Use a mono/stereo WAV or AIFF impulse response.");
        double seconds = double(reader->lengthInSamples) / reader->sampleRate;
        if (seconds > (slot == Cab ? 2.0 : 20.0))
            throw std::runtime_error(slot == Cab ? "Cab IR maximum: 2 seconds."
                                                 : "Reverb IR maximum: 20 seconds.");
        auto conv = std::make_unique<dsp::Convolution>();
        conv->loadImpulseResponse(file, dsp::Convolution::Stereo::yes, dsp::Convolution::Trim::no, 0,
                                  dsp::Convolution::Normalise::yes);
        conv->prepare({sr, (uint32)bs, 2});
        {
            const SpinLock::ScopedLockType guard(modelLock);
            if (rate != sr || maxBlock != bs)
                throw std::runtime_error("Audio setup changed; please reload the IR.");
            (slot == Cab ? cab : room).swap(conv);
            loaded[slot] = true;
        }
    }
    {
        std::lock_guard g(metadataMutex);
        paths[slot] = file.getFullPathName();
        message = "Loaded / Caricato: " + file.getFileName();
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
    program = i;
    // Deterministic sound settings; preserve interface selection and input/output calibration.
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
    tree.setProperty("language", getLanguage(), nullptr);
    tree.setProperty("program", program.load(), nullptr);
    auto xml = tree.createXml();
    copyXmlToBinary(*xml, dest);
}
void LowlineProcessor::setStateInformation(const void *data, int size)
{
    auto xml = getXmlFromBinary(data, size);
    if (!xml || !xml->hasTagName("Lowline"))
        return;
    auto tree = ValueTree::fromXml(*xml);
    state.replaceState(tree);
    setLanguage(tree.getProperty("language", "IT").toString());
    program = jlimit(0, 3, int(tree.getProperty("program", 0)));
    {
        std::lock_guard guard(jobsMutex);
        for (int i = 0; i < 4; ++i)
        {
            auto path = tree.getProperty("file" + String(i)).toString();
            jobs.push_back({(Slot)i, File{}});
            if (path.isNotEmpty())
                jobs.push_back({(Slot)i, File(path)});
        }
        busy = true;
    }
    jobsReady.notify_one();
}
void LowlineProcessor::savePreset(const File &f)
{
    if (busy)
    {
        setStatus("Wait for files to finish loading before saving.");
        return;
    }
    MemoryBlock b;
    getStateInformation(b);
    if (!f.replaceWithData(b.getData(), b.getSize()))
        setStatus("Cannot save preset");
    else
        setStatus("Preset saved / Salvato");
}
void LowlineProcessor::openPreset(const File &f)
{
    MemoryBlock b;
    if (!f.loadFileAsData(b))
    {
        setStatus("Cannot read preset");
        return;
    }
    auto xml = getXmlFromBinary(b.getData(), (int)b.getSize());
    if (!xml || !xml->hasTagName("Lowline"))
    {
        setStatus("Invalid Lowline preset");
        return;
    }
    setStateInformation(b.getData(), (int)b.getSize());
    // Loader errors must remain visible instead of being overwritten by a success message.
}
AudioProcessorEditor *LowlineProcessor::createEditor()
{
    return new LowlineEditor(*this);
}
AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new LowlineProcessor();
}
