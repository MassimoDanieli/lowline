#pragma once
#include "Dsp.h"
#include "Capture.h"
#include "Tuner.h"
#include <juce_cryptography/juce_cryptography.h>
#include <NAM/get_dsp.h>
#include <condition_variable>
#include <deque>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <thread>
class LowlineProcessor : public juce::AudioProcessor
{
  public:
    LowlineProcessor();
    ~LowlineProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void processBlockBypassed(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
    bool isBusesLayoutSupported(const BusesLayout &) const override;
    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Lowline"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 90; }
    int getNumPrograms() override { return 4; }
    int getCurrentProgram() override { return program.load(); }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String &) override {}
    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;
    juce::AudioProcessorValueTreeState state;
    std::atomic<float> inputPeak{0}, outputPeak{0}, gainReduction{0};
    std::atomic<bool> clipped{false}, busy{false};
    enum Slot
    {
        Pedal = 0,
        Amp = 1,
        Cab = 2,
        Room = 3
    };
    void loadFile(Slot, const juce::File &);
    void clearSlot(Slot);
    juce::String slotName(Slot) const;
    juce::String status() const;
    void savePreset(const juce::File &, bool portable = false);
    void tapTempo();
    void armMidiLearn(int target) { learnTarget = target; }
    void resetMidiMappings();
    void clearMidiTarget(int target);
    juce::String midiDescription() const;
    juce::StringArray midiTargetNames() const;
    void assignProgram(int pc, const juce::File &file);
    juce::String programAssignment(int pc) const;
    std::atomic<int> learnTarget{-1};
    std::atomic<float> cpuLoad{0}, effectiveDelay{380}, effectiveTempo{120};
    std::atomic<bool> usingHostTempo{false};
    std::array<std::atomic<bool>, 2> normalisationAvailable{};
    std::array<std::atomic<float>, 2> modelLoudness{};
    LowlineTuner tuner;
    void openPreset(const juce::File &, bool preserveMidi = false);
    juce::String getLanguage() const { return italian.load() ? "IT" : "EN"; }
    void setLanguage(const juce::String &value) { italian = value != "EN"; }
    static juce::AudioProcessorValueTreeState::ParameterLayout parameters();

  private:
    float p(const char *id) const { return state.getRawParameterValue(id)->load(); }
    void set(const char *, float);
    void midi(const juce::MidiBuffer &);
    void workerLoop();
    void performLoad(Slot, const juce::File &);
    void processChunk(juce::AudioBuffer<float> &, int, int);
    void limitOutput(juce::AudioBuffer<float> &);
    void setStatus(const juce::String &);
    std::atomic<int> program{0};
    std::atomic<bool> italian{true};
    lowline::PeakLimiter limiter;
    std::atomic<double> rate{48000};
    std::atomic<int> maxBlock{512};
    juce::AudioBuffer<float> work, dry;
    std::array<LowlineCaptureSlot, 4> captures;
    juce::SpinLock modelLock;

    std::array<std::atomic<bool>, 4> loaded{};
    mutable std::mutex metadataMutex;
    std::array<juce::String, 4> paths{};
    juce::String message = "Ready / Pronto";
    std::mutex jobsMutex;
    std::condition_variable jobsReady;
    struct Job
    {
        Slot slot;
        juce::File file;
    };
    std::deque<Job> jobs;
    bool stopping = false;
    std::thread worker;
    lowline::Compressor compressor;
    lowline::Delay delay;
    lowline::DryDelay dryDelay;
    std::array<lowline::Biquad, 5> eq;
    lowline::Gate gate;
    lowline::CleanCrossover crossover;
    lowline::Slew crossMix, muteGain;
    std::vector<juce::RangedAudioParameter *> midiTargets;
    std::array<std::atomic<int>, 128> midiMap{};
    std::array<bool, 128> ccHigh{};
    std::array<juce::String, 128> programFiles{};
    std::array<std::atomic<bool>, 128> programAssigned{};
    std::atomic<int> requestedProgram{-1};
    std::atomic<double> lastTap{0};
    lowline::Slew inputGain, outputGain, blend, drive, delayMix, reverbMix;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowlineProcessor)
};
