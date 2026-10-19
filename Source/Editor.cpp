#include "Editor.h"
using namespace juce;
namespace
{
Colour bg(0xff11151b), card(0xff1c232d), mint(0xff2ef2a0), gold(0xffffbf47), text(0xfff3f6fa),
    muted(0xffacbacb);
}
LowlineLook::LowlineLook()
{
    setColour(Slider::textBoxTextColourId, text);
    setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour(Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour(Label::textColourId, text);
    setColour(TextButton::textColourOffId, text);
    setColour(TextButton::textColourOnId, bg);
    setColour(ComboBox::backgroundColourId, card);
    setColour(ComboBox::textColourId, text);
    setColour(ComboBox::outlineColourId, Colour(0xff46566b));
    setColour(PopupMenu::backgroundColourId, card);
    setColour(PopupMenu::textColourId, text);
}
void LowlineLook::drawRotarySlider(Graphics &g, int x, int y, int w, int h, float pos, float start, float end,
                                   Slider &s)
{
    auto b = Rectangle<float>((float)x, (float)y, (float)w, (float)h).reduced(9);
    auto d = jmin(b.getWidth(), b.getHeight());
    b = b.withSizeKeepingCentre(d, d);
    auto c = b.getCentre();
    float r = d / 2;
    Path track;
    track.addCentredArc(c.x, c.y, r, r, 0, start, end, true);
    g.setColour(Colour(0xff39485b));
    g.strokePath(track, PathStrokeType(3.5f));
    Path fill;
    fill.addCentredArc(c.x, c.y, r, r, 0, start, start + pos * (end - start), true);
    g.setColour(s.isMouseOverOrDragging() ? gold : mint);
    g.strokePath(fill, PathStrokeType(3.5f));
    auto inner = b.reduced(6);
    g.setGradientFill(ColourGradient(Colour(0xff536276), inner.getTopLeft(), Colour(0xff222c3a),
                                     inner.getBottomRight(), false));
    g.fillEllipse(inner);
    g.setColour(Colour(0xff687b92));
    g.drawEllipse(inner, 1);
    float angle = start + pos * (end - start);
    g.setColour(text);
    g.drawLine(c.x + std::sin(angle) * r * .30f, c.y - std::cos(angle) * r * .30f,
               c.x + std::sin(angle) * r * .64f, c.y - std::cos(angle) * r * .64f, 2.5f);
}
void LowlineLook::drawButtonBackground(Graphics &g, Button &b, const Colour &, bool hover, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced(.5f);
    g.setColour(b.getToggleState() ? mint
                                   : (down    ? Colour(0xff43566e)
                                      : hover ? Colour(0xff34475e)
                                              : Colour(0xff293647)));
    g.fillRoundedRectangle(r, 7);
    g.setColour(b.getToggleState() ? mint : Colour(0xff526780));
    g.drawRoundedRectangle(r, 7, 1);
}
LowlineEditor::LowlineEditor(LowlineProcessor &p) : AudioProcessorEditor(p), proc(p)
{
    setLookAndFeel(&look);
    auto knob = [this](const char *id, const char *label, const char *suffix)
    {
        auto k = std::make_unique<Knob>();
        k->slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
        k->slider.onDragStart = [this,id] { proc.undo.beginNewTransaction(id); };
        k->slider.setTextBoxStyle(Slider::TextBoxBelow, false, 88, 20);
        k->slider.setTextValueSuffix(suffix);
        k->slider.setDoubleClickReturnValue(true, proc.state.getParameter(id)->convertFrom0to1(
                                                      proc.state.getParameter(id)->getDefaultValue()));
        k->label.setText(label, dontSendNotification);
        k->label.setJustificationType(Justification::centred);
        k->label.setFont(Font(12.f));
        addAndMakeVisible(k->slider);
        addAndMakeVisible(k->label);
        k->link = std::make_unique<SA>(proc.state, id, k->slider);
        knobs[id] = std::move(k);
    };
    knob("input", "INPUT", " dB");
    knob("output", "OUTPUT", " dB");
    knob("threshold", "THRESHOLD", " dB");
    knob("ratio", "RATIO", ":1");
    knob("attack", "ATTACK", " ms");
    knob("release", "RELEASE", " ms");
    knob("makeup", "MAKEUP", " dB");
    knob("drive", "BOOST", " dB");
    knob("bass", "BASS / 100 Hz", " dB");
    knob("mid", "MID", " dB");
    knob("treble", "TREBLE / 3.5 kHz", " dB");
    knob("blend", "AMP / CLEAN", " %");
    knob("align", "CLEAN ALIGN", " ms");
    knob("time", "TIME", " ms");
    knob("feedback", "FEEDBACK", " %");
    knob("delayMix", "DELAY LEVEL", " %");
    knob("highcut", "HIGH CUT", " Hz");
    knob("reverbMix", "REVERB MIX", " %");
    knob("gateThreshold", "GATE THRESHOLD", " dB");
    knob("gateRelease", "GATE RELEASE", " ms");
    knob("lowMid", "LOW MID", " dB");
    knob("highMid", "HIGH MID", " dB");
    knob("lowMidFreq", "LOW MID FREQ", " Hz");
    knob("midFreq", "MID FREQ", " Hz");
    knob("highMidFreq", "HIGH MID FREQ", " Hz");
    knob("crossFreq", "CLEAN LOW PASS", " Hz");
    knob("normTarget", "NAM TARGET", " dB");
    knob("tunerRef", "A4 REFERENCE", " Hz");
    knob("tempo", "TEMPO", " BPM");
    knob("lowcut", "DELAY LOW CUT", " Hz");
    const std::pair<const char *, const char *> ts[] = {
        {"compOn", "COMP ON"},     {"driveOn", "BOOST ON"},   {"pedalOn", "PEDAL ON"}, {"ampOn", "AMP ON"},
        {"cabOn", "CAB ON"},       {"invert", "CLEAN PHASE"}, {"delayOn", "DELAY ON"}, {"ping", "PING PONG"},
        {"reverbOn", "REVERB ON"}, {"bypass", "BYPASS"},
        {"gateOn", "GATE ON"}, {"crossOn", "CLEAN LP"}, {"normOn", "NORMALISE"},
        {"tunerOn", "TUNER"}, {"tunerMute", "MUTE"}, {"sync", "SYNC"}, {"hostSync", "HOST BPM"}};
    for (auto [id, label] : ts)
    {
        auto t = std::make_unique<Toggle>();
        t->button.setButtonText(label);
        t->button.setClickingTogglesState(true);
        addAndMakeVisible(t->button);
        t->link = std::make_unique<BA>(proc.state, id, t->button);
        toggles[id] = std::move(t);
    }
    for (int i = 0; i < 4; ++i)
    {
        loadButtons[i].onClick = [this, i] { choose((LowlineProcessor::Slot)i); };
        clearButtons[i].setButtonText("x");
        clearButtons[i].onClick = [this, i] { proc.clearSlot((LowlineProcessor::Slot)i); };
        addAndMakeVisible(loadButtons[i]);
        addAndMakeVisible(clearButtons[i]);
    }
    for (auto *b : {&saveButton, &openButton, &toneButton, &languageButton, &helpButton, &packButton,
                    &rigTab, &toolsTab, &midiTab, &tapButton, &learnButton, &cancelLearn,
                    &clearMapping, &resetMapping, &assignButton, &unassignButton})
        addAndMakeVisible(*b);
    helpButton.setComponentID("lowline-help");
    helpButton.onClick = [this]
    {
        if (!helpWindow)
            helpWindow = std::make_unique<LowlineHelpWindow>(proc.getLanguage() == "IT");
        helpWindow->setVisible(true);
        helpWindow->toFront(true);
    };
    packButton.onClick = [this] { preset(true, true); };
    rigTab.onClick = [this] { page = 0; resized(); repaint(); };
    toolsTab.onClick = [this] { page = 1; resized(); repaint(); };
    midiTab.onClick = [this] { page = 2; resized(); repaint(); };
    rigTab.setComponentID("tab-rig"); toolsTab.setComponentID("tab-tools"); midiTab.setComponentID("tab-midi");
    tapButton.onClick = [this] { proc.tapTempo(); };
    midiTarget.addItemList(proc.midiTargetNames(), 1);
    midiTarget.setSelectedId(1);
    for (int pc = 0; pc < 128; ++pc) midiProgram.addItem("PC " + String(pc), pc + 1);
    midiProgram.setSelectedId(5);
    switchMode.addItem("Latching: 127 = on, 0 = off", 1);
    switchMode.addItem("Momentary: each press toggles", 2);
    switchMode.setSelectedId(proc.momentarySwitches ? 2 : 1, dontSendNotification);
    switchMode.onChange = [this] { proc.momentarySwitches = switchMode.getSelectedId() == 2; };
    for (auto *box : {&division, &midiTarget, &midiProgram, &switchMode}) addAndMakeVisible(*box);
    division.addItemList({"1/16", "1/8", "1/8 dotted", "1/4", "1/4 dotted", "1/2"}, 1);
    divisionLink = std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.state, "division", division);
    learnButton.onClick = [this] { proc.armMidiLearn(midiTarget.getSelectedId() - 1); };
    cancelLearn.onClick = [this] { proc.armMidiLearn(-1); };
    clearMapping.onClick = [this] { proc.clearMidiTarget(midiTarget.getSelectedId() - 1); };
    resetMapping.onClick = [this] { proc.resetMidiMappings(); };
    assignButton.onClick = [this] { assignMidiPreset(); };
    unassignButton.onClick = [this] { proc.assignProgram(midiProgram.getSelectedId() - 1, File{}); };
    for (auto *label : {&tunerLabel, &syncLabel, &normLabel, &midiHint, &assignmentLabel}) addAndMakeVisible(*label);
    tunerLabel.setFont(Font(22.f));
    tunerLabel.setColour(Label::textColourId, mint);
    mappingText.setMultiLine(true); mappingText.setReadOnly(true);
    mappingText.setColour(TextEditor::backgroundColourId, card);
    mappingText.setColour(TextEditor::textColourId, text);
    mappingText.setFont(Font(16.f)); addAndMakeVisible(mappingText);
    saveButton.onClick = [this] { preset(true); };
    openButton.onClick = [this] { preset(false); };
    toneButton.onClick = [] { URL("https://www.tone3000.com/").launchInDefaultBrowser(); };
    languageButton.onClick = [this]
    {
        proc.setLanguage(proc.getLanguage() == "IT" ? "EN" : "IT");
        translate();
    };
    source.addItemList({"Input 1 / Left", "Input 2 / Right", "Stereo > Mono"}, 1);
    addAndMakeVisible(source);
    sourceLink =
        std::make_unique<AudioProcessorValueTreeState::ComboBoxAttachment>(proc.state, "source", source);
    for (int i = 0; i < 4; ++i)
        programs.addItem(proc.getProgramName(i), i + 1);
    programs.setSelectedId(proc.getCurrentProgram() + 1, dontSendNotification);
    programs.onChange = [this] { proc.setCurrentProgram(programs.getSelectedId() - 1); };
    addAndMakeVisible(programs);
    for (auto *l : {&statusLabel, &subtitle, &meterLabel})
        addAndMakeVisible(*l);
    subtitle.setColour(Label::textColourId, muted);
    statusLabel.setColour(Label::textColourId, muted);
    meterLabel.setFont(Font(13.f));
    workbench = std::make_unique<LowlineWorkbench>(proc);
    addAndMakeVisible(*workbench);
    addAndMakeVisible(workbenchTab);
    workbenchTab.setComponentID("tab-workspace");
    workbenchTab.onClick = [this] { page = 3; resized(); repaint(); workbench->grabKeyboardFocus(); };
    setSize(1180, 830);
    resized();
    translate();
    timerCallback();
    startTimerHz(20);
}
LowlineEditor::~LowlineEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}
void LowlineEditor::translate()
{
    displayedLanguage = proc.getLanguage();
    bool it = displayedLanguage == "IT";
    subtitle.setText(it ? "IL TUO BASSO. IL TUO SUONO.   /   NAM + IR PERFORMANCE RIG"
                        : "YOUR BASS. YOUR SOUND.   /   NAM + IR PERFORMANCE RIG",
                     dontSendNotification);
    saveButton.setButtonText(it ? "SALVA" : "SAVE");
    openButton.setButtonText(it ? "APRI" : "OPEN");
    languageButton.setButtonText(proc.getLanguage() + " / " + (it ? "EN" : "IT"));
    packButton.setButtonText(it ? "ESPORTA" : "EXPORT");
    toolsTab.setButtonText(it ? "STRUMENTI" : "TOOLS");
    workbenchTab.setButtonText(it ? "AREA STUDIO" : "WORKSPACE");
    midiHint.setText(it ? "Scegli un controllo, premi LEARN e muovi il pedale. TAP usa il fronte di salita del CC."
                        : "Choose a control, press LEARN and move the pedal. TAP uses the rising edge of the CC.", dontSendNotification);
    assignButton.setButtonText(it ? "ASSEGNA PRESET" : "ASSIGN PRESET");
    helpButton.setButtonText(it ? "GUIDA" : "HELP");
    if (helpWindow)
        helpWindow->setLanguage(it);
    repaint();
}
void LowlineEditor::panel(Graphics &g, Rectangle<int> r, const String &title, Colour accent)
{
    g.setColour(card);
    g.fillRoundedRectangle(r.toFloat(), 13);
    g.setColour(Colour(0xff36465b));
    g.drawRoundedRectangle(r.toFloat().reduced(.5f), 13, 1);
    g.setColour(accent);
    g.setFont(Font(13.f, Font::bold));
    g.drawText(title, r.reduced(18).removeFromTop(22), Justification::centredLeft);
}
void LowlineEditor::paint(Graphics &g)
{
    g.fillAll(bg);
    g.setColour(mint);
    g.setFont(Font(42.f, Font::bold));
    g.drawText("lowline", 24, 10, 220, 54, Justification::centredLeft);
    g.setColour(gold);
    g.setFont(Font(11.f));
    g.drawText(String(JucePlugin_VersionString) + "  /  BASS WORKSPACE", 250, 31, 240, 20, Justification::centredLeft);
    if (page == 0)
    {
    panel(g, {24, 122, 550, 210}, "01  /  DYNAMICS", mint);
    panel(g, {590, 122, 566, 210}, "02  /  CAPTURES + CABINET", gold);
    panel(g, {24, 348, 1132, 215}, "03  /  INPUT + PARALLEL CLEAN", mint);
    panel(g, {24, 579, 684, 180}, "04  /  STEREO TAPE-STYLE DELAY", gold);
    panel(g, {724, 579, 432, 180}, "05  /  CONVOLUTION ROOM", mint);
    }
    else if (page == 1)
    {
        panel(g, {24, 122, 1132, 236}, "01  /  FIVE-BAND EQ", mint);
        panel(g, {24, 374, 350, 180}, "02  /  NOISE GATE", gold);
        panel(g, {390, 374, 366, 180}, "03  /  CLEAN CROSSOVER", mint);
        panel(g, {772, 374, 384, 180}, "04  /  TUNER", gold);
        panel(g, {24, 570, 732, 189}, "05  /  DELAY TEMPO + FILTER", gold);
        panel(g, {772, 570, 384, 189}, "06  /  NAM LEVEL", mint);
    }
    else if (page == 2)
    {
        panel(g, {24, 122, 1132, 390}, "01  /  MIDI LEARN", mint);
        panel(g, {24, 528, 1132, 231}, "02  /  PROGRAM CHANGE PRESETS", gold);
    }
    g.setColour(muted);
    g.setFont(Font(12.f));
    g.drawText(proc.getLanguage() == "IT"
                   ? "GATE > COMP > BOOST > NAM PEDAL > NAM AMP > EQ > CAB + CLEAN > DELAY > IR REVERB"
                   : "GATE > COMP > BOOST > NAM PEDAL > NAM AMP > EQ > CAB + CLEAN > DELAY > IR REVERB",
               24, 765, 1132, 20, Justification::centred);
}
void LowlineEditor::placeKnob(const char *id, Rectangle<int> r)
{
    auto &k = *knobs.at(id);
    k.slider.setVisible(true); k.label.setVisible(true);
    k.label.setBounds(r.removeFromTop(21));
    k.slider.setBounds(r);
}
void LowlineEditor::placeToggle(const char *id, Rectangle<int> r)
{
    toggles.at(id)->button.setVisible(true);
    toggles.at(id)->button.setBounds(r);
}
void LowlineEditor::resized()
{
    if (workbench) { workbench->setVisible(page == 3); workbench->setBounds(24,122,1132,637); }
    workbenchTab.setBounds(346,92,148,24);
    workbenchTab.setToggleState(page == 3, dontSendNotification);
    for (auto &[id, k] : knobs) { k->slider.setVisible(false); k->label.setVisible(false); }
    for (auto &[id, t] : toggles) t->button.setVisible(false);
    for (auto &button : loadButtons) button.setVisible(page == 0);
    for (auto &button : clearButtons) button.setVisible(page == 0);
    for (auto *component : std::initializer_list<Component *>{&division, &tapButton, &tunerLabel, &syncLabel, &normLabel})
        component->setVisible(page == 1);
    for (auto *component : std::initializer_list<Component *>{&midiTarget, &midiProgram, &learnButton, &cancelLearn,
            &clearMapping, &resetMapping, &assignButton, &unassignButton, &midiHint, &assignmentLabel, &mappingText, &switchMode})
        component->setVisible(page == 2);
    rigTab.setToggleState(page == 0, dontSendNotification);
    toolsTab.setToggleState(page == 1, dontSendNotification);
    midiTab.setToggleState(page == 2, dontSendNotification);
    subtitle.setBounds(26, 67, 600, 20);
    rigTab.setBounds(24, 92, 90, 24); toolsTab.setBounds(122, 92, 118, 24); midiTab.setBounds(248, 92, 90, 24);
    source.setBounds(740, 25, 180, 30);
    helpButton.setBounds(628, 25, 96, 30);
    programs.setBounds(936, 25, 220, 30);
    saveButton.setBounds(652, 70, 80, 28);
    packButton.setBounds(740, 70, 86, 28);
    openButton.setBounds(834, 70, 78, 28);
    toneButton.setBounds(920, 70, 136, 28);
    languageButton.setBounds(1064, 70, 92, 28);
    meterLabel.setBounds(24, 785, 1132, 20);
    statusLabel.setBounds(24, 807, 1132, 20);
    if (page == 3) return;
    if (page == 1)
    {
        const char *eq[] = {"bass", "lowMid", "mid", "highMid", "treble", "lowMidFreq", "midFreq", "highMidFreq"};
        for (int i = 0; i < 8; ++i) placeKnob(eq[i], {36 + i * 139, 175, 130, 156});
        placeToggle("gateOn", {245, 389, 110, 26});
        placeKnob("gateThreshold", {40, 428, 150, 112}); placeKnob("gateRelease", {200, 428, 150, 112});
        placeToggle("crossOn", {630, 389, 110, 26});
        placeKnob("crossFreq", {418, 428, 144, 112}); placeKnob("align", {582, 428, 144, 112});
        placeToggle("tunerOn", {922, 389, 104, 26}); placeToggle("tunerMute", {1034, 389, 104, 26});
        tunerLabel.setBounds(788, 435, 210, 96); placeKnob("tunerRef", {1018, 430, 124, 112});
        placeToggle("sync", {510, 585, 90, 26}); placeToggle("hostSync", {608, 585, 130, 26});
        placeKnob("tempo", {40, 627, 145, 115}); placeKnob("lowcut", {200, 627, 145, 115});
        division.setBounds(380, 632, 168, 30); tapButton.setBounds(568, 632, 168, 30);
        syncLabel.setBounds(380, 677, 356, 58);
        placeToggle("normOn", {1018, 585, 120, 26}); placeKnob("normTarget", {788, 627, 136, 115});
        normLabel.setBounds(936, 632, 206, 106);
        return;
    }
    if (page == 2)
    {
        midiHint.setBounds(42, 166, 740, 30);
        switchMode.setBounds(800, 166, 338, 30);
        midiTarget.setBounds(42, 210, 430, 32);
        learnButton.setBounds(492, 210, 138, 32); cancelLearn.setBounds(646, 210, 138, 32);
        clearMapping.setBounds(800, 210, 154, 32); resetMapping.setBounds(970, 210, 168, 32);
        mappingText.setBounds(42, 260, 1096, 232);
        midiProgram.setBounds(42, 582, 180, 32); assignButton.setBounds(242, 582, 250, 32);
        unassignButton.setBounds(512, 582, 180, 32);
        assignmentLabel.setBounds(42, 635, 1096, 95);
        return;
    }
    placeToggle("compOn", {440, 137, 116, 26});
    const char *c[] = {"threshold", "ratio", "attack", "release", "makeup"};
    for (int i = 0; i < 5; ++i)
        placeKnob(c[i], {36 + i * 106, 179, 100, 136});
    const char *labels[] = {"pedalOn", "ampOn", "cabOn"};
    for (int i = 0; i < 3; ++i)
    {
        placeToggle(labels[i], {606, 174 + i * 47, 106, 30});
        loadButtons[i].setBounds(720, 174 + i * 47, 382, 30);
        clearButtons[i].setBounds(1108, 174 + i * 47, 30, 30);
    }
    placeToggle("driveOn", {790, 364, 110, 26});
    placeToggle("invert", {908, 364, 108, 26});
    placeToggle("bypass", {1024, 364, 114, 26});
    const char *tones[] = {"input", "drive", "blend", "align", "output"};
    for (int i = 0; i < 5; ++i)
        placeKnob(tones[i], {60 + i * 218, 408, 180, 139});
    placeToggle("delayOn", {462, 595, 108, 26});
    placeToggle("ping", {578, 595, 112, 26});
    const char *ds[] = {"time", "feedback", "highcut", "delayMix"};
    for (int i = 0; i < 4; ++i)
        placeKnob(ds[i], {44 + i * 165, 636, 144, 112});
    placeToggle("reverbOn", {1020, 595, 118, 26});
    loadButtons[3].setBounds(742, 641, 244, 32);
    clearButtons[3].setBounds(992, 641, 28, 32);
    placeKnob("reverbMix", {1025, 635, 116, 112});

}
void LowlineEditor::timerCallback()
{
    if (displayedLanguage != proc.getLanguage())
        translate();
    for (int i = 0; i < 4; ++i)
    {
        auto name = proc.slotName((LowlineProcessor::Slot)i);
        auto prefix = i < 2 ? ".nam  /  " : "IR  /  ";
        loadButtons[i].setButtonText(
            prefix + (name == "(empty)" ? (proc.getLanguage() == "IT" ? "Carica..." : "Load...") : name));
    }
    statusLabel.setText((proc.busy ? "Loading...  " : "") + proc.status(), dontSendNotification);
    const bool tuningMute = proc.state.getRawParameterValue("tunerOn")->load() && proc.state.getRawParameterValue("tunerMute")->load();
    const bool synced = proc.state.getRawParameterValue("sync")->load() > .5f;
    knobs.at("time")->slider.setEnabled(!synced);
    knobs.at("time")->label.setText(synced ? "TIME (SYNC)" : "TIME", dontSendNotification);
    if (++statusTicks % 10 == 0)
        saveButton.setButtonText(String(proc.getLanguage() == "IT" ? "SALVA" : "SAVE") + (proc.isDirty() ? " *" : ""));
    bool clip = proc.clipped.exchange(false);
    meterLabel.setColour(Label::textColourId, clip ? Colour(0xffff785d) : mint);
    meterLabel.setText("IN " + String(lowline::toDb(proc.inputPeak), 1) + " dB  /  OUT " +
                           String(lowline::toDb(proc.outputPeak), 1) + " dB  /  GR " +
                           String(proc.gainReduction.load(), 1) + " dB  /  CPU " + String(proc.cpuLoad.load(), 1) + "%" +
                           (tuningMute ? "   TUNER MUTE" : "") + (clip ? "   LIMIT - lower output" : ""),
                       dontSendNotification);
    programs.setSelectedId(proc.getCurrentProgram() + 1, dontSendNotification);
    switchMode.setSelectedId(proc.momentarySwitches ? 2 : 1, dontSendNotification);
    const bool it = proc.getLanguage() == "IT";
    const float frequency = proc.tuner.frequency.load();
    String tuning = it ? "Suona una corda" : "Play one string";
    if (proc.state.getRawParameterValue("tunerOn")->load() && frequency > 0)
    {
        const double midiNote = 69 + 12 * std::log2(frequency / proc.state.getRawParameterValue("tunerRef")->load());
        const int note = int(std::round(midiNote));
        const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        const double cents = (midiNote - note) * 100;
        tuning = String(names[(note % 12 + 12) % 12]) + String(note / 12 - 1) + "  " + String(cents, 1) + " ct\n" + String(frequency, 1) + " Hz";
        tunerLabel.setColour(Label::textColourId, std::abs(cents) <= 3 ? mint : gold);
    }
    else if (!proc.state.getRawParameterValue("tunerOn")->load()) tuning = it ? "Accordatore spento" : "Tuner off";
    tunerLabel.setText(tuning, dontSendNotification);
    syncLabel.setText(String(proc.effectiveTempo.load(), 1) + " BPM / " + String(proc.effectiveDelay.load(), 0) + " ms\n" +
                      (proc.usingHostTempo ? "DAW tempo" : "Manual / TAP tempo"), dontSendNotification);
    String normal;
    for (int slot = 0; slot < 2; ++slot)
        normal += String(slot == 0 ? "PEDAL: " : "AMP: ") +
            (proc.normalisationAvailable[size_t(slot)] ? String(proc.modelLoudness[size_t(slot)].load(), 1) + " dB" : (it ? "nessun dato" : "no metadata")) + "\n";
    normLabel.setText(normal, dontSendNotification);
    const String mapping = proc.midiDescription();
    if (mappingText.getText() != mapping) mappingText.setText(mapping, false);
    learnButton.setButtonText(proc.learnTarget >= 0 ? (it ? "MUOVI PEDALE..." : "MOVE PEDAL...") : "LEARN");
    const String assignment = proc.programAssignment(midiProgram.getSelectedId() - 1);
    assignmentLabel.setText((assignment.isEmpty() ? (it ? "Nessun preset assegnato." : "No preset assigned.") : assignment) +
        String(it ? "\nPC 0-3: preset di fabbrica se non assegnati. SALVA conserva la mappa MIDI e gli abbinamenti."
                  : "\nPC 0-3: factory presets when unassigned. SAVE retains MIDI mappings and assignments."), dontSendNotification);
}
void LowlineEditor::choose(LowlineProcessor::Slot slot)
{
    chooser = std::make_unique<FileChooser>(slot < 2 ? "Load NAM capture" : "Load impulse response", File{},
                                            slot < 2 ? "*.nam" : "*.wav;*.aiff;*.aif");
    Component::SafePointer<LowlineEditor> safe(this);
    chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                         [safe, slot](const FileChooser &c)
                         {
                             if (safe && c.getResult().existsAsFile())
                                 safe->proc.loadFile(slot, c.getResult());
                         });
}
void LowlineEditor::preset(bool save, bool portable)
{
    chooser = std::make_unique<FileChooser>(
        save ? "Save Lowline preset" : "Open Lowline preset",
        File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("My bass.lowline"), "*.lowline");
    Component::SafePointer<LowlineEditor> safe(this);
    chooser->launchAsync((save ? FileBrowserComponent::saveMode | FileBrowserComponent::warnAboutOverwriting
                               : FileBrowserComponent::openMode) |
                             FileBrowserComponent::canSelectFiles,
                         [safe, save, portable](const FileChooser &c)
                         {
                             if (safe && c.getResult() != File{})
                             {
                                 auto f = c.getResult();
                                 if (save)
                                     safe->proc.savePreset(f.withFileExtension("lowline"), portable);
                                 else
                                     safe->proc.openPreset(f);
                             }
                         });
}
bool LowlineEditor::isInterestedInFileDrag(const StringArray &files)
{
    return files.size() == 1 && (File(files[0]).hasFileExtension("nam;wav;aiff;aif;lowline"));
}
void LowlineEditor::filesDropped(const StringArray &files, int x, int y)
{
    if (files.isEmpty())
        return;
    File f(files[0]);
    if (f.hasFileExtension("lowline"))
    {
        proc.openPreset(f);
        return;
    }
    // Dropped on a slot row: load exactly there (a .nam can go to the pedal slot too).
    if (page == 0)
        for (int i = 0; i < 4; ++i)
            if (loadButtons[size_t(i)].getBounds().expanded(0, 8).contains(x, y) &&
                f.hasFileExtension(i < 2 ? "nam" : "wav;aiff;aif"))
            {
                proc.loadFile((LowlineProcessor::Slot)i, f);
                return;
            }
    proc.loadFile(f.hasFileExtension("nam") ? LowlineProcessor::Amp
                  : y >= 579                ? LowlineProcessor::Room
                                            : LowlineProcessor::Cab,
                  f);
}

void LowlineEditor::assignMidiPreset()
{
    const int pc = midiProgram.getSelectedId() - 1;
    chooser = std::make_unique<FileChooser>("Assign .lowline preset to PC " + String(pc), File{}, "*.lowline");
    Component::SafePointer<LowlineEditor> safe(this);
    chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
        [safe, pc](const FileChooser &choice)
        {
            if (safe && choice.getResult().existsAsFile()) safe->proc.assignProgram(pc, choice.getResult());
        });
}
