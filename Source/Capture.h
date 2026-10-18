#pragma once
#include "Dsp.h"
#include <NAM/get_dsp.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

struct LowlineCapture
{
    std::unique_ptr<nam::DSP> model;
    std::unique_ptr<juce::dsp::Convolution> ir;
    float loudness = 0;
    bool hasLoudness = false, compatible = true;
    lowline::Slew normalGain;
    void prepare(double sr, int size)
    {
        normalGain.prepare(sr, 1);
        if (model)
        {
            const auto expected = model->GetExpectedSampleRate();
            compatible = expected < 0 || model->SupportsArbitrarySampleRate() || std::abs(expected - sr) < 1;
            if (compatible) model->Reset(sr, size);
        }
        if (ir) ir->prepare({sr, (juce::uint32)size, 2});
    }
    void process(juce::AudioBuffer<float> &input, juce::AudioBuffer<float> &output, int n,
                 bool normalise, float target)
    {
        if (model && compatible)
        {
            float *in[] = {input.getWritePointer(0)}, *out[] = {output.getWritePointer(0)};
            model->process(in, out, n);
            const float gain = normalise && hasLoudness ? lowline::db(std::clamp(target - loudness, -18.f, 18.f)) : 1;
            for (int i = 0; i < n; ++i) output.setSample(0, i, output.getSample(0, i) * normalGain.next(gain));
            output.copyFrom(1, 0, output, 0, 0, n);
        }
        else
        {
            for (int c = 0; c < 2; ++c) output.copyFrom(c, 0, input, c, 0, n);
            if (ir)
            {
                auto block = juce::dsp::AudioBlock<float>(output).getSubBlock(0, size_t(n));
                juce::dsp::ProcessContextReplacing<float> context(block);
                ir->process(context);
            }
        }
    }
};

// Single worker publishes immutable prepared engines. The audio thread owns active
// engines and returns retired ones through bounded atomic slots; deletion is off-thread.
class LowlineCaptureSlot
{
  public:
    ~LowlineCaptureSlot()
    {
        delete pending.exchange(nullptr); delete active; delete previous; collect();
    }
    void publish(std::unique_ptr<LowlineCapture> capture)
    {
        delete pending.exchange(capture.release(), std::memory_order_acq_rel);
        collect();
    }
    void collect()
    {
        for (auto &slot : retired) delete slot.exchange(nullptr, std::memory_order_acq_rel);
    }
    bool prepare(double sr, int block)
    {
        sampleRate = sr;
        nextAudio.setSize(2, block); oldAudio.setSize(2, block);
        if (active) active->prepare(sr, block);
        if (previous) previous->prepare(sr, block);
        // Called with audio stopped and publication locked. Pending engines prepared
        // for an older device configuration are reinitialised before use.
        if (auto *p = pending.load()) p->prepare(sr, block);
        enable.prepare(sr, 1);
        fadeRemaining = 0;
        const auto *waiting = pending.load();
        return (!active || active->compatible) && (!previous || previous->compatible) && (!waiting || waiting->compatible);
    }
    void process(juce::AudioBuffer<float> &audio, int n, bool enabled, bool normalise, float target)
    {
        if (previous && fadeRemaining == 0)
            for (auto &slot : retired)
            {
                LowlineCapture *empty = nullptr;
                if (slot.compare_exchange_strong(empty, previous)) { previous = nullptr; break; }
            }
        if (!previous && fadeRemaining == 0)
            if (auto *next = pending.exchange(nullptr, std::memory_order_acq_rel))
            {
                previous = active; active = next;
                fadeLength = std::max(1, int(sampleRate * .05)); fadeRemaining = fadeLength;
            }
        auto render = [&](LowlineCapture *capture, juce::AudioBuffer<float> &destination)
        {
            if (capture) capture->process(audio, destination, n, normalise, target);
            else for (int c = 0; c < 2; ++c) destination.copyFrom(c, 0, audio, c, 0, n);
        };
        render(active, nextAudio);
        if (fadeRemaining > 0) render(previous, oldAudio);
        for (int i = 0; i < n; ++i)
        {
            const float wet = enable.next(enabled ? 1.f : 0.f);
            const float mix = fadeRemaining > 0 ? 1.f - float(fadeRemaining--) / fadeLength : 1.f;
            for (int c = 0; c < 2; ++c)
            {
                const float value = mix < 1 ? (1 - mix) * oldAudio.getSample(c, i) + mix * nextAudio.getSample(c, i)
                                            : nextAudio.getSample(c, i);
                audio.setSample(c, i, (1 - wet) * audio.getSample(c, i) + wet * value);
            }
        }
    }
  private:
    std::atomic<LowlineCapture *> pending{nullptr};
    std::array<std::atomic<LowlineCapture *>, 16> retired{};
    LowlineCapture *active = nullptr, *previous = nullptr;
    juce::AudioBuffer<float> nextAudio, oldAudio;
    lowline::Slew enable;
    double sampleRate = 48000;
    int fadeRemaining = 0, fadeLength = 2400;
};
