#pragma once
#include "Processor.h"

class LowlineWorkbench : public juce::Component,
                         private juce::Timer,
                         private juce::ListBoxModel,
                         private juce::Thread
{
  public:
    explicit LowlineWorkbench(LowlineProcessor &);
    ~LowlineWorkbench() override;
    void resized() override;
    void paint(juce::Graphics &) override;
    bool keyPressed(const juce::KeyPress &) override;
    void showPage(int);
    int currentPage() const
    {
        return page;
    }

  private:
    struct Item
    {
        juce::String path, tags, title;
        bool favourite = false;
        juce::int64 recent = 0;
    };
    LowlineProcessor &proc;
    int page = 0;
    juce::String language;
    std::map<juce::String, std::unique_ptr<juce::TextButton>> buttons;
    std::map<juce::String, std::unique_ptr<juce::Slider>> sliders;
    std::map<juce::String, std::unique_ptr<juce::Label>> labels;
    juce::ComboBox kind, target, accents, meter;
    juce::TextEditor search, name, tags, notes, itemTags;
    juce::ListBox list;
    juce::Label info, heading, stageTitle, stageTuner, diagnostic;
    juce::ToggleButton favourites, recents;
    std::unique_ptr<juce::FileChooser> chooser;
    std::vector<Item> items;
    std::vector<int> visible;
    std::mutex scanLock;
    juce::File scanFolder;
    std::vector<juce::File> scanned;
    std::atomic<bool> scanReady{false}, scanning{false};
    int requestedScan = 0, finishedScan = 0;
    juce::String lastPresetDetails;
    void run() override;
    void timerCallback() override;
    int getNumRows() override
    {
        return int(visible.size());
    }
    void paintListBoxItem(int, juce::Graphics &, int, int, bool) override;
    void listBoxItemDoubleClicked(int, const juce::MouseEvent &) override;
    void selectedRowsChanged(int) override;
    void refresh();
    void addFiles(const juce::Array<juce::File> &);
    void saveCatalog();
    void chooseFiles(bool folder);
    void activate(int direction = 0);
    void button(const char *, const char *, const char *, std::function<void()>);
    void slider(const char *, const char *, const char *, double, double, double, double,
                std::function<void(double)>);
    void layoutButton(const char *, int, int, int = 150, int = 34);
    void layoutSlider(const char *, int, int, int = 220);
    void translate();
    void chooseBacking();
    void record();
    bool it() const
    {
        return proc.getLanguage() == "IT";
    }
    juce::File catalog() const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowlineWorkbench)
};
