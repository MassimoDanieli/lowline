#pragma once
#include <atomic>
#include <juce_audio_utils/juce_audio_utils.h>
#include <mutex>

// File decoding and time stretching are worker-only. Audio never waits for I/O.
class LowlineStudio : private juce::Thread
{
  public:
    LowlineStudio() : Thread("Lowline backing preparation"), disk("Lowline recording")
    {
        formats.registerBasicFormats();
        disk.startThread();
        startThread();
    }
    ~LowlineStudio() override
    {
        signalThreadShouldExit();
        notify();
        stopThread(-1);
        stopRecording();
        disk.stopThread(-1);
    }
    std::atomic<bool> playing{false}, looping{false}, click{false}, loading{false}, recording{false};
    std::atomic<float> backingGain{.35f}, clickGain{.12f}, bpm{80}, speed{1};
    std::atomic<int> accent{1}, beats{4}, beatDisplay{0};                   // 0 none, 1 first, 2 beats 2/4
    std::atomic<double> position{0}, duration{0}, loopStart{0}, loopEnd{0}; // seconds in original file
    // Time of a downbeat in the backing file (seconds, original speed); < 0 = metronome runs free.
    // While the backing plays, clicks follow its position: speed changes, seeks and loops stay in time.
    std::atomic<double> beatOne{-1};
    static constexpr double maxSeconds = 600, maxMemoryBytes = 768.0 * 1024 * 1024;
    std::atomic<int> dropped{0};
    std::atomic<int64_t> recordedSamples{0};
    std::atomic<double> sampleRate{48000};
    void prepare(double sr, int block)
    {
        stopRecording();
        sampleRate = sr;
        raw.setSize(1, block);
        wet.setSize(2, block);
        beatPhase = 0;
        beat = -1;
        clickRemaining = 0;
    }
    void load(const juce::File &f, float tempoRatio)
    {
        {
            std::lock_guard lock(requestLock);
            request = f;
            requestSpeed = juce::jlimit(.5f, 1.5f, tempoRatio);
            ++generation;
        }
        loading = true;
        notify();
    }
    juce::String status() const
    {
        std::lock_guard lock(requestLock);
        return message;
    }
    juce::File file() const
    {
        std::lock_guard lock(requestLock);
        return currentFile;
    }
    void seek(double seconds)
    {
        requestedPosition = std::max(0.0, seconds);
    }
    void setBeatOneHere() { beatOne = position.load(); }
    bool startRecording(const juce::File &base)
    {
        stopRecording();
        auto dryFile = base.getSiblingFile(base.getFileNameWithoutExtension() + "-DI.wav");
        auto wetFile = base.getSiblingFile(base.getFileNameWithoutExtension() + "-FX.wav");
        // Never overwrite an existing take, including a partially created pair.
        if (dryFile.exists() || wetFile.exists())
        {
            setMessage("Take already exists / Registrazione gia esistente");
            return false;
        }
        juce::WavAudioFormat format;
        auto make = [&](const juce::File &f, int channels) {
            auto stream = f.createOutputStream();
            if (!stream)
                return std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter>{};
            auto writer = std::unique_ptr<juce::AudioFormatWriter>(
                format.createWriterFor(stream.get(), sampleRate, (unsigned)channels, 32, {}, 0));
            if (!writer)
                return std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter>{};
            stream.release();
            return std::make_unique<juce::AudioFormatWriter::ThreadedWriter>(writer.release(), disk, 262144);
        };
        auto d = make(dryFile, 1);
        auto w = make(wetFile, 2);
        if (!d || !w)
        {
            d.reset();
            w.reset();
            dryFile.deleteFile();
            wetFile.deleteFile();
            setMessage("Cannot create WAV files / Impossibile creare WAV");
            return false;
        }
        {
            juce::SpinLock::ScopedLockType guard(audioLock);
            dryWriter = std::move(d);
            wetWriter = std::move(w);
            dropped = 0;
            recordedSamples = 0;
            recording = true;
        }
        setMessage("REC: " + base.getFileNameWithoutExtension());
        return true;
    }
    void stopRecording()
    {
        std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> d, w;
        {
            juce::SpinLock::ScopedLockType guard(audioLock);
            recording = false;
            d = std::move(dryWriter);
            w = std::move(wetWriter);
        }
        // Flush and close files away from the audio callback.
    }
    void captureInput(const juce::AudioBuffer<float> &buffer, int off, int n, int source, int inputs)
    {
        for (int i = 0; i < n; ++i)
        {
            float v = buffer.getSample(source == 1 && inputs > 1 ? 1 : 0, off + i);
            if (source == 2 && inputs > 1)
                v = .5f * (buffer.getSample(0, off + i) + buffer.getSample(1, off + i));
            raw.setSample(0, i, std::isfinite(v) ? v : 0);
        }
    }
    // 32-bit float WAVs capture bass before the final monitor limiter and tuner mute.
    void record(const juce::AudioBuffer<float> &buffer, int off, int n, int outputs)
    {
        if (!recording)
            return;
        juce::SpinLock::ScopedTryLockType guard(audioLock);
        if (!guard.isLocked())
        {
            ++dropped;
            return;
        }
        if (!dryWriter || !wetWriter)
            return;
        for (int c = 0; c < 2; ++c)
            wet.copyFrom(c, 0, buffer, std::min(c, outputs - 1), off, n);
        const bool d = dryWriter->write(raw.getArrayOfReadPointers(), n);
        const bool w = wetWriter->write(wet.getArrayOfReadPointers(), n);
        if (!d || !w)
            ++dropped;
        recordedSamples += n;
    }
    void mix(juce::AudioBuffer<float> &buffer, int off, int n, int outputs)
    {
        juce::SpinLock::ScopedTryLockType guard(audioLock);
        // A failed publication try-lock skips backing only, never the bass.
        const bool hasTrack = guard.isLocked() && track.getNumSamples() > 1;
        // Consume a seek only when it can be applied: a busy lock no longer loses it.
        const auto seekTo = hasTrack ? requestedPosition.exchange(-1) : -1.0;
        if (hasTrack && seekTo >= 0)
            cursor = juce::jlimit(0.0, double(track.getNumSamples() - 1), seekTo / actualSpeed * fileRate);
        const double renderRate = hasTrack ? fileRate : sampleRate.load();
        const double renderSpeed = hasTrack ? actualSpeed : 1;
        const double step = renderRate / sampleRate.load();
        double a = loopStart / renderSpeed * renderRate;
        double b = loopEnd / renderSpeed * renderRate;
        if (hasTrack)
        {
            a = juce::jlimit(0.0, double(track.getNumSamples() - 1), a);
            b = juce::jlimit(a + 1, double(track.getNumSamples()), b);
        }
        for (int i = 0; i < n; ++i)
        {
            float backing[2]{};
            if (hasTrack && playing)
            {
                if (looping && b > a + renderRate * .05 && (cursor < a || cursor >= b))
                    cursor = a;
                if (cursor >= track.getNumSamples() - 1)
                {
                    playing = false;
                    cursor = 0;
                }
                if (playing)
                {
                    const int index = int(cursor);
                    const float frac = float(cursor - index);
                    // Short boundary ramp avoids clicks at loop seams; no claim of beat detection.
                    float envelope = 1;
                    if (looping && b > a + renderRate * .05)
                        envelope = float(std::min({1.0, std::max(0.0, (cursor - a) / (renderRate * .005)),
                                                   std::max(0.0, (b - cursor) / (renderRate * .005))}));
                    for (int c = 0; c < 2; ++c)
                    {
                        const int ch = std::min(c, track.getNumChannels() - 1); // mono tracks keep one channel
                        backing[c] =
                            (track.getSample(ch, index) * (1 - frac) + track.getSample(ch, index + 1) * frac) *
                            backingGain.load() * envelope;
                    }
                    cursor += step;
                }
            }
            float metronome = 0;
            const double anchor = beatOne.load();
            const bool following = click && hasTrack && playing && anchor >= 0;
            if (following)
            {
                // Beat number from the backing position (original-file seconds).
                const double seconds = cursor / fileRate * actualSpeed;
                const double beatPosition = (seconds - anchor) * juce::jlimit(30.f, 300.f, bpm.load()) / 60.0;
                const auto whole = int64_t(std::floor(beatPosition));
                if (lastBeat == INT64_MIN || whole != lastBeat)
                {
                    if (lastBeat != INT64_MIN || beatPosition - double(whole) < .02)
                    {
                        const int n = std::max(1, beats.load());
                        beat = int(((whole % n) + n) % n);
                        beatDisplay = beat + 1;
                        const bool strong = (accent == 1 && beat == 0) || (accent == 2 && (beat == 1 || beat == 3));
                        clickHz = strong ? 1600.f : 900.f;
                        clickRemaining = int(sampleRate * .025);
                    }
                    lastBeat = whole;
                }
                beatPhase = 0;
            }
            else
                lastBeat = INT64_MIN;
            if (click && !following)
            {
                if (beatPhase <= 0)
                {
                    beat = (beat + 1) % std::max(1, beats.load());
                    beatDisplay = beat + 1;
                    const bool strong =
                        (accent == 1 && beat == 0) || (accent == 2 && (beat == 1 || beat == 3));
                    clickHz = strong ? 1600.f : 900.f;
                    clickRemaining = int(sampleRate * .025);
                    beatPhase += sampleRate * 60.0 / juce::jlimit(30.f, 300.f, bpm.load());
                }
                --beatPhase;
            }
            if (click && clickRemaining > 0)
            {
                metronome = float(std::sin(clickRemaining * 2 * juce::MathConstants<double>::pi * clickHz / sampleRate)) *
                            float(clickRemaining / (sampleRate * .025)) * clickGain;
                --clickRemaining;
            }
            if (!click)
            {
                beatPhase = 0;
                beat = -1;
                clickRemaining = 0;
            }
            for (int c = 0; c < outputs; ++c)
            {
                const float add = outputs == 1 ? (backing[0] + backing[1]) * .5f : backing[c];
                buffer.addSample(c, off + i, add + metronome);
            }
        }
        if (hasTrack)
            position = cursor / fileRate * actualSpeed;
    }
    // Offline WSOLA: fixed output hops, correlation-aligned input windows. Pitch is
    // retained; transient smearing is possible. Stereo channels share offsets.
    static float hann(int j, int window)
    {
        return .5f - .5f * std::cos(float(2 * juce::MathConstants<double>::pi * (j + .5) / window));
    }
    static juce::AudioBuffer<float> stretch(const juce::AudioBuffer<float> &input, double ratio, double sr)
    {
        if (std::abs(ratio - 1) < .001)
            return input;
        const int window = juce::jlimit(256, 4096, int(sr * .04)) & ~1; // even: exact Hann overlap
        const int hop = window / 2, search = int(sr * .008);
        const int length = int(input.getNumSamples() / ratio);
        const int channels = input.getNumChannels();
        juce::AudioBuffer<float> out(channels, length + window);
        out.clear();
        // Hann windows at 50 % overlap sum to exactly 1, so only the first half window
        // (where a single window contributes) needs normalising: no full-length weight buffer.
        int previous = 0;
        for (int dst = 0; dst < length; dst += hop)
        {
            int source = juce::jlimit(0, std::max(0, input.getNumSamples() - window), int(dst * ratio));
            if (dst > 0 && input.getNumSamples() > window * 2)
            {
                double best = -2;
                const int reference = previous + hop;
                const int lo = std::max(0, source - search),
                          hi = std::min(input.getNumSamples() - window, source + search);
                for (int candidate = lo; candidate <= hi; candidate += 4)
                {
                    double dot = 0, e1 = 1e-12, e2 = 1e-12;
                    for (int j = 0; j < hop && reference + j < input.getNumSamples(); j += 8)
                    {
                        const float x = input.getSample(0, reference + j),
                                    y = input.getSample(0, candidate + j);
                        dot += x * y;
                        e1 += x * x;
                        e2 += y * y;
                    }
                    const double score = dot / std::sqrt(e1 * e2);
                    if (score > best)
                    {
                        best = score;
                        source = candidate;
                    }
                }
            }
            previous = source;
            for (int j = 0; j < window && dst + j < length && source + j < input.getNumSamples(); ++j)
            {
                const float w = hann(j, window);
                for (int c = 0; c < channels; ++c)
                    out.addSample(c, dst + j, input.getSample(c, source + j) * w);
            }
        }
        for (int i = 0; i < std::min(hop, length); ++i)
        {
            const float w = hann(i, window);
            if (w > 1e-3f)
                for (int c = 0; c < channels; ++c) out.setSample(c, i, out.getSample(c, i) / w);
        }
        out.setSize(channels, length, true);
        return out;
    }

  private:
    void setMessage(const juce::String &s)
    {
        std::lock_guard lock(requestLock);
        message = s;
    }
    void run() override
    {
        int done = 0;
        while (!threadShouldExit())
        {
            juce::File f;
            float ratio;
            int version;
            {
                std::lock_guard lock(requestLock);
                f = request;
                ratio = requestSpeed;
                version = generation;
            }
            if (version == done)
            {
                wait(100);
                continue;
            }
            done = version;
            std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(f));
            if (!reader || reader->lengthInSamples < 2 || reader->numChannels > 2 ||
                reader->lengthInSamples / reader->sampleRate > maxSeconds)
            {
                setMessage("Use mono/stereo WAV/AIFF, max 10 min / Usa WAV/AIFF mono o stereo, max 10 min");
                loading = false;
                continue;
            }
            // Peak memory: the decoded file plus the stretched copy (longer when slowed down).
            const int channels = int(reader->numChannels);
            const double peak = double(reader->lengthInSamples) * channels * sizeof(float) * (1 + 1 / ratio);
            if (peak > maxMemoryBytes)
            {
                setMessage("Too long for this speed (memory) / Troppo lungo per questa velocita");
                loading = false;
                continue;
            }
            juce::AudioBuffer<float> original(channels, int(reader->lengthInSamples));
            if (!reader->read(&original, 0, original.getNumSamples(), 0, true, channels > 1))
            {
                setMessage("Audio read failed");
                loading = false;
                continue;
            }
            auto rendered = stretch(original, ratio, reader->sampleRate);
            if (threadShouldExit())
                return;
            {
                std::lock_guard lock(requestLock);
                if (version != generation)
                    continue;
                currentFile = f;
                message = f.getFileName();
            }
            {
                juce::SpinLock::ScopedLockType guard(audioLock);
                std::swap(track, rendered);
                fileRate = reader->sampleRate;
                actualSpeed = ratio;
                cursor = 0;
                playing = false;
            }
            duration = reader->lengthInSamples / reader->sampleRate;
            loopStart = 0;
            loopEnd = duration.load();
            position = 0;
            speed = ratio;
            loading = false;
        }
    }
    juce::AudioFormatManager formats;
    juce::TimeSliceThread disk;
    mutable std::mutex requestLock;
    juce::File request, currentFile;
    juce::String message = "WAV / AIFF";
    float requestSpeed = 1;
    int generation = 0;
    juce::SpinLock audioLock;
    juce::AudioBuffer<float> track, raw, wet;
    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> dryWriter, wetWriter;
    std::atomic<double> requestedPosition{-1};
    double cursor = 0, fileRate = 48000, actualSpeed = 1, beatPhase = 0;
    int beat = -1, clickRemaining = 0;
    int64_t lastBeat = INT64_MIN;
    float clickHz = 900;
};
