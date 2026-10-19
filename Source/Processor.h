#pragma once
#include "Dsp.h"
#include "Capture.h"
#include "Tuner.h"
#include "Studio.h"
#include <juce_cryptography/juce_cryptography.h>
#include <NAM/get_dsp.h>
#include <condition_variable>
#include <deque>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <thread>
class LowlineProcessor : public juce::AudioProcessor,
                          public juce::AsyncUpdater,
                          private juce::AudioProcessorParameter::Listener
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
    juce::UndoManager undo;
    juce::AudioProcessorValueTreeState state;
    LowlineStudio studio;
    std::atomic<float> compareTrim{0}, measuredRms{-100};
    std::atomic<int> measureRemaining{0};
    void measureLevel() { measureRequested = true; }
    double audioRate() const { return rate; }
    int audioBlock() const { return maxBlock; }
    void storeComparison(int index);
    // Undo covers what you do in the interface (knob, button and menu gestures, factory programs,
    // A/B recall) - never MIDI, host automation or tap tempo.
    std::vector<float> parameterSnapshot() const;
    void applySnapshot(const std::vector<float> &);
    void recordUndo(const juce::String &name, std::vector<float> before);
    std::atomic<uint32_t> stateVersion{0}; // bumps on every parameter or capture change
    float comparisonLevel(int i) const { return i >= 0 && i < 2 ? comparisonRms[size_t(i)].load() : -100.f; }
    bool recallComparison(int index);
    void matchComparison(int reference);
    bool hasComparison(int index) const { return index >= 0 && index < 2 && comparison[size_t(index)].getSize() > 0; }
    juce::String presetName() const;
    juce::String presetTags() const;
    juce::String presetNotes() const;
    void setPresetDetails(const juce::String &, const juce::String &, const juce::String &);
    bool isDirty();
    void markClean();
    void recallProgram(int pc) { if (pc >= 0 && pc < 128) requestedProgram = pc; }

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
    void tapTempo() { tapTempoAt(juce::Time::getMillisecondCounterHiRes()); }
    void tapTempoAt(double nowMs); // explicit clock: deterministic tests
    void armMidiLearn(int target) { learnTarget = target; }
    void resetMidiMappings();
    void clearMidiTarget(int target);
    juce::String midiDescription() const;
    juce::StringArray midiTargetNames() const;
    void assignProgram(int pc, const juce::File &file);
    juce::String programAssignment(int pc) const;
    std::atomic<int> learnTarget{-1};
    // false: the CC value sets a switch (latching pedals, 127 = on / 0 = off).
    // true: every press (rising edge >= 64) flips it (momentary footswitches).
    std::atomic<bool> momentarySwitches{false};
    std::atomic<float> cpuLoad{0}, effectiveDelay{380}, effectiveTempo{120};
    std::atomic<bool> usingHostTempo{false};
    std::array<std::atomic<bool>, 2> normalisationAvailable{};
    std::array<std::atomic<float>, 2> modelLoudness{};
    LowlineTuner tuner;
    void openPreset(const juce::File &, bool preserveMidi = false);
    // Program Change presets are read on the worker and applied here, on the message thread
    // (APVTS replaceState also clears the UndoManager, which is not thread-safe).
    void handleAsyncUpdate() override;
    juce::String getLanguage() const { return italian.load() ? "IT" : "EN"; }
    void setLanguage(const juce::String &value) { italian = value != "EN"; }
    static juce::AudioProcessorValueTreeState::ParameterLayout parameters();

  private:
    float p(const char *id) const { return state.getRawParameterValue(id)->load(); }
    void set(const char *, float);
    void midi(const juce::MidiBuffer &);
    void workerLoop();
    void performLoad(Slot, const juce::File &);
    std::unique_ptr<LowlineCapture> prepareCapture(Slot, const juce::File &, double, int);
    void publishCapture(Slot, const juce::File &, std::unique_ptr<LowlineCapture>);

    void processChunk(juce::AudioBuffer<float> &, int, int);
    void limitOutput(juce::AudioBuffer<float> &, bool allowMute = true);
    bool buildPreset(const juce::File &, bool preserveMidi, juce::MemoryBlock &);
    std::mutex presetMutex;
    juce::MemoryBlock pendingPreset;
    void setStatus(const juce::String &);
    std::atomic<int> program{0};
    std::atomic<bool> italian{true};
    lowline::PeakLimiter limiter;
    std::atomic<double> rate{48000};
    std::atomic<int> maxBlock{512};
    juce::AudioBuffer<float> work, dry, raw;
    lowline::Slew bypassMix, driveMix;
    lowline::AdaaTanh saturator;
    std::array<float, 5> eqGainNow{}, eqFreqNow{};
    void updateEq(bool snap);
    std::array<LowlineCaptureSlot, 4> captures;
    juce::SpinLock modelLock;
    juce::SpinLock publicationLock;

    std::array<std::atomic<bool>, 4> loaded{};
    mutable std::mutex metadataMutex;
    std::array<juce::String, 4> paths{};
    std::array<juce::Time, 4> modified{};
    juce::String message = "Ready / Pronto";
    std::mutex jobsMutex;
    std::condition_variable jobsReady;
    struct Job
    {
        Slot slot;
        juce::File file;
        bool scene = false;
        juce::String warning;
        std::array<juce::File, 4> files;
        std::array<bool, 4> keep{}; // scene: slot already holds this exact file
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
    juce::SpinLock tapLock;
    std::array<double, 3> tapIntervals{};
    int tapCount = 0;
    std::array<juce::MemoryBlock, 2> comparison;
    std::array<std::atomic<float>, 2> comparisonRms{-100.f, -100.f};
    std::atomic<int> measureFor{-1};
    std::atomic<uint32_t> measureStartVersion{0}, measuredVersion{0xffffffffu};
    std::atomic<bool> measureAfterLoad{false};
    bool measurementFresh() const { return measureRemaining == 0 && measuredVersion.load() == stateVersion.load(); }
    void parameterValueChanged(int, float) override { ++stateVersion; }
    void parameterGestureChanged(int, bool starting) override;
    int gestureDepth = 0;
    bool applyingUndo = false;
    std::vector<float> gestureBefore;
    std::atomic<bool> clearUndoPending{false};
    void restoreState(const void *, int, bool keepUndo);
    void applyProgram(int);
    std::atomic<int> programNamePending{-1};
    juce::String cleanSignature;
    std::atomic<bool> cleanWhenReady{false};
    std::atomic<bool> measureRequested{false};
    double measureEnergy = 0; int measureCount = 0;
    lowline::Slew compareGain;
    lowline::Slew inputGain, outputGain, blend, drive, delayMix, reverbMix;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowlineProcessor)
};
