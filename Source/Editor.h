#pragma once
#include "Processor.h"
#include "Help.h"
#include "Workbench.h"
class LowlineLook : public juce::LookAndFeel_V4
{
  public:
    LowlineLook();
    void drawRotarySlider(juce::Graphics &, int, int, int, int, float, float, float, juce::Slider &) override;
    void drawButtonBackground(juce::Graphics &, juce::Button &, const juce::Colour &, bool, bool) override;
};
// The whole interface, laid out at its design size (1180 x 830).
class LowlineView : public juce::Component,
                    private juce::Timer,
                    private juce::Value::Listener,
                    public juce::FileDragAndDropTarget
{
  public:
    static constexpr int designWidth = 1180, designHeight = 830;
    explicit LowlineView(LowlineProcessor &);
    ~LowlineView() override;
    void paint(juce::Graphics &) override;
    void resized() override;
    bool isInterestedInFileDrag(const juce::StringArray &) override;
    void filesDropped(const juce::StringArray &, int, int) override;

  private:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<SA> link;
    };
    struct Toggle
    {
        juce::TextButton button;
        std::unique_ptr<BA> link;
    };
    void timerCallback() override;
    void choose(LowlineProcessor::Slot);
    void preset(bool save, bool portable = false);
    void assignMidiPreset();
    void translate();
    void panel(juce::Graphics &, juce::Rectangle<int>, const juce::String &, juce::Colour);
    void placeKnob(const char *, juce::Rectangle<int>);
    void placeToggle(const char *, juce::Rectangle<int>);
    juce::String displayedLanguage;
    LowlineProcessor &proc;
    LowlineLook look;
    std::map<juce::String, std::unique_ptr<Knob>> knobs;
    std::map<juce::String, std::unique_ptr<Toggle>> toggles;
    std::array<juce::TextButton, 4> loadButtons, clearButtons;
    juce::TextButton saveButton{"SAVE"}, openButton{"OPEN"}, toneButton{"TONE3000 "},
        languageButton{"IT / EN"}, helpButton{"GUIDA"}, packButton{"PACK"},
        rigTab{"RIG"}, toolsTab{"TOOLS"}, midiTab{"MIDI"}, tapButton{"TAP"},
        learnButton{"LEARN"}, cancelLearn{"CANCEL"}, clearMapping{"CLEAR"}, resetMapping{"RESET"},
        assignButton{"ASSIGN PRESET"}, unassignButton{"CLEAR PC"};
    juce::TextButton workbenchTab{"WORKSPACE"};
    std::unique_ptr<LowlineWorkbench> workbench;
    int page = 0;
    int statusTicks = 0;
    juce::ComboBox division, midiTarget, midiProgram, switchMode;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionLink;
    juce::Label tunerLabel, syncLabel, normLabel, midiHint, assignmentLabel;
    juce::TextEditor mappingText;
    juce::ComboBox source, programs;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> sourceLink;
    juce::Label statusLabel, subtitle, meterLabel;
    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<LowlineHelpWindow> helpWindow;
    std::unique_ptr<LowlineHelpOverlay> helpOverlay;
    // iOS standalone: audio settings, Bluetooth MIDI pairing, remembered input un-mute.
    juce::TextButton audioButton{"AUDIO"}, bluetoothButton{"BLUETOOTH MIDI"};
    juce::Value inputMuted;
    void valueChanged(juce::Value &) override;
    bool inlineHelp() const;
    bool mobileStandalone() const;
};
// Resizable window: the view is scaled to fit (fixed aspect ratio); the size is remembered.
class LowlineEditor : public juce::AudioProcessorEditor
{
  public:
    explicit LowlineEditor(LowlineProcessor &);
    ~LowlineEditor() override;
    void resized() override;
    void paint(juce::Graphics &g) override { g.fillAll(juce::Colour(0xff11151b)); }
    LowlineView &getView() { return view; }
    static juce::File sizeFile();

  private:
    LowlineView view;
};
