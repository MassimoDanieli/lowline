#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>
namespace lowline
{
constexpr float pi = 3.14159265358979323846f;
inline float db(float d)
{
    return std::pow(10.0f, d * .05f);
}
inline float toDb(float a)
{
    return 20 * std::log10(std::max(a, 1.e-9f));
}
struct Slew
{
    float value = 0, coefficient = 0;
    void prepare(double sr, float initial)
    {
        value = initial;
        coefficient = std::exp(-1.0f / (float(sr) * .015f));
    }
    float next(float target) { return value = target + coefficient * (value - target); }
};
struct Compressor
{
    float envelope = 0, reduction = 0;
    double sr = 48000;
    void prepare(double rate)
    {
        sr = rate;
        envelope = reduction = 0;
    }
    float tick(float x, float threshold, float ratio, float attack, float release, float makeup)
    {
        float level = std::abs(x);
        float c = std::exp(-1.0f / (float(sr) * .001f * (level > envelope ? attack : release)));
        envelope = level + c * (envelope - level);
        float over = toDb(envelope) - threshold;
        constexpr float knee = 6;
        float curve = over < -knee / 2  ? 0
                      : over > knee / 2 ? over
                                        : (over + knee / 2) * (over + knee / 2) / (2 * knee);
        reduction = curve * (1 - 1 / ratio);
        return x * db(makeup - reduction);
    }
};
struct Gate
{
    float envelope = 0, gain = 1;
    int hold = 0;
    double rate = 48000;
    void prepare(double sr) { rate = sr; envelope = 0; gain = 1; hold = 0; }
    float tick(float x, bool enabled, float threshold, float releaseMs)
    {
        const float level = std::abs(x);
        const float detector = std::exp(-1.f / (float(rate) * (level > envelope ? .001f : .05f)));
        envelope = level + detector * (envelope - level);
        if (envelope > db(threshold)) hold = int(rate * .04);
        else if (envelope < db(threshold - 3) && hold > 0) --hold;
        const float target = !enabled || hold > 0 ? 1.f : 0.f;
        const float speed = target > gain ? .002f : std::max(.01f, releaseMs * .001f);
        gain = target + std::exp(-1.f / (float(rate) * speed)) * (gain - target);
        return x * gain;
    }
};
// Two cascaded one-pole low-pass stages provide a 12 dB/octave clean branch.
// The processed branch remains full-range.
struct CleanCrossover
{
    float a = 0, b = 0;
    void reset() { a = b = 0; }
    float low(float x, double sr, float hz)
    {
        float c = 1 - std::exp(-2 * pi * std::min(hz, float(sr) * .45f) / float(sr));
        a += c * (x - a); b += c * (a - b); return b;
    }
};
inline float tempoDelay(float bpm, int division)
{
    constexpr float beats[] = {.25f, .5f, .75f, 1.f, 1.5f, 2.f};
    return 60000.f / std::clamp(bpm, 30.f, 300.f) * beats[std::clamp(division, 0, 5)];
}
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    float tick(float x)
    {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void peak(double sr, float f, float gain, float q = .707f)
    {
        f = std::clamp(f, 10.f, float(sr) * .45f);
        float a = std::pow(10.f, gain / 40), w = 2 * pi * f / float(sr), al = std::sin(w) / (2 * q),
              co = std::cos(w), d = 1 + al / a;
        b0 = (1 + al * a) / d;
        b1 = -2 * co / d;
        b2 = (1 - al * a) / d;
        a1 = -2 * co / d;
        a2 = (1 - al / a) / d;
    }
    // RBJ shelves, slope 1: half the gain (in dB) at f, the full gain well beyond it.
    void lowShelf(double sr, float f, float gain)
    {
        f = std::clamp(f, 10.f, float(sr) * .45f);
        const float a = std::pow(10.f, gain / 40), w = 2 * pi * f / float(sr), co = std::cos(w),
                    al = std::sin(w) / 2 * std::sqrt(2.f), s = 2 * std::sqrt(a) * al, d = (a + 1) + (a - 1) * co + s;
        b0 = a * ((a + 1) - (a - 1) * co + s) / d;
        b1 = 2 * a * ((a - 1) - (a + 1) * co) / d;
        b2 = a * ((a + 1) - (a - 1) * co - s) / d;
        a1 = -2 * ((a - 1) + (a + 1) * co) / d;
        a2 = ((a + 1) + (a - 1) * co - s) / d;
    }
    void highShelf(double sr, float f, float gain)
    {
        f = std::clamp(f, 10.f, float(sr) * .45f);
        const float a = std::pow(10.f, gain / 40), w = 2 * pi * f / float(sr), co = std::cos(w),
                    al = std::sin(w) / 2 * std::sqrt(2.f), s = 2 * std::sqrt(a) * al, d = (a + 1) - (a - 1) * co + s;
        b0 = a * ((a + 1) + (a - 1) * co + s) / d;
        b1 = -2 * a * ((a - 1) + (a + 1) * co) / d;
        b2 = a * ((a + 1) + (a - 1) * co - s) / d;
        a1 = 2 * ((a - 1) - (a + 1) * co) / d;
        a2 = ((a + 1) - (a - 1) * co - s) / d;
    }
};
// tanh saturation with first-order antiderivative anti-aliasing (ADAA): far less aliasing
// than a plain tanh at high boost, no oversampling filters and only half a sample of delay.
struct AdaaTanh
{
    double x1 = 0, f1 = 0;
    void reset() { x1 = f1 = 0; }
    static double logCosh(double x)
    {
        const double a = std::abs(x);
        return a + std::log1p(std::exp(-2 * a)) - 0.6931471805599453;
    }
    float tick(float input)
    {
        const double x = input, f = logCosh(x), d = x - x1;
        const double y = std::abs(d) < 1.e-5 ? std::tanh(.5 * (x + x1)) : (f - f1) / d;
        x1 = x; f1 = f;
        return float(y);
    }
};
struct Delay
{
    std::vector<float> l, r;
    size_t pos = 0;
    float dampL = 0, dampR = 0, lowL = 0, lowR = 0;
    double sr = 48000;
    Slew time;
    void prepare(double rate)
    {
        sr = rate;
        l.assign(size_t(sr * 6.1) + 2, 0);
        r = l;
        pos = 0;
        dampL = dampR = lowL = lowR = 0;
        time.prepare(sr, float(sr * .38));
    }
    float read(const std::vector<float> &v, float samples) const
    {
        double p = double(pos) - samples;
        if (p < 0)
            p += v.size();
        auto i = size_t(p);
        auto j = (i + 1) % v.size();
        return v[i] + float(p - i) * (v[j] - v[i]);
    }
    void tick(float &left, float &right, float ms, float feedback, float mix, bool ping, float highcut,
              float send = 1, float lowcut = 0)
    {
        float n = time.next(std::clamp(ms * .001f * float(sr), 1.f, float(l.size() - 2)));
        float dl = read(l, n), dr = read(r, n);
        if (lowcut > 0)
        {
            float hp = 1 - std::exp(-2 * pi * lowcut / float(sr));
            lowL += hp * (dl - lowL); lowR += hp * (dr - lowR);
            dl -= lowL; dr -= lowR;
        }
        float c = 1 - std::exp(-2 * pi * highcut / float(sr));
        dampL += c * (dl - dampL);
        dampR += c * (dr - dampR);
        l[pos] = left * send + feedback * (ping ? dampR : dampL);
        // Ping-pong injects the source on the left only, so the repeats really alternate.
        r[pos] = (ping ? 0.f : right * send) + feedback * (ping ? dampL : dampR);
        left += mix * dl;
        right += mix * dr;
        pos = (pos + 1) % l.size();
    }
};
struct DryDelay
{
    std::vector<float> ring;
    size_t pos = 0;
    void prepare(double sr)
    {
        ring.assign(size_t(sr * .025) + 4, 0);
        pos = 0;
    }
    float tick(float x, float samples)
    {
        ring[pos] = x;
        double p = double(pos) - std::clamp(samples, 0.f, float(ring.size() - 2));
        if (p < 0)
            p += ring.size();
        auto i = size_t(p);
        float y = ring[i] + float(p - i) * (ring[(i + 1) % ring.size()] - ring[i]);
        pos = (pos + 1) % ring.size();
        return y;
    }
};
// Stereo-linked peak limiter. One millisecond of lookahead is reported to the host.
// The detector covers the entire lookahead window; release is smoothed, attack anticipates peaks.
struct PeakLimiter
{
    std::vector<float> left, right, peaks;
    std::vector<uint64_t> indices;
    uint64_t clock = 0;
    size_t head = 0, tail = 0;
    int delaySamples = 0;
    float gain = 1, release = 0;
    void prepare(double sr)
    {
        delaySamples = std::max(1, int(std::round(sr * .001)));
        const auto size = size_t(delaySamples + 2);
        left.assign(size, 0);
        right.assign(size, 0);
        peaks.assign(size, 0);
        indices.assign(size, 0);
        clock = 0;
        head = tail = 0;
        gain = 1;
        release = std::exp(-1.f / (float(sr) * .08f));
    }
    void tick(float &l, float &r)
    {
        if (!std::isfinite(l))
            l = 0;
        if (!std::isfinite(r))
            r = 0;
        const size_t size = left.size(), write = size_t(clock % size);
        left[write] = l;
        right[write] = r;
        while (head != tail && indices[head] + uint64_t(delaySamples) < clock)
            head = (head + 1) % size;
        const float peak = std::max(std::abs(l), std::abs(r));
        while (head != tail && peaks[(tail + size - 1) % size] <= peak)
            tail = (tail + size - 1) % size;
        indices[tail] = clock;
        peaks[tail] = peak;
        tail = (tail + 1) % size;
        const float maximum = head == tail ? 0 : peaks[head];
        const float target = maximum > .98f ? .98f / maximum : 1;
        gain = target < gain ? target : target + release * (gain - target);
        const size_t read = size_t((clock + size - size_t(delaySamples)) % size);
        l = clock >= uint64_t(delaySamples) ? left[read] * gain : 0;
        r = clock >= uint64_t(delaySamples) ? right[read] * gain : 0;
        ++clock;
    }
};
inline float safety(float x)
{
    if (!std::isfinite(x))
        return 0;
    return std::clamp(x, -.98f, .98f);
}
} // namespace lowline
