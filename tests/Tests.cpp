#include "../Source/Processor.h"
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
        LowlineProcessor preview;
        preview.prepareToPlay(48000, 256);
        std::unique_ptr<AudioProcessorEditor> e(preview.createEditor());
        require(e->getWidth() == 1180 && e->getHeight() == 830, "native editor opens with all controls");
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
