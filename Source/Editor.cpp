#include "Editor.h"
using namespace juce;
namespace
{
Colour bg(0xff101715), card(0xff19241f), mint(0xffa5e2b0), gold(0xffe7be78), text(0xffe9eee8),
    muted(0xff93a698);
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
    setColour(ComboBox::outlineColourId, Colour(0xff395044));
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
    g.setColour(Colour(0xff35473d));
    g.strokePath(track, PathStrokeType(3.5f));
    Path fill;
    fill.addCentredArc(c.x, c.y, r, r, 0, start, start + pos * (end - start), true);
    g.setColour(s.isMouseOverOrDragging() ? gold : mint);
    g.strokePath(fill, PathStrokeType(3.5f));
    auto inner = b.reduced(6);
    g.setGradientFill(ColourGradient(Colour(0xff44564a), inner.getTopLeft(), Colour(0xff1c2922),
                                     inner.getBottomRight(), false));
    g.fillEllipse(inner);
    g.setColour(Colour(0xff536358));
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
                                   : (down    ? Colour(0xff3a5142)
                                      : hover ? Colour(0xff304638)
                                              : Colour(0xff24362b)));
    g.fillRoundedRectangle(r, 7);
    g.setColour(b.getToggleState() ? mint : Colour(0xff415448));
    g.drawRoundedRectangle(r, 7, 1);
}
LowlineEditor::LowlineEditor(LowlineProcessor &p) : AudioProcessorEditor(p), proc(p)
{
    setLookAndFeel(&look);
    auto knob = [this](const char *id, const char *label, const char *suffix)
    {
        auto k = std::make_unique<Knob>();
        k->slider.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
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
    knob("mid", "MID / 700 Hz", " dB");
    knob("treble", "TREBLE / 3.5 kHz", " dB");
    knob("blend", "AMP / CLEAN", " %");
    knob("align", "CLEAN ALIGN", " ms");
    knob("time", "TIME", " ms");
    knob("feedback", "FEEDBACK", " %");
    knob("delayMix", "DELAY LEVEL", " %");
    knob("highcut", "HIGH CUT", " Hz");
    knob("reverbMix", "REVERB MIX", " %");
    const std::pair<const char *, const char *> ts[] = {
        {"compOn", "COMP ON"},     {"driveOn", "BOOST ON"},   {"pedalOn", "PEDAL ON"}, {"ampOn", "AMP ON"},
        {"cabOn", "CAB ON"},       {"invert", "CLEAN PHASE"}, {"delayOn", "DELAY ON"}, {"ping", "PING PONG"},
        {"reverbOn", "REVERB ON"}, {"bypass", "BYPASS"}};
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
    for (auto *b : {&saveButton, &openButton, &toneButton, &languageButton})
        addAndMakeVisible(*b);
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
    setSize(1180, 830);
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
    repaint();
}
void LowlineEditor::panel(Graphics &g, Rectangle<int> r, const String &title, Colour accent)
{
    g.setColour(card);
    g.fillRoundedRectangle(r.toFloat(), 13);
    g.setColour(Colour(0xff304237));
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
    g.drawText("0.1.1  /  DEVELOPMENT BUILD", 250, 31, 240, 20, Justification::centredLeft);
    panel(g, {24, 122, 550, 210}, "01  /  DYNAMICS", mint);
    panel(g, {590, 122, 566, 210}, "02  /  CAPTURES + CABINET", gold);
    panel(g, {24, 348, 1132, 215}, "03  /  TONE + PARALLEL CLEAN", mint);
    panel(g, {24, 579, 684, 180}, "04  /  STEREO TAPE-STYLE DELAY", gold);
    panel(g, {724, 579, 432, 180}, "05  /  CONVOLUTION ROOM", mint);
    g.setColour(muted);
    g.setFont(Font(12.f));
    g.drawText(proc.getLanguage() == "IT"
                   ? "COMP > BOOST > NAM PEDAL > NAM AMP > EQ > CAB + CLEAN > DELAY > IR REVERB"
                   : "COMP > BOOST > NAM PEDAL > NAM AMP > EQ > CAB + CLEAN > DELAY > IR REVERB",
               24, 773, 1132, 20, Justification::centred);
}
void LowlineEditor::placeKnob(const char *id, Rectangle<int> r)
{
    auto &k = *knobs.at(id);
    k.label.setBounds(r.removeFromTop(21));
    k.slider.setBounds(r);
}
void LowlineEditor::placeToggle(const char *id, Rectangle<int> r)
{
    toggles.at(id)->button.setBounds(r);
}
void LowlineEditor::resized()
{
    subtitle.setBounds(26, 69, 650, 22);
    source.setBounds(740, 25, 180, 30);
    programs.setBounds(936, 25, 220, 30);
    saveButton.setBounds(740, 70, 86, 28);
    openButton.setBounds(834, 70, 78, 28);
    toneButton.setBounds(920, 70, 136, 28);
    languageButton.setBounds(1064, 70, 92, 28);
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
    const char *tones[] = {"input", "drive", "bass", "mid", "treble", "blend", "align", "output"};
    for (int i = 0; i < 8; ++i)
        placeKnob(tones[i], {38 + i * 139, 408, 130, 139});
    placeToggle("delayOn", {462, 595, 108, 26});
    placeToggle("ping", {578, 595, 112, 26});
    const char *ds[] = {"time", "feedback", "highcut", "delayMix"};
    for (int i = 0; i < 4; ++i)
        placeKnob(ds[i], {44 + i * 165, 636, 144, 112});
    placeToggle("reverbOn", {1020, 595, 118, 26});
    loadButtons[3].setBounds(742, 641, 244, 32);
    clearButtons[3].setBounds(992, 641, 28, 32);
    placeKnob("reverbMix", {1025, 635, 116, 112});
    meterLabel.setBounds(742, 688, 278, 48);
    statusLabel.setBounds(24, 799, 1132, 23);
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
    bool clip = proc.clipped.exchange(false);
    meterLabel.setColour(Label::textColourId, clip ? Colour(0xffff8d73) : mint);
    meterLabel.setText("IN " + String(lowline::toDb(proc.inputPeak), 1) + " dB  /  OUT " +
                           String(lowline::toDb(proc.outputPeak), 1) + " dB\nGR " +
                           String(proc.gainReduction.load(), 1) + " dB" +
                           (clip ? "   LIMIT - lower output" : ""),
                       dontSendNotification);
    programs.setSelectedId(proc.getCurrentProgram() + 1, dontSendNotification);
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
void LowlineEditor::preset(bool save)
{
    chooser = std::make_unique<FileChooser>(
        save ? "Save Lowline preset" : "Open Lowline preset",
        File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("My bass.lowline"), "*.lowline");
    Component::SafePointer<LowlineEditor> safe(this);
    chooser->launchAsync((save ? FileBrowserComponent::saveMode | FileBrowserComponent::warnAboutOverwriting
                               : FileBrowserComponent::openMode) |
                             FileBrowserComponent::canSelectFiles,
                         [safe, save](const FileChooser &c)
                         {
                             if (safe && c.getResult() != File{})
                             {
                                 auto f = c.getResult();
                                 if (save)
                                     safe->proc.savePreset(f.withFileExtension("lowline"));
                                 else
                                     safe->proc.openPreset(f);
                             }
                         });
}
bool LowlineEditor::isInterestedInFileDrag(const StringArray &files)
{
    return files.size() == 1 && (File(files[0]).hasFileExtension("nam;wav;aiff;aif;lowline"));
}
void LowlineEditor::filesDropped(const StringArray &files, int, int y)
{
    if (files.isEmpty())
        return;
    File f(files[0]);
    if (f.hasFileExtension("lowline"))
        proc.openPreset(f);
    else
        proc.loadFile(f.hasFileExtension("nam") ? LowlineProcessor::Amp
                      : y >= 579                ? LowlineProcessor::Room
                                                : LowlineProcessor::Cab,
                      f);
}
