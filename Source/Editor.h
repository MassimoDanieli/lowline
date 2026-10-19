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
class LowlineEditor : public juce::AudioProcessorEditor,
                      private juce::Timer,
                      public juce::FileDragAndDropTarget
{
  public:
    explicit LowlineEditor(LowlineProcessor &);
    ~LowlineEditor() override;
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
};
