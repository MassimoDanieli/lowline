#include "../Source/Processor.h"
#include "../Source/Help.h"
#include "../Source/Workbench.h"
#include <iostream>
#include <stdexcept>
using namespace juce;
void require(bool ok, const char *what)
{
    if (!ok)
        throw std::runtime_error(what);
    std::cout << "PASS " << what << "\n";
}
void set(LowlineProcessor &p, const char *id, float v)
{
    auto *x = p.state.getParameter(id);
    x->setValueNotifyingHost(x->convertTo0to1(v));
}
void settle(LowlineProcessor &p, AudioBuffer<float> &b, float value = 0)
{
    MidiBuffer m;
    for (int n = 0; n < 80; ++n)
    {
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < b.getNumSamples(); ++i)
                b.setSample(c, i, value);
        p.processBlock(b, m);
    }
}
float toneRms(LowlineProcessor &processor, float hz)
{
    AudioBuffer<float> audio(2, 256);
    MidiBuffer midi;
    for (int block = 0; block < 120; ++block)
    {
        for (int i = 0; i < 256; ++i)
            for (int ch = 0; ch < 2; ++ch)
                audio.setSample(ch, i, .05f * std::sin(2 * lowline::pi * hz * (block * 256 + i) / 48000));
        processor.processBlock(audio, midi);
    }
    return audio.getRMSLevel(0, 0, 256);
}
class TempoPlayHead : public AudioPlayHead
{
  public:
    Optional<PositionInfo> getPosition() const override
    {
        PositionInfo position; position.setBpm(90.0); return position;
    }
};
void wait(LowlineProcessor &p)
{
    for (int i = 0; i < 1000 && p.busy; ++i)
        Thread::sleep(10);
    require(!p.busy, "background load completes");
}
int main(int argc, char **argv)
{
    ScopedJuceInitialiser_GUI gui;
    try
    {
        lowline::Compressor c;
        c.prepare(48000);
        float x = 0;
        for (int i = 0; i < 48000; ++i)
            x = c.tick(.5f, -24, 4, 5, 100, 0);
        require(std::abs(x) < .15f && c.reduction > 10, "compressor reduces steady input");
        lowline::Delay d;
        d.prepare(48000);
        float leftEnergy = 0, rightEnergy = 0;
        for (int i = 0; i < 90000; ++i)
        {
            float l = i == 0 ? 1.f : 0.f, r = l;
            d.tick(l, r, 380, .5f, 1, true, 10000);
            if (i > 18000 && i < 18700)
                leftEnergy += std::abs(l);
            if (i > 36000 && i < 37500)
                rightEnergy += std::abs(r);
        }
        require(leftEnergy > .8 && rightEnergy > .3, "ping-pong alternates left then right");
        lowline::PeakLimiter limiter;
        limiter.prepare(48000);
        float maxPeak = 0, maxRatioError = 0;
        for (int i = 0; i < 48000; ++i)
        {
            float l = 2.f * std::sin(2.f * lowline::pi * 100.f * float(i) / 48000.f), r = l * .5f;
            limiter.tick(l, r);
            maxPeak = std::max(maxPeak, std::abs(l));
            maxRatioError = std::max(maxRatioError, std::abs(l - 2.f * r));
        }
        require(maxPeak <= .98001f && maxPeak > .95f, "lookahead limiter contains overloaded sine peaks");
        require(maxRatioError < .00001f,
                "limiter preserves stereo balance instead of clipping channels independently");
        lowline::PeakLimiter impulseLimiter;
        impulseLimiter.prepare(48000);
        int impulseIndex = -1;
        for (int i = 0; i < 100; ++i)
        {
            float l = i == 0 ? .2f : 0, r = l;
            impulseLimiter.tick(l, r);
            if (l > .1f)
                impulseIndex = i;
        }
        require(impulseIndex == 48, "limiter adds exactly the declared 1 ms lookahead");
        lowline::Delay disabledDelay;
        disabledDelay.prepare(48000);
        for (int i = 0; i < 50000; ++i)
        {
            float l = .5f, r = l;
            disabledDelay.tick(l, r, 380, .5f, 0, true, 3500, 0);
        }
        float silence = 0;
        for (int i = 0; i < 2000; ++i)
        {
            float l = 0, r = 0;
            disabledDelay.tick(l, r, 380, .5f, 1, true, 3500, 1);
            silence = std::max(silence, std::abs(l) + std::abs(r));
        }
        require(silence == 0, "disabled delay does not secretly record new notes");
        // Bass fundamentals, harmonics, DC rejection and cents accuracy.
        std::array<float, 2048> pitchFrame{};
        for (float hz : {30.8677f, 41.2034f, 55.f, 73.4162f, 98.f, 110.f, 440.f})
        {
            for (int i = 0; i < int(pitchFrame.size()); ++i)
            {
                const double phase = 2 * lowline::pi * hz * i / 6000;
                pitchFrame[size_t(i)] = float(.2 * std::sin(phase) + .08 * std::sin(2 * phase) + .03 * std::sin(3 * phase));
            }
            const float detected = LowlineTuner::detect(pitchFrame.data(), int(pitchFrame.size()), 6000);
            require(detected > 0 && std::abs(1200 * std::log2(detected / hz)) < 4,
                    "tuner identifies bass fundamental with harmonics within four cents");
        }
        pitchFrame.fill(.2f);
        require(LowlineTuner::detect(pitchFrame.data(), int(pitchFrame.size()), 6000) == 0, "tuner rejects DC");
        pitchFrame.fill(0);
        require(LowlineTuner::detect(pitchFrame.data(), int(pitchFrame.size()), 6000) == 0, "tuner rejects silence");
        lowline::Gate gate;
        gate.prepare(48000);
        float gated = 0;
        for (int i = 0; i < 96000; ++i) gated = gate.tick(.0001f, true, -55, 150);
        require(std::abs(gated) < 1.e-8f, "gate suppresses quiet background noise");
        for (int i = 0; i < 4800; ++i) gated = gate.tick(.1f, true, -55, 150);
        require(std::abs(gated - .1f) < .001f, "gate opens for played notes");
        require(std::abs(lowline::tempoDelay(120, 2) - 375) < .01f &&
                std::abs(lowline::tempoDelay(60, 5) - 2000) < .01f,
                "delay computes dotted and half-note subdivisions");
        lowline::CleanCrossover cross;
        double lowEnergy = 0, highEnergy = 0;
        for (int i = 0; i < 48000; ++i)
        {
            const float y = cross.low(std::sin(2 * lowline::pi * 50 * i / 48000), 48000, 180);
            if (i > 24000) lowEnergy += y * y;
        }
        cross.reset();
        for (int i = 0; i < 48000; ++i)
        {
            const float y = cross.low(std::sin(2 * lowline::pi * 2000 * i / 48000), 48000, 180);
            if (i > 24000) highEnergy += y * y;
        }
        require(lowEnergy > 9000 && highEnergy < lowEnergy * .001, "clean crossover retains bass and rejects high frequencies");
        LowlineProcessor p;
        p.prepareToPlay(48000, 256);
        require(p.getLatencySamples() == 48, "processor reports limiter latency to host");
        AudioBuffer<float> b(2, 256);
        set(p, "compOn", 0);
        set(p, "output", 0);
        set(p, "blend", 0);
        settle(p, b, .2f);
        require(std::abs(b.getSample(0, 255) - .2f) < .0005f, "clean path unity");
        set(p, "blend", 100);
        settle(p, b, .2f);
        require(std::abs(b.getSample(0, 255) - .2f) < .0005f, "unloaded wet path unity");
        set(p, "blend", 0);
        set(p, "invert", 1);
        settle(p, b, .2f);
        require(std::abs(b.getSample(0, 255) + .2f) < .0005f, "clean polarity inversion");
        set(p, "invert", 0);
        set(p, "output", 12);
        settle(p, b, .8f);
        require(b.getMagnitude(0, 256) <= .98001f, "output ceiling");
        set(p, "output", 0);
        set(p, "blend", 100);
        const auto temp =
            File::getSpecialLocation(File::tempDirectory).getNonexistentChildFile("lowline-test", "", false);
        temp.createDirectory();
        auto model = temp.getChildFile("gain.nam");
        model.replaceWithText(
            R"({"version":"0.5.4","architecture":"Linear","config":{"receptive_field":1,"bias":false},"weights":[0.5],"sample_rate":48000})");
        p.loadFile(LowlineProcessor::Amp, model);
        wait(p);
        require(p.slotName(LowlineProcessor::Amp) == "gain.nam", "real NAM loader accepts model");
        settle(p, b, .2f);
        require(std::abs(b.getSample(0, 255) - .1f) < .001f, "NAM engine processes known gain model");
        set(p, "blend", 50);
        settle(p, b, .2f);
        require(std::abs(b.getSample(0, 255) - .15f) < .001f,
                "parallel blend combines clean and real NAM output");
        set(p, "blend", 100);
        auto ir = temp.getChildFile("unity.wav");
        {
            WavAudioFormat wav;
            std::unique_ptr<AudioFormatWriter> w(
                wav.createWriterFor(new FileOutputStream(ir), 48000, 2, 24, {}, 0));
            AudioBuffer<float> impulse(2, 512);
            impulse.clear();
            impulse.setSample(0, 0, 1);
            impulse.setSample(1, 0, 1);
            require(w && w->writeFromAudioSampleBuffer(impulse, 0, 512), "write test IR");
        }
        p.loadFile(LowlineProcessor::Cab, ir);
        wait(p);
        settle(p, b, .2f);
        require(std::isfinite(b.getSample(0, 255)) && b.getMagnitude(0, 256) > .01,
                "cab convolution produces finite audio");
        auto bad = temp.getChildFile("bad.nam");
        bad.replaceWithText("not json");
        p.loadFile(LowlineProcessor::Amp, bad);
        wait(p);
        require(p.slotName(LowlineProcessor::Amp) == "gain.nam", "invalid load preserves active model");
        MemoryBlock state;
        p.getStateInformation(state);
        set(p, "blend", 12);
        p.setStateInformation(state.getData(), (int)state.getSize());
        wait(p);
        require(std::abs(p.state.getRawParameterValue("blend")->load() - 100) < .01f,
                "preset roundtrip restores controls");
        AudioBuffer<float> big(2, 2048);
        settle(p, big, .05f);
        require(std::isfinite(big.getSample(0, 2047)), "oversized host blocks are chunked safely");
        settle(p, b, std::numeric_limits<float>::quiet_NaN());
        require(std::isfinite(b.getSample(0, 255)), "nonfinite input is contained");
        settle(p, b, .05f);
        require(b.getMagnitude(0, 256) > .001f, "audio recovers after nonfinite input");
        MidiBuffer midi;
        midi.addEvent(MidiMessage::controllerEvent(1, 11, 0), 0);
        p.processBlock(b, midi);
        require(p.state.getRawParameterValue("blend")->load() == 0, "MIDI CC11 controls blend");
        p.setCurrentProgram(0);
        MidiBuffer unknownPC;
        unknownPC.addEvent(MidiMessage::programChange(1, 127), 0);
        p.processBlock(b, unknownPC);
        require(p.getCurrentProgram() == 0, "unmapped Program Changes are ignored instead of wrapping");
        set(p, "align", 12);
        set(p, "attack", 80);
        set(p, "feedback", 80);
        set(p, "ampOn", 0);
        p.setCurrentProgram(0);
        require(p.state.getRawParameterValue("align")->load() == 0 &&
                    p.state.getRawParameterValue("attack")->load() == 15 &&
                    p.state.getRawParameterValue("feedback")->load() == 27 &&
                    p.state.getRawParameterValue("ampOn")->load() == 1,
                "factory recall resets sound controls deterministically");
        set(p, "cabOn", 0);
        p.clearSlot(LowlineProcessor::Amp);
        wait(p);
        p.prepareToPlay(48000, 256);
        set(p, "compOn", 0);
        set(p, "output", 0);
        set(p, "blend", 100);
        set(p, "cabOn", 0);
        settle(p, b, .2f);
        require(p.slotName(LowlineProcessor::Amp) == "(empty)" && std::abs(b.getSample(0, 255) - .2f) < .001f,
                "removed NAM remains removed after prepareToPlay");
        auto wrongPreset = temp.getChildFile("wrong.lowline");
        MemoryBlock wrongBytes;
        XmlElement wrongRoot("OtherApplication");
        AudioProcessor::copyXmlToBinary(wrongRoot, wrongBytes);
        wrongPreset.replaceWithData(wrongBytes.getData(), wrongBytes.getSize());
        set(p, "blend", 37);
        p.openPreset(wrongPreset);
        require(p.state.getRawParameterValue("blend")->load() == 37 && p.status().contains("Invalid"),
                "foreign XML preset is rejected without changing controls");
        LowlineProcessor shaping;
        shaping.prepareToPlay(48000, 256);
        set(shaping, "compOn", 0); set(shaping, "blend", 100); set(shaping, "output", 0);
        for (auto entry : {std::pair<const char *, float>{"bass", 100}, {"lowMid", 250}, {"mid", 700}, {"highMid", 1500}, {"treble", 3500}})
        {
            const float dryLevel = toneRms(shaping, entry.second);
            set(shaping, entry.first, 6);
            const float boosted = toneRms(shaping, entry.second);
            require(std::abs(boosted / dryLevel - lowline::db(6)) < .03f, "five-band EQ applies the selected band's gain");
            set(shaping, entry.first, 0);
        }
        TempoPlayHead hostTempo;
        shaping.setPlayHead(&hostTempo);
        set(shaping, "sync", 1); set(shaping, "hostSync", 1); set(shaping, "division", 3);
        MidiBuffer hostMessages;
        shaping.processBlock(b, hostMessages);
        require(shaping.usingHostTempo && std::abs(shaping.effectiveDelay.load() - 60000.f / 90) < .1f,
                "delay follows the host's supplied tempo");
        shaping.setPlayHead(nullptr);
        // Portable assets survive removal of the original files.
        auto portableModel = temp.getChildFile("portable.nam");
        model.copyFileTo(portableModel);
        auto portableIR = temp.getChildFile("portable.wav");
        ir.copyFileTo(portableIR);
        LowlineProcessor portableSource;
        portableSource.prepareToPlay(48000, 256);
        set(portableSource, "compOn", 0); set(portableSource, "output", 0); set(portableSource, "blend", 100);
        set(portableSource, "pedalOn", 0); set(portableSource, "cabOn", 0);
        portableSource.loadFile(LowlineProcessor::Pedal, portableModel);
        portableSource.loadFile(LowlineProcessor::Amp, portableModel);
        portableSource.loadFile(LowlineProcessor::Cab, portableIR);
        portableSource.loadFile(LowlineProcessor::Room, portableIR);
        wait(portableSource);
        const int blendTarget = portableSource.midiTargetNames().indexOf("Amp blend %");
        require(blendTarget >= 0, "blend is available for MIDI learn");
        portableSource.armMidiLearn(blendTarget);
        MidiBuffer learn;
        learn.addEvent(MidiMessage::controllerEvent(2, 45, 127), 0);
        portableSource.processBlock(b, learn);
        require(portableSource.midiDescription().contains("CC 45") && !portableSource.midiDescription().contains("CC 11 "),
                "MIDI learn replaces the target assignment and accepts another channel");
        auto portablePreset = temp.getChildFile("portable.lowline");
        portableSource.savePreset(portablePreset, true);
        require(portablePreset.existsAsFile(), "portable preset is saved");
        portableModel.deleteFile(); portableIR.deleteFile();
        LowlineProcessor restored;
        restored.prepareToPlay(48000, 256);
        restored.openPreset(portablePreset); wait(restored);
        require(restored.slotName(LowlineProcessor::Amp) != "(empty)" && restored.slotName(LowlineProcessor::Room) != "(empty)",
                "portable preset restores models and IRs without original paths");
        settle(restored, b, .2f);
        require(std::abs(b.getSample(0, 255) - .1f) < .002f, "portable NAM still produces the expected output");
        require(restored.midiDescription().contains("CC 45"), "MIDI learn survives preset export and restore");
        MidiBuffer expression;
        expression.addEvent(MidiMessage::controllerEvent(1, 45, 0), 0);
        restored.processBlock(b, expression);
        require(restored.state.getRawParameterValue("blend")->load() == 0, "learned expression CC controls the target");
        auto assignedPreset = temp.getChildFile("assigned.lowline");
        set(restored, "blend", 33); restored.savePreset(assignedPreset);
        restored.assignProgram(8, assignedPreset); set(restored, "blend", 77);
        MidiBuffer pc;
        pc.addEvent(MidiMessage::programChange(1, 8), 0); restored.processBlock(b, pc);
        for (int i = 0; i < 200 && std::abs(restored.state.getRawParameterValue("blend")->load() - 33) > .01f; ++i)
        {
            Thread::sleep(10);
            restored.handleUpdateNowIfNeeded(); // the preset is applied on the message thread
        }
        wait(restored);
        require(std::abs(restored.state.getRawParameterValue("blend")->load() - 33) < .01f &&
                restored.programAssignment(8) == assignedPreset.getFullPathName(),
                "Program Change recalls a user preset without erasing its bank mapping");
        set(restored, "sync", 1); set(restored, "hostSync", 0); set(restored, "tempo", 120); set(restored, "division", 2);
        MidiBuffer tempoMessages; restored.processBlock(b, tempoMessages);

        require(std::abs(restored.effectiveDelay.load() - 375) < .01f, "processor applies manual tempo sync");
        set(restored, "tunerOn", 1); set(restored, "tunerMute", 1);
        settle(restored, b, .2f);
        require(b.getMagnitude(0, 256) < .00001f, "tuner mute fades the output to silence");
        auto quieter = temp.getChildFile("quieter.nam");
        quieter.replaceWithText(R"({"version":"0.5.4","architecture":"Linear","config":{"receptive_field":1,"bias":false},"weights":[0.25],"sample_rate":48000,"metadata":{"loudness":-24.0}})");
        LowlineProcessor transitions;
        transitions.prepareToPlay(48000, 256);
        set(transitions, "compOn", 0); set(transitions, "output", 0); set(transitions, "blend", 100);
        transitions.loadFile(LowlineProcessor::Amp, model); wait(transitions); settle(transitions, b, .2f);
        transitions.loadFile(LowlineProcessor::Amp, quieter); wait(transitions);
        float last = .1f, maxJump = 0;
        MidiBuffer transitionMessages;
        for (int block = 0; block < 30; ++block)
        {
            for (int channel = 0; channel < 2; ++channel)
                for (int sample = 0; sample < 256; ++sample) b.setSample(channel, sample, .2f);
            transitions.processBlock(b, transitionMessages);
            for (int sample = 0; sample < 256; ++sample)
            {
                const float current = b.getSample(0, sample);
                maxJump = std::max(maxJump, std::abs(current - last)); last = current;
            }
        }
        require(maxJump < .001f && std::abs(last - .05f) < .001f,
                "NAM replacement crossfades instead of stepping or bypassing the rig");
        set(transitions, "normOn", 1); set(transitions, "normTarget", -18);
        settle(transitions, b, .2f);
        require(transitions.normalisationAvailable[1] && std::abs(b.getSample(0, 255) - .05f * lowline::db(6)) < .001f,
                "NAM metadata normalisation applies the bounded target gain");
        // A legacy tree without new parameters must reset them to neutral defaults.
        auto legacy = transitions.state.copyState();
        for (int i = legacy.getNumChildren(); --i >= 0;)
            if (legacy.getChild(i).getProperty("id").toString() == "normOn") legacy.removeChild(i, nullptr);
        auto legacyXml = legacy.createXml(); MemoryBlock legacyBytes;
        AudioProcessor::copyXmlToBinary(*legacyXml, legacyBytes);
        transitions.setStateInformation(legacyBytes.getData(), int(legacyBytes.getSize())); wait(transitions);
        require(transitions.state.getRawParameterValue("normOn")->load() == 0,
                "old presets receive neutral defaults for new parameters");
        LowlineCaptureSlot rateChange;
        auto fixedRate = std::make_unique<LowlineCapture>();
        fixedRate->model = std::make_unique<nam::DSP>(1, 1, 48000);
        fixedRate->prepare(48000, 256); rateChange.publish(std::move(fixedRate));
        require(!rateChange.prepare(44100, 256), "fixed-rate capture reports incompatible device configuration");
        for (int ch = 0; ch < 2; ++ch)
            for (int i = 0; i < 256; ++i) b.setSample(ch, i, .2f);
        rateChange.process(b, 256, true, false, -18);
        require(std::abs(b.getSample(0, 255) - .2f) < .001f,
                "incompatible capture passes through safely after a device rate change");
        std::atomic<bool> audioValid{true};
        std::thread audioThread([&]
        {
            AudioBuffer<float> audio(2, 256); MidiBuffer messages;
            for (int block = 0; block < 350; ++block)
            {
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < 256; ++i) audio.setSample(ch, i, .2f);
                transitions.processBlock(audio, messages);
                for (int ch = 0; ch < 2; ++ch)
                    for (int i = 0; i < 256; ++i)
                        if (!std::isfinite(audio.getSample(ch, i)) || std::abs(audio.getSample(ch, i)) > .21f)
                            audioValid = false;
                Thread::sleep(1);
            }
        });
        for (int request = 0; request < 40; ++request)
        {
            transitions.loadFile(LowlineProcessor::Amp, request % 2 ? quieter : model);
            Thread::sleep(3);
        }
        audioThread.join(); wait(transitions);
        settle(transitions, b, .2f);
        require(audioValid && std::abs(b.getSample(0, 255) - .05f) < .001f,
                "concurrent rapid capture loading remains finite and reaches the last requested model");
        MemoryBlock invalidPortable;
        require(portablePreset.loadFileAsData(invalidPortable), "read portable fixture");
        auto invalidXml = AudioProcessor::getXmlFromBinary(invalidPortable.getData(), int(invalidPortable.getSize()));
        auto invalidTree = ValueTree::fromXml(*invalidXml);
        invalidTree.setProperty("ext0", ".exe", nullptr);
        auto invalidAssetXml = invalidTree.createXml(); invalidPortable.reset();
        AudioProcessor::copyXmlToBinary(*invalidAssetXml, invalidPortable);
        auto invalidAssetPreset = temp.getChildFile("invalid-asset.lowline");
        invalidAssetPreset.replaceWithData(invalidPortable.getData(), invalidPortable.getSize());
        set(restored, "blend", 44); restored.openPreset(invalidAssetPreset);
        require(restored.state.getRawParameterValue("blend")->load() == 44 && restored.status().contains("Invalid"),
                "invalid portable asset is rejected before changing controls");
        model.deleteFile();
        p.loadFile(LowlineProcessor::Amp, model);
        wait(p);
        require(p.status().contains("File not found"), "missing asset is reported explicitly");
        auto otherModel = temp.getChildFile("other.nam");
        otherModel.replaceWithText(
            R"({"version":"0.5.4","architecture":"Linear","config":{"receptive_field":1,"bias":false},"weights":[0.25],"sample_rate":48000})");
        p.loadFile(LowlineProcessor::Amp, otherModel);
        wait(p);
        auto missingTree = p.state.copyState();
        missingTree.setProperty("file1", model.getFullPathName(), nullptr);
        auto missingXml = missingTree.createXml();
        MemoryBlock missingState;
        AudioProcessor::copyXmlToBinary(*missingXml, missingState);
        p.setStateInformation(missingState.getData(), (int)missingState.getSize());
        wait(p);
        require(p.slotName(LowlineProcessor::Amp) == "(empty)" && p.status().contains("File not found"),
                "missing preset asset clears stale model and preserves loader error");
        LowlineProcessor bypassProcessor;
        bypassProcessor.prepareToPlay(48000, 256);
        AudioBuffer<float> bypassAudio(2, 256);
        bypassAudio.clear();
        bypassAudio.setSample(0, 0, .2f);
        MidiBuffer none;
        bypassProcessor.processBlockBypassed(bypassAudio, none);
        require(bypassAudio.getSample(0, 0) == 0 && std::abs(bypassAudio.getSample(0, 48) - .2f) < .00001f,
                "host bypass retains declared latency");
        auto rightIR = temp.getChildFile("right-only.wav");
        {
            WavAudioFormat wav;
            std::unique_ptr<AudioFormatWriter> w(
                wav.createWriterFor(new FileOutputStream(rightIR), 48000, 2, 24, {}, 0));
            AudioBuffer<float> impulse(2, 64);
            impulse.clear();
            impulse.setSample(1, 0, 1);
            require(w && w->writeFromAudioSampleBuffer(impulse, 0, 64), "write asymmetric stereo IR");
        }
        LowlineProcessor mono;
        auto layout = mono.getBusesLayout();
        layout.inputBuses.set(0, AudioChannelSet::mono());
        layout.outputBuses.set(0, AudioChannelSet::mono());
        require(mono.setBusesLayout(layout), "mono bus layout is accepted");
        mono.prepareToPlay(48000, 256);
        set(mono, "compOn", 0);
        set(mono, "output", 0);
        set(mono, "blend", 100);
        mono.loadFile(LowlineProcessor::Cab, rightIR);
        wait(mono);
        AudioBuffer<float> monoAudio(1, 256);
        for (int pass = 0; pass < 80; ++pass)
        {
            for (int j = 0; j < 256; ++j)
                monoAudio.setSample(0, j, .2f);
            mono.processBlock(monoAudio, none);
        }
        require(std::abs(monoAudio.getSample(0, 255) - .0125f) < .001f,
                "mono output includes right-hand cabinet channel");
        layout.inputBuses.set(0, AudioChannelSet::stereo());
        require(mono.setBusesLayout(layout), "stereo-input mono-output bus layout is accepted");
        mono.prepareToPlay(48000, 256);
        AudioBuffer<float> mixedLayout(2, 256);
        for (int pass = 0; pass < 80; ++pass)
        {
            for (int j = 0; j < 256; ++j)
            {
                mixedLayout.setSample(0, j, .2f);
                mixedLayout.setSample(1, j, 0);
            }
            mono.processBlock(mixedLayout, none);
        }
        require(std::abs(mixedLayout.getSample(0, 255) - .0125f) < .001f,
                "mono fold uses output bus rather than input buffer width");
        {
            LowlineCaptureSlot roomSlot;roomSlot.prepare(48000,256);
            auto oldRoom=std::make_unique<LowlineCapture>();oldRoom->ir=std::make_unique<dsp::Convolution>();oldRoom->tailSeconds=.5;
            AudioBuffer<float> echo(2,24000);echo.clear();echo.setSample(0,12000,.5f);echo.setSample(1,12000,.5f);
            oldRoom->ir->loadImpulseResponse(std::move(echo),48000,dsp::Convolution::Stereo::yes,dsp::Convolution::Trim::no,dsp::Convolution::Normalise::no);
            oldRoom->prepare(48000,256);roomSlot.publish(std::move(oldRoom));
            AudioBuffer<float> signal(2,256);for(int j=0;j<50;++j){signal.clear();roomSlot.process(signal,256,true,false,0);}
            signal.clear();signal.setSample(0,0,.2f);signal.setSample(1,0,.2f);roomSlot.process(signal,256,true,false,0);
            for(int j=0;j<10;++j){signal.clear();roomSlot.process(signal,256,true,false,0);}
            auto replacement=std::make_unique<LowlineCapture>();replacement->prepare(48000,256);roomSlot.publish(std::move(replacement));
            float tailPeak=0;for(int j=0;j<65;++j){signal.clear();roomSlot.process(signal,256,true,false,0);tailPeak=std::max(tailPeak,signal.getMagnitude(0,256));}
            require(tailPeak>.08f,"outgoing room IR retains delayed energy beyond the 50ms transition");
        }
        // 0.3: offline pitch preservation, playback/loop, recorder, A/B and editor workflows.
        AudioBuffer<float> referenceTone(2,48000);
        for (int i=0;i<48000;++i) for(int ch=0;ch<2;++ch)
            referenceTone.setSample(ch,i,.2f*std::sin(2*lowline::pi*220*i/48000));
        for(double speed : {.5, .75, 1.25, 1.5})
        {
            auto stretched=LowlineStudio::stretch(referenceTone,speed,48000);
            require(std::abs(stretched.getNumSamples()-int(48000/speed))<=1,"stretch length follows speed");
            int crossings=0;const int start=4000,end=stretched.getNumSamples()-4000;
            for(int i=start+1;i<end;++i)
                if(stretched.getSample(0,i-1)<=0&&stretched.getSample(0,i)>0)++crossings;
            const double frequency=double(crossings)*48000/(end-start);
            require(std::abs(frequency-220)<3,"offline stretch retains tone pitch at slow and fast speeds");
            require(stretched.getMagnitude(0,stretched.getNumSamples())<.201f,"stretch remains bounded");
        }
        const auto backingFile=temp.getChildFile("backing.wav");
        {
            WavAudioFormat wav;std::unique_ptr<AudioFormatWriter> writer(wav.createWriterFor(new FileOutputStream(backingFile),48000,2,32,{},0));
            require(writer&&writer->writeFromAudioSampleBuffer(referenceTone,0,48000),"write backing fixture");
        }
        {
            LowlineStudio studio;studio.prepare(48000,256);studio.load(backingFile,1);
            for(int i=0;i<500&&studio.loading;++i)Thread::sleep(10);
            require(!studio.loading&&std::abs(studio.duration-1)<.001,"worker decodes backing duration");
            AudioBuffer<float> block(2,256);block.clear();studio.playing=true;studio.mix(block,0,256,2);
            require(block.getMagnitude(0,256)>.05f&&studio.position>0,"backing playback mixes and advances");
            studio.loopStart=.1;studio.loopEnd=.2;studio.looping=true;studio.seek(.19);
            for(int i=0;i<50;++i){block.clear();studio.mix(block,0,256,2);}
            require(studio.position>=.1&&studio.position<.201,"backing loop wraps inside requested range");
            studio.playing=false;studio.click=true;studio.accent=2;studio.bpm=120;
            double clickEnergy=0;for(int i=0;i<100;++i){block.clear();studio.mix(block,0,256,2);clickEnergy+=block.getRMSLevel(0,0,256);}
            require(clickEnergy>.01&&studio.beatDisplay==2,"metronome produces beats with 2/4 accent mode");
            studio.click=false;auto take=temp.getChildFile("take.wav");
            require(studio.startRecording(take),"dual recording starts");
            for(int i=0;i<12;++i)
            {
                for(int ch=0;ch<2;++ch)FloatVectorOperations::fill(block.getWritePointer(ch),.2f,256);
                studio.captureInput(block,0,256,0,2);block.applyGain(.5f);studio.record(block,0,256,2);
            }
            studio.stopRecording();
            AudioFormatManager fm;fm.registerBasicFormats();
            std::unique_ptr<AudioFormatReader> di(fm.createReaderFor(temp.getChildFile("take-DI.wav")));
            std::unique_ptr<AudioFormatReader> fx(fm.createReaderFor(temp.getChildFile("take-FX.wav")));
            require(di&&fx&&di->lengthInSamples==3072&&fx->lengthInSamples==3072,"recording closes two complete aligned WAV files");
            require(di->numChannels==1&&fx->numChannels==2&&di->usesFloatingPointData&&fx->usesFloatingPointData,"recording uses mono DI and stereo 32-bit float FX");
            AudioBuffer<float> read(2,256);di->read(&read,0,256,0,true,false);require(std::abs(read.getSample(0,128)-.2f)<1e-5,"DI preserves input before effects");
            fx->read(&read,0,256,0,true,true);require(std::abs(read.getSample(0,128)-.1f)<1e-5,"FX captures processed bass independently");
            require(!studio.startRecording(take),"recorder refuses to overwrite an existing take");
        }
        {
            LowlineProcessor comparison;comparison.prepareToPlay(48000,256);AudioBuffer<float> block(2,256);MidiBuffer messages;
            set(comparison,"compOn",0);set(comparison,"blend",0);set(comparison,"output",0);
            comparison.setPresetDetails("Walking bass","jazz,clean","A clean practice sound");comparison.markClean();
            require(!comparison.isDirty(),"saved fingerprint initially clean");
            comparison.measuredRms=-20;comparison.storeComparison(0);set(comparison,"bass",6);comparison.measuredRms=-14;comparison.storeComparison(1);
            require(comparison.isDirty(),"parameter edit marks preset dirty");
            require(comparison.recallComparison(0),"A comparison can be recalled");wait(comparison);
            require(std::abs(comparison.state.getRawParameterValue("bass")->load())<.001,"A recall restores controls");
            comparison.measuredRms=-14;comparison.matchComparison(0);require(std::abs(comparison.compareTrim+6)<.01,"RMS match computes bounded reference gain");
            comparison.measuredRms=-100;comparison.compareTrim=0;comparison.matchComparison(0);require(comparison.compareTrim==0,"silence cannot cause positive auto gain");
            comparison.measureLevel();for(int j=0;j<940;++j){for(int ch=0;ch<2;++ch)FloatVectorOperations::fill(block.getWritePointer(ch),.1f,256);comparison.processBlock(block,messages);}
            require(comparison.measureRemaining==0&&std::abs(comparison.measuredRms+20)<.2,"five-second meter measures actual processed RMS");
            comparison.undo.clearUndoHistory();comparison.undo.beginNewTransaction("Bass");set(comparison,"bass",4);comparison.state.copyState();
            require(comparison.undo.canUndo(),"parameter changes enter undo history");comparison.undo.undo();
            require(std::abs(comparison.state.getRawParameterValue("bass")->load())<.01,"undo restores parameter");
            auto saved=temp.getChildFile("named.lowline");comparison.savePreset(saved);comparison.setPresetDetails("Other","","");comparison.openPreset(saved);wait(comparison);
            require(comparison.presetName()=="Walking bass"&&comparison.presetTags().contains("jazz"),"preset name tags and description persist");
        }
        LowlineProcessor preview;
        preview.prepareToPlay(48000, 256);
        std::unique_ptr<AudioProcessorEditor> e(preview.createEditor());
        require(e->getWidth() == 1180 && e->getHeight() == 830, "native editor opens with all controls");
        LowlineWorkbench *practiceView = nullptr;
        for (auto *child : e->getChildren()) if (auto *view = dynamic_cast<LowlineWorkbench *>(child)) practiceView = view;
        require(practiceView != nullptr,"practice workspace constructed");
        auto *accentChoice = dynamic_cast<ComboBox *>(practiceView->findChildWithID("practice-accents"));
        require(accentChoice && accentChoice->getSelectedId()==2 && accentChoice->getText().isNotEmpty(),"translated metronome retains selected accent");
        preview.setLanguage("EN");practiceView->showPage(2);
        require(accentChoice->getSelectedId()==2 && accentChoice->getText().contains("1"),"language change preserves metronome selection");
        preview.setLanguage("IT");practiceView->showPage(0);

        auto *helpButton = dynamic_cast<TextButton *>(e->findChildWithID("lowline-help"));
        require(helpButton != nullptr && bool(helpButton->onClick), "editor exposes its help action");
        LowlineHelpContent help(true);
        require(help.getTopicCount() == 18 && help.getDisplayedText().containsChar(0x00e8),
                "Italian offline guide contains eighteen topics and decoded UTF-8 text");
        for (int topic = 0; topic < 18; ++topic)
        {
            help.selectTopic(topic);
            require(help.getDisplayedText().length() > 400, "Italian help topic contains instructions");
        }
        help.setLanguage(false);
        require(help.getTopicCount() == 18 && help.getDisplayedText().contains("installation and updates"),
                "English guide switches language and preserves the selected topic");
        for (int topic = 0; topic < 18; ++topic)
        {
            help.selectTopic(topic);
            require(help.getDisplayedText().length() > 400, "English help topic contains instructions");
        }
        help.setLanguage(true);
        help.selectTopic(0);
        if (argc > 1 && String(argv[1]) == "--screenshot")
        {
            e->setVisible(true);
            auto img = e->createComponentSnapshot(e->getLocalBounds());
            File png = File::getCurrentWorkingDirectory().getChildFile("lowline-preview.png");
            FileOutputStream out(png);
            out.setPosition(0);
            out.truncate();
            PNGImageFormat fmt;
            fmt.writeImageToStream(img, out);
            std::cout << png.getFullPathName() << "\n";
            for (const auto *page : {"tools", "midi"})
            {
                auto *button = dynamic_cast<TextButton *>(e->findChildWithID(String("tab-") + page));
                require(button != nullptr, "new page navigation exists");
                button->onClick();
                FileOutputStream pageOut(File::getCurrentWorkingDirectory().getChildFile(String("lowline-") + page + "-preview.png"));
                pageOut.setPosition(0); pageOut.truncate();
                fmt.writeImageToStream(e->createComponentSnapshot(e->getLocalBounds()), pageOut);
            }
            auto *workspaceButton = dynamic_cast<TextButton *>(e->findChildWithID("tab-workspace"));
            require(workspaceButton != nullptr, "workspace navigation exists");
            workspaceButton->onClick();
            LowlineWorkbench *workspace = nullptr;
            for (auto *child : e->getChildren()) if (auto *w = dynamic_cast<LowlineWorkbench *>(child)) workspace = w;
            require(workspace != nullptr, "workspace is a native component");
            for (int page = 0; page < 5; ++page)
            {
                workspace->showPage(page);
                FileOutputStream pageOut(File::getCurrentWorkingDirectory().getChildFile("lowline-workspace-"+String(page)+".png"));
                pageOut.setPosition(0); pageOut.truncate();
                fmt.writeImageToStream(e->createComponentSnapshot(e->getLocalBounds()), pageOut);
            }
            File helpPng = File::getCurrentWorkingDirectory().getChildFile("lowline-help-preview.png");
            FileOutputStream helpOut(helpPng);
            helpOut.setPosition(0);
            helpOut.truncate();
            fmt.writeImageToStream(help.createComponentSnapshot(help.getLocalBounds()), helpOut);
        }
        {
            // 0.2.1 regressions: preset reload gap, momentary switches, TAP, host bypass mute, idle captures.
            auto halfModel = temp.getChildFile("half.nam");
            halfModel.replaceWithText(
                R"({"version":"0.5.4","architecture":"Linear","config":{"receptive_field":1,"bias":false},"weights":[0.5],"sample_rate":48000})");
            LowlineProcessor r;
            r.prepareToPlay(48000, 256);
            for (auto *id : {"compOn", "pedalOn", "cabOn", "delayOn", "reverbOn"}) set(r, id, 0);
            set(r, "blend", 100); set(r, "output", 0);
            r.loadFile(LowlineProcessor::Amp, halfModel); wait(r);
            AudioBuffer<float> a(2, 256);
            settle(r, a, .1f);
            MemoryBlock same; r.getStateInformation(same);
            r.setStateInformation(same.getData(), int(same.getSize()));
            float worst = 0;
            MidiBuffer none2;
            for (int k = 0; k < 60; ++k)
            {
                for (int c = 0; c < 2; ++c) for (int i = 0; i < 256; ++i) a.setSample(c, i, .1f);
                r.processBlock(a, none2);
                worst = std::max(worst, a.getMagnitude(0, 0, 256));
                Thread::sleep(3);
            }
            wait(r);
            require(worst < .06f, "reloading the same preset keeps the amp playing (no raw DI gap)");

            set(r, "compOn", 0);
            r.momentarySwitches = true;
            auto footswitch = [&](int value)
            {
                MidiBuffer event; // processBlock consumes the buffer
                event.addEvent(MidiMessage::controllerEvent(1, 20, value), 0);
                r.processBlock(a, event);
            };
            footswitch(127); footswitch(0);
            require(r.state.getRawParameterValue("compOn")->load() > .5f, "momentary footswitch: press turns on, release keeps it");
            footswitch(127); footswitch(0);
            require(r.state.getRawParameterValue("compOn")->load() < .5f, "momentary footswitch: second press turns off");
            MemoryBlock withMode; r.getStateInformation(withMode);
            LowlineProcessor r2; r2.setStateInformation(withMode.getData(), int(withMode.getSize()));
            require(r2.momentarySwitches.load(), "footswitch mode is saved with the MIDI map");
            r.momentarySwitches = false;

            set(r, "sync", 0);
            // Explicit timestamps: CI runners (macOS especially) oversleep, so sleep-based taps are flaky.
            const double t0 = Time::getMillisecondCounterHiRes() + 10000;
            r.tapTempoAt(t0); r.tapTempoAt(t0 + 500); r.tapTempoAt(t0 + 1000);
            require(std::abs(r.state.getRawParameterValue("time")->load() - 500) < 1, "TAP without sync sets the delay time");
            r.tapTempoAt(t0 + 1520); r.tapTempoAt(t0 + 2000);
            require(std::abs(r.state.getRawParameterValue("tempo")->load() - 120) < 2, "TAP averages slightly uneven taps");
            r.tapTempoAt(t0 + 5000);
            require(std::abs(r.state.getRawParameterValue("tempo")->load() - 120) < 2, "a tap after a long pause is ignored");

            set(r, "tunerOn", 1); set(r, "tunerMute", 1);
            AudioBuffer<float> hb(2, 256);
            for (int k = 0; k < 4; ++k)
            {
                for (int c = 0; c < 2; ++c) for (int i = 0; i < 256; ++i) hb.setSample(c, i, .2f);
                r.processBlockBypassed(hb, none2);
            }
            require(std::abs(hb.getSample(0, 255) - .2f) < .001f, "host bypass is never muted by the tuner");
            set(r, "tunerOn", 0); set(r, "tunerMute", 0);

            set(r, "ampOn", 0);
            settle(r, a, .1f);
            set(r, "ampOn", 1);
            settle(r, a, .1f);
            require(std::abs(a.getSample(0, 255) - .05f) < .002f, "a capture switched back on after idling plays normally");
        }
        temp.deleteRecursively();
        std::cout << "All audio checks passed.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FAIL " << e.what() << "\n";
        return 1;
    }
}
