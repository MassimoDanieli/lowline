#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>
#include <cmath>

// Decimated SPSC input; all pitch analysis is done on a background thread.
class LowlineTuner : private juce::Thread
{
  public:
    LowlineTuner() : Thread("Radice tuner") {}
    ~LowlineTuner() override { stopThread(-1); }
    std::atomic<float> frequency{0};
    void prepare(double sr)
    {
        stopThread(-1);
        fifo.reset();
        factor = std::max(1, int(sr / 6000));
        analysisRate = sr / factor;
        count = 0; filter1 = filter2 = 0;
        coefficient = 1 - std::exp(-2 * 3.141592653589793 * 1200 / sr);
        frequency = 0;
        startThread();
    }
    void push(float x)
    {
        filter1 += coefficient * (x - filter1);
        filter2 += coefficient * (filter1 - filter2);
        if (++count < factor) return;
        count = 0;
        int a, n, b, m;
        fifo.prepareToWrite(1, a, n, b, m);
        if (n) { samples[size_t(a)] = filter2; fifo.finishedWrite(1); }
    }
    static float detect(const float *x, int size, double sr)
    {
        const int window = size / 2;
        const int maxLag = std::min(window - 2, int(sr / 25));
        const int minLag = std::max(2, int(sr / 500));
        double mean = 0, power = 0;
        for (int i = 0; i < size; ++i) mean += x[i];
        mean /= size;
        for (int i = 0; i < size; ++i) power += (x[i] - mean) * (x[i] - mean);
        if (power / size < 1.e-7) return 0;
        std::array<double, 1024> difference{};
        if (maxLag >= int(difference.size())) return 0;
        double sum = 0;
        for (int lag = 1; lag <= maxLag; ++lag)
        {
            double d = 0;
            for (int i = 0; i < window; ++i) { double v = x[i] - x[i + lag]; d += v * v; }
            sum += d;
            difference[size_t(lag)] = sum > 0 ? d * lag / sum : 1;
        }
        for (int lag = minLag; lag < maxLag - 1; ++lag)
            if (difference[size_t(lag)] < .15)
            {
                while (lag + 1 < maxLag && difference[size_t(lag + 1)] < difference[size_t(lag)]) ++lag;
                const double a = difference[size_t(lag - 1)], b = difference[size_t(lag)],
                             c = difference[size_t(lag + 1)], denominator = a - 2 * b + c;
                const double offset = std::abs(denominator) > 1.e-12 ? .5 * (a - c) / denominator : 0;
                return float(sr / (lag + std::clamp(offset, -.5, .5)));
            }
        return 0;
    }
  private:
    void run() override
    {
        std::array<float, 2048> frame{};
        int filled = 0;
        auto last = juce::Time::getMillisecondCounter();
        while (!threadShouldExit())
        {
            int a, n, b, m;
            fifo.prepareToRead(std::min(512, fifo.getNumReady()), a, n, b, m);
            auto consume = [&](int start, int length)
            {
                for (int i = 0; i < length; ++i)
                {
                    frame[size_t(filled++)] = samples[size_t(start + i)];
                    if (filled == int(frame.size()))
                    {
                        frequency = detect(frame.data(), filled, analysisRate);
                        std::move(frame.begin() + 512, frame.end(), frame.begin());
                        filled -= 512;
                    }
                }
            };
            consume(a, n); consume(b, m); fifo.finishedRead(n + m);
            if (n + m) last = juce::Time::getMillisecondCounter();
            else if (juce::Time::getMillisecondCounter() - last > 250) { frequency = 0; filled = 0; }
            wait(10);
        }
    }
    juce::AbstractFifo fifo{8192};
    std::array<float, 8192> samples{};
    int factor = 8, count = 0;
    float filter1 = 0, filter2 = 0, coefficient = 0;
    double analysisRate = 6000;
};
