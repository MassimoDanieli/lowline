#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <LowlineHelpData.h>

// Read the same UTF-8 manuals that are published in docs/. No network or browser is needed.
class LowlineHelpContent : public juce::Component
{
  public:
    explicit LowlineHelpContent(bool italian)
    {
        addAndMakeVisible(title);
        addAndMakeVisible(topics);
        addAndMakeVisible(language);
        addAndMakeVisible(body);
        title.setFont(juce::Font(24.f, juce::Font::bold));
        title.setColour(juce::Label::textColourId, juce::Colour(0xff2ef2a0));
        body.setMultiLine(true, true);
        body.setReadOnly(true);
        body.setScrollbarsShown(true);
        body.setCaretVisible(false);
        body.setPopupMenuEnabled(true);
        body.setFont(juce::Font(16.f));
        body.setIndents(18, 14);
        body.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1c232d));
        body.setColour(juce::TextEditor::textColourId, juce::Colour(0xfff3f6fa));
        body.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff46566b));
        topics.onChange = [this] { showTopic(); };
        language.onClick = [this] { setLanguage(!isItalian); };
        setSize(860, 640);
        setLanguage(italian);
    }

    void setLanguage(bool italian)
    {
        const int previous = juce::jmax(1, topics.getSelectedId());
        isItalian = italian;
        headings.clear();
        sections.clear();
        const auto manual = juce::String::fromUTF8(
            italian ? LowlineHelpData::HELP_IT_md : LowlineHelpData::HELP_EN_md,
            italian ? LowlineHelpData::HELP_IT_mdSize : LowlineHelpData::HELP_EN_mdSize);
        juce::StringArray lines;
        lines.addLines(manual);
        for (const auto &line : lines)
        {
            if (line.startsWith("## "))
            {
                headings.add(line.substring(3));
                sections.add(juce::String());
            }
            else if (!sections.isEmpty())
                sections.getReference(sections.size() - 1) += line + "\n";
        }
        title.setText(italian ? "Radice / Guida utente" : "Radice / User guide",
                      juce::dontSendNotification);
        language.setButtonText(italian ? "ENGLISH" : "ITALIANO");
        topics.clear(juce::dontSendNotification);
        topics.addItemList(headings, 1);
        topics.setSelectedId(juce::jmin(previous, headings.size()), juce::dontSendNotification);
        showTopic();
    }

    int getTopicCount() const { return sections.size(); }
    void selectTopic(int index) { topics.setSelectedId(index + 1, juce::sendNotificationSync); }
    juce::String getDisplayedText() const { return body.getText(); }

    void paint(juce::Graphics &g) override { g.fillAll(juce::Colour(0xff11151b)); }
    void resized() override
    {
        auto area = getLocalBounds().reduced(20);
        auto header = area.removeFromTop(38);
        language.setBounds(header.removeFromRight(105).reduced(0, 4));
        title.setBounds(header);
        area.removeFromTop(10);
        topics.setBounds(area.removeFromTop(34));
        area.removeFromTop(14);
        body.setBounds(area);
    }

  private:
    void showTopic()
    {
        const int index = topics.getSelectedId() - 1;
        body.setText(index >= 0 && index < sections.size()
                         ? headings[index] + "\n\n" + sections[index].trim()
                         : juce::String(),
                     false);
        body.setCaretPosition(0);
    }
    bool isItalian = true;
    juce::Label title;
    juce::ComboBox topics;
    juce::TextButton language;
    juce::TextEditor body;
    juce::StringArray headings, sections;
};

// iPad and AUv3: the guide opens inside the interface (no separate windows there).
class LowlineHelpOverlay : public juce::Component
{
  public:
    explicit LowlineHelpOverlay(bool italian) : content(italian)
    {
        addAndMakeVisible(content);
        addAndMakeVisible(close);
        close.onClick = [this] { setVisible(false); };
        close.setComponentID("lowline-help-close");
    }
    void setLanguage(bool italian) { content.setLanguage(italian); }
    void paint(juce::Graphics &g) override { g.fillAll(juce::Colour(0xe011151b)); }
    void resized() override
    {
        auto area = getLocalBounds().reduced(40, 30);
        close.setBounds(area.removeFromTop(40).removeFromRight(120));
        content.setBounds(area);
    }

  private:
    LowlineHelpContent content;
    juce::TextButton close{"OK"};
};

class LowlineHelpWindow : public juce::DocumentWindow
{
  public:
    explicit LowlineHelpWindow(bool italian)
        : DocumentWindow("Radice / Help", juce::Colour(0xff11151b), closeButton)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new LowlineHelpContent(italian), true);
        setResizable(true, false);
        setResizeLimits(640, 460, 1400, 1000);
        centreWithSize(getWidth(), getHeight());
    }
    void setLanguage(bool italian)
    {
        static_cast<LowlineHelpContent *>(getContentComponent())->setLanguage(italian);
    }
    void closeButtonPressed() override { setVisible(false); }
};
