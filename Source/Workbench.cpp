#include "Workbench.h"
using namespace juce;
namespace
{
Colour panel(0xff1c232d), green(0xff2ef2a0), amber(0xffffbf47);
}
LowlineWorkbench::LowlineWorkbench(LowlineProcessor &p)
    : Thread("Radice library scan"), proc(p), list("Library", this)
{
    setWantsKeyboardFocus(true);
    button("library", "LIBRERIA", "LIBRARY", [this] { showPage(0); });
    button("compare", "CONFRONTO A/B", "COMPARE A/B", [this] { showPage(1); });
    button("studio", "STUDIO", "PRACTICE", [this] { showPage(2); });
    button("stage", "PALCO", "STAGE", [this] { showPage(3); });
    button("setup", "AUDIO / APP", "AUDIO / APP", [this] { showPage(4); });
    button("add", "AGGIUNGI FILE", "ADD FILES", [this] { chooseFiles(false); });
    button("folder", "SCANSIONA CARTELLA", "SCAN FOLDER", [this] { chooseFiles(true); });
    button("load", "CARICA", "LOAD", [this] { activate(); });
    button("prev", "< PRECEDENTE", "< PREVIOUS", [this] { activate(-1); });
    button("next", "SUCCESSIVO >", "NEXT >", [this] { activate(1); });
    button("favourite", "PREFERITO +/-", "FAVOURITE +/-", [this] {
        int r = list.getSelectedRow();
        if (isPositiveAndBelow(r, int(visible.size())))
        {
            auto &i = items[size_t(visible[size_t(r)])];
            i.favourite = !i.favourite;
            saveCatalog();
            refresh();
        }
    });
    button("tagItem", "TAG VOCE", "TAG ITEM", [this] {
        int r = list.getSelectedRow();
        if (isPositiveAndBelow(r, int(visible.size())))
        {
            auto &e = items[size_t(visible[size_t(r)])];
            e.tags = itemTags.getText().trim();
            saveCatalog();
            refresh();
        }
    });
    button("remove", "RIMUOVI DALLA LISTA", "REMOVE FROM LIST", [this] {
        int r = list.getSelectedRow();
        if (isPositiveAndBelow(r, int(visible.size())))
        {
            items.erase(items.begin() + visible[size_t(r)]);
            saveCatalog();
            refresh();
        }
    });
    button("reveal", "MOSTRA FILE", "SHOW FILE", [this] {
        int r = list.getSelectedRow();
        if (isPositiveAndBelow(r, int(visible.size())))
            File(items[size_t(visible[size_t(r)])].path).revealToUser();
    });
    button("details", "APPLICA DATI PRESET", "APPLY PRESET DETAILS",
           [this] { proc.setPresetDetails(name.getText(), tags.getText(), notes.getText()); });
    button("undo", "ANNULLA", "UNDO", [this] { proc.undo.undo(); });
    button("redo", "RIPRISTINA", "REDO", [this] { proc.undo.redo(); });
    button("storeA", "MEMORIZZA A", "STORE A", [this] { proc.storeComparison(0); });
    button("storeB", "MEMORIZZA B", "STORE B", [this] { proc.storeComparison(1); });
    button("recallA", "ASCOLTA A", "RECALL A", [this] { proc.recallComparison(0); });
    button("recallB", "ASCOLTA B", "RECALL B", [this] { proc.recallComparison(1); });
    button("measure", "MISURA 5 SECONDI", "MEASURE 5 SECONDS", [this] { proc.measureLevel(); });
    button("matchA", "ALLINEA AD A", "MATCH TO A", [this] { proc.matchComparison(0); });
    button("matchB", "ALLINEA A B", "MATCH TO B", [this] { proc.matchComparison(1); });
    button("resetTrim", "AZZERA COMPENSAZIONE", "RESET MATCH GAIN", [this] { proc.compareTrim = 0; });
    button("backing", "CARICA BASE WAV/AIFF", "LOAD WAV/AIFF", [this] { chooseBacking(); });
    button("play", "PLAY / PAUSA", "PLAY / PAUSE", [this] { proc.studio.playing = !proc.studio.playing; });
    button("stop", "STOP / INIZIO", "STOP / START", [this] {
        proc.studio.playing = false;
        proc.studio.seek(0);
    });
    button("loop", "LOOP ON/OFF", "LOOP ON/OFF", [this] { proc.studio.looping = !proc.studio.looping; });
    button("loopA", "INIZIO LOOP = ORA", "LOOP START = NOW",
           [this] { proc.studio.loopStart = proc.studio.position.load(); });
    button("loopB", "FINE LOOP = ORA", "LOOP END = NOW",
           [this] { proc.studio.loopEnd = proc.studio.position.load(); });
    button("render", "APPLICA VELOCITA", "APPLY SPEED", [this] {
        auto f = proc.studio.file();
        if (f.existsAsFile())
            proc.studio.load(f, float(sliders.at("speed")->getValue()));
    });
    button("click", "METRONOMO ON/OFF", "METRONOME ON/OFF",
           [this] { proc.studio.click = !proc.studio.click; });
    button("beatOne", "BATTITO 1 QUI", "BEAT 1 HERE", [this] { proc.studio.setBeatOneHere(); });
    button("record", "REGISTRA DI + FX", "RECORD DI + FX", [this] { record(); });
    button("endRecord", "FERMA REGISTRAZIONE", "STOP RECORDING", [this] { proc.studio.stopRecording(); });
    button("stagePrev", "< PRESET", "< PRESET", [this] { activate(-1); });
    button("stageNext", "PRESET >", "PRESET >", [this] { activate(1); });
    button("tune", "ACCORDA / MUTE", "TUNE / MUTE", [this] {
        auto *v = proc.state.getParameter("tunerOn");
        const bool on = v->getValue() < .5f;
        proc.state.getParameter("tunerMute")->setValueNotifyingHost(1);
        v->setValueNotifyingHost(on ? 1.f : 0.f);
    });
    button("tap", "TAP TEMPO", "TAP TEMPO", [this] { proc.tapTempo(); });
    button("bypass", "BYPASS ON/OFF", "BYPASS ON/OFF", [this] {
        auto *v = proc.state.getParameter("bypass");
        v->setValueNotifyingHost(v->getValue() < .5f ? 1.f : 0.f);
    });
    button("updates", "CONTROLLA AGGIORNAMENTI", "CHECK FOR UPDATES",
           [] { URL("https://github.com/MassimoDanieli/radice/releases/latest").launchInDefaultBrowser(); });
    button("guide", "GUIDA CONFIGURAZIONE", "SETUP GUIDE", [this] {
        URL(String("https://github.com/MassimoDanieli/radice/blob/main/docs/HELP_") + (it() ? "IT" : "EN") +
            ".md")
            .launchInDefaultBrowser();
    });
    button("diagnostics", "COPIA DIAGNOSTICA", "COPY DIAGNOSTICS",
           [this] { SystemClipboard::copyTextToClipboard(diagnostic.getText()); });
    slider("speed", "VELOCITA (mantiene tonalita)", "SPEED (retains pitch)", .5, 1.5, .05, 1, [](double) {});
    slider("backingGain", "VOLUME BASE", "BACKING LEVEL", 0, 1, .01, .35,
           [this](double v) { proc.studio.backingGain = float(v); });
    slider("clickGain", "VOLUME METRONOMO", "CLICK LEVEL", 0, .5, .01, .12,
           [this](double v) { proc.studio.clickGain = float(v); });
    slider("bpm", "TEMPO METRONOMO", "METRONOME TEMPO", 30, 300, 1, 80,
           [this](double v) { proc.studio.bpm = float(v); });
    slider("position", "POSIZIONE (secondi originali)", "POSITION (original seconds)", 0, 600, .1, 0,
           [this](double v) { proc.studio.seek(v); });
    slider("trim", "COMPENSAZIONE A/B (dB)", "A/B MATCH GAIN (dB)", -12, 12, .1, 0,
           [this](double v) { proc.compareTrim = float(v); });
    auto beatLabel = std::make_unique<Label>();
    beatLabel->getProperties().set("it", "battiti per misura");
    beatLabel->getProperties().set("en", "beats per bar");
    addAndMakeVisible(*beatLabel);
    labels["beatCount"] = std::move(beatLabel);
    for (auto *c : std::initializer_list<Component *>{&kind, &target, &accents, &meter, &search, &name, &tags,
                                                      &notes, &itemTags, &list, &info, &heading, &stageTitle,
                                                      &stageTuner, &diagnostic, &favourites, &recents})
        addAndMakeVisible(*c);
    kind.addItemList({"NAM", "IR", "Preset"}, 1);
    kind.setSelectedId(1);
    kind.onChange = [this] { refresh(); };
    target.addItemList({"Pedal", "Amp", "Cab", "Room"}, 1);
    target.setSelectedId(2);
    accents.setComponentID("practice-accents");
    accents.addItemList({"No accent", "Beat 1", "Beats 2 + 4"}, 1);
    accents.setSelectedId(2);
    accents.onChange = [this] { proc.studio.accent = accents.getSelectedId() - 1; };
    meter.addItemList({"2", "3", "4", "5", "6", "7"}, 2);
    meter.setSelectedId(4);
    meter.onChange = [this] { proc.studio.beats = meter.getSelectedId(); };
    search.onTextChange = [this] { refresh(); };
    notes.setMultiLine(true);
    notes.setReturnKeyStartsNewLine(true);
    for (auto *e : {&search, &name, &tags, &notes, &itemTags})
    {
        e->setColour(TextEditor::backgroundColourId, Colour(0xff11151b));
        e->setColour(TextEditor::textColourId, Colours::white);
    }
    favourites.onClick = [this] { refresh(); };
    recents.onClick = [this] { refresh(); };
    list.setRowHeight(38);
    list.setColour(ListBox::backgroundColourId, Colour(0xff11151b));
    stageTitle.setFont(Font(32.f, Font::bold));
    stageTitle.setColour(Label::textColourId, green);
    stageTuner.setFont(Font(48.f, Font::bold));
    stageTuner.setJustificationType(Justification::centred);
    heading.setFont(Font(19.f, Font::bold));
    heading.setColour(Label::textColourId, green);
    info.setFont(Font(15.f));
    diagnostic.setFont(Font(17.f));
    if (auto xml = XmlDocument::parse(catalog()))
        for (auto *e : xml->getChildIterator())
            items.push_back({e->getStringAttribute("path"), e->getStringAttribute("tags"),
                             e->getStringAttribute("title"), e->getBoolAttribute("favourite"),
                             e->getStringAttribute("recent").getLargeIntValue()});
    sliders.at("speed")->setValue(proc.studio.speed, dontSendNotification);
    sliders.at("backingGain")->setValue(proc.studio.backingGain, dontSendNotification);
    sliders.at("clickGain")->setValue(proc.studio.clickGain, dontSendNotification);
    sliders.at("bpm")->setValue(proc.studio.bpm, dontSendNotification);
    meter.setSelectedId(proc.studio.beats, dontSendNotification);
    refresh();
    translate();
    showPage(0);
    startThread();
    startTimerHz(4);
}
LowlineWorkbench::~LowlineWorkbench()
{
    stopTimer();
    signalThreadShouldExit();
    notify();
    stopThread(-1);
    saveCatalog();
}
File LowlineWorkbench::catalog() const
{
    return lowline::platform::appData().getChildFile("library.xml");
}
void LowlineWorkbench::button(const char *id, const char *italian, const char *english,
                              std::function<void()> f)
{
    auto b = std::make_unique<TextButton>();
    b->setComponentID(id);
    b->getProperties().set("it", italian);
    b->getProperties().set("en", english);
    b->onClick = std::move(f);
    addAndMakeVisible(*b);
    buttons[id] = std::move(b);
}
void LowlineWorkbench::slider(const char *id, const char *italian, const char *english, double lo, double hi,
                              double step, double value, std::function<void(double)> f)
{
    auto s = std::make_unique<Slider>();
    s->setSliderStyle(Slider::LinearHorizontal);
    s->setTextBoxStyle(Slider::TextBoxRight, false, 65, 26);
    s->setRange(lo, hi, step);
    s->setValue(value);
    auto *ptr = s.get();
    s->onValueChange = [ptr, f] { f(ptr->getValue()); };
    addAndMakeVisible(*s);
    sliders[id] = std::move(s);
    auto l = std::make_unique<Label>();
    l->getProperties().set("it", italian);
    l->getProperties().set("en", english);
    l->setFont(Font(13.f));
    addAndMakeVisible(*l);
    labels[id] = std::move(l);
}
void LowlineWorkbench::translate()
{
    language = proc.getLanguage();
    for (auto &[id, b] : buttons)
        b->setButtonText(b->getProperties()[it() ? "it" : "en"].toString());
    for (auto &[id, l] : labels)
        l->setText(l->getProperties()[it() ? "it" : "en"].toString(), dontSendNotification);
    favourites.setButtonText(it() ? "Solo preferiti" : "Favourites only");
    recents.setButtonText(it() ? "Ordina per recenti" : "Sort by recent");
    search.setTextToShowWhenEmpty(it() ? "Cerca nome, percorso o tag..." : "Search name, path or tags...",
                                  Colours::grey);
    itemTags.setTextToShowWhenEmpty(it() ? "Tag della voce selezionata" : "Selected item tags",
                                    Colours::grey);
    name.setTextToShowWhenEmpty(it() ? "Nome preset corrente" : "Current preset name", Colours::grey);
    tags.setTextToShowWhenEmpty(it() ? "Tag: jazz, clean, live..." : "Tags: jazz, clean, live...",
                                Colours::grey);
    notes.setTextToShowWhenEmpty(it() ? "Descrizione del preset corrente" : "Current preset description",
                                 Colours::grey);
    accents.changeItemText(1, it() ? "Nessun accento" : "No accent");
    accents.changeItemText(2, it() ? "Accento su 1" : "Accent on 1");
    accents.changeItemText(3, it() ? "Accenti 2 e 4" : "Accents 2 and 4");
    accents.setSelectedId(proc.studio.accent.load() + 1, dontSendNotification);
}
void LowlineWorkbench::showPage(int next)
{
    page = jlimit(0, 4, next);
    if (page == 3)
    {
        kind.setSelectedId(3, dontSendNotification);
        refresh();
    }
    resized();
    timerCallback();
    repaint();
}
void LowlineWorkbench::paint(Graphics &g)
{
    g.setColour(panel);
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 12);
}
void LowlineWorkbench::layoutButton(const char *id, int x, int y, int w, int h)
{
    auto &b = *buttons.at(id);
    b.setVisible(true);
    b.setBounds(x, y, w, h);
}
void LowlineWorkbench::layoutSlider(const char *id, int x, int y, int w)
{
    labels.at(id)->setVisible(true);
    labels.at(id)->setBounds(x, y, w, 22);
    sliders.at(id)->setVisible(true);
    sliders.at(id)->setBounds(x, y + 24, w, 32);
}
void LowlineWorkbench::resized()
{
    for (auto &[id, b] : buttons)
        b->setVisible(false);
    for (auto &[id, s] : sliders)
        s->setVisible(false);
    for (auto &[id, l] : labels)
        l->setVisible(false);
    for (auto *c : std::initializer_list<Component *>{&kind, &target, &accents, &meter, &search, &name, &tags,
                                                      &notes, &itemTags, &list, &stageTitle, &stageTuner,
                                                      &diagnostic, &favourites, &recents})
        c->setVisible(false);
    const char *tabs[] = {"library", "compare", "studio", "stage", "setup"};
    for (int i = 0; i < 5; ++i)
    {
        layoutButton(tabs[i], 18 + i * 219, 16, 207);
        buttons.at(tabs[i])->setToggleState(page == i, dontSendNotification);
    }
    heading.setBounds(22, 62, 1080, 34);
    info.setBounds(22, 515, 1080, 106);
    if (page == 0)
    {
        for (auto *c : std::initializer_list<Component *>{&kind, &target, &search, &name, &tags, &notes,
                                                          &itemTags, &list, &favourites, &recents})
            c->setVisible(true);
        kind.setBounds(22, 103, 95, 30);
        search.setBounds(127, 103, 490, 30);
        target.setBounds(628, 103, 150, 30);
        favourites.setBounds(790, 103, 150, 30);
        recents.setBounds(945, 103, 170, 30);
        list.setBounds(22, 148, 752, 296);
        name.setBounds(795, 148, 315, 32);
        tags.setBounds(795, 190, 315, 32);
        notes.setBounds(795, 232, 315, 140);
        layoutButton("details", 795, 384, 315);
        layoutButton("undo", 795, 430, 150);
        layoutButton("redo", 960, 430, 150);
        layoutButton("load", 22, 458, 130);
        layoutButton("prev", 162, 458, 145);
        layoutButton("next", 317, 458, 145);
        layoutButton("favourite", 472, 458, 145);
        layoutButton("reveal", 627, 458, 147);
        buttons.at("reveal")->setVisible(!lowline::platform::sandboxed()); // no Finder on iPad
        layoutButton("add", 22, 510, 170);
        layoutButton("folder", 202, 510, 220);
        layoutButton("remove", 432, 510, 230);
        layoutButton("tagItem", 682, 510, 150);
        itemTags.setBounds(845, 510, 265, 34);
        info.setBounds(22, 553, 1080, 65);
    }
    if (page == 1)
    {
        layoutButton("storeA", 40, 130, 240, 50);
        layoutButton("recallA", 40, 196, 240, 70);
        layoutButton("storeB", 330, 130, 240, 50);
        layoutButton("recallB", 330, 196, 240, 70);
        layoutButton("measure", 635, 130, 430, 50);
        layoutButton("matchA", 635, 196, 205, 70);
        layoutButton("matchB", 860, 196, 205, 70);
        layoutSlider("trim", 40, 310, 530);
        layoutButton("resetTrim", 635, 334, 430);
        info.setBounds(40, 400, 1030, 200);
    }
    if (page == 2)
    {
        layoutButton("backing", 22, 108, 260);
        layoutButton("play", 298, 108, 170);
        layoutButton("stop", 484, 108, 170);
        layoutButton("loop", 670, 108, 180);
        layoutSlider("position", 22, 160, 750);
        layoutButton("loopA", 795, 160, 315);
        layoutButton("loopB", 795, 205, 315);
        layoutSlider("speed", 22, 245, 330);
        layoutButton("render", 380, 269, 230);
        layoutSlider("backingGain", 660, 260, 450);
        layoutButton("click", 22, 352, 260);
        layoutSlider("bpm", 310, 328, 300);
        layoutSlider("clickGain", 660, 328, 450);
        accents.setVisible(true);
        meter.setVisible(true);
        accents.setBounds(22, 405, 260, 32);
        meter.setBounds(310, 405, 105, 32);
        labels.at("beatCount")->setVisible(true);
        labels.at("beatCount")->setBounds(430, 405, 210, 32);
        layoutButton("beatOne", 660, 400, 450, 40);
        layoutButton("record", 22, 468, 300);
        layoutButton("endRecord", 340, 468, 300);
    }
    if (page == 3)
    {
        stageTitle.setVisible(true);
        stageTuner.setVisible(true);
        stageTitle.setBounds(40, 110, 1020, 75);
        stageTuner.setBounds(40, 190, 1020, 120);
        layoutButton("stagePrev", 40, 330, 300, 80);
        layoutButton("stageNext", 375, 330, 300, 80);
        layoutButton("tune", 710, 330, 360, 80);
        layoutButton("tap", 40, 440, 460, 60);
        layoutButton("bypass", 540, 440, 530, 60);
    }
    if (page == 4)
    {
        diagnostic.setVisible(true);
        diagnostic.setBounds(35, 105, 1050, 170);
        layoutButton("guide", 35, 300, 325);
        layoutButton("diagnostics", 390, 300, 325);
        layoutButton("updates", 745, 300, 325);
        info.setBounds(35, 365, 1040, 245);
    }
}
void LowlineWorkbench::refresh()
{
    visible.clear();
    const auto q = search.getText().trim();
    for (int i = 0; i < int(items.size()); ++i)
    {
        auto &e = items[size_t(i)];
        File f(e.path);
        const bool type = kind.getSelectedId() == 1   ? f.hasFileExtension("nam")
                          : kind.getSelectedId() == 2 ? f.hasFileExtension("wav;aiff;aif")
                                                      : lowline::platform::isPreset(f);
        if (type && (!favourites.getToggleState() || e.favourite) &&
            (q.isEmpty() || (e.path + " " + e.tags + " " + e.title).containsIgnoreCase(q)))
            visible.push_back(i);
    }
    std::stable_sort(visible.begin(), visible.end(), [this](int a, int b) {
        if (recents.getToggleState())
            return items[size_t(a)].recent > items[size_t(b)].recent;
        return File(items[size_t(a)].path)
                   .getFileName()
                   .compareNatural(File(items[size_t(b)].path).getFileName()) < 0;
    });
    list.updateContent();
    list.repaint();
}
void LowlineWorkbench::paintListBoxItem(int row, Graphics &g, int width, int height, bool selected)
{
    if (!isPositiveAndBelow(row, int(visible.size())))
        return;
    auto &e = items[size_t(visible[size_t(row)])];
    File f(e.path);
    if (selected)
        g.fillAll(Colour(0xff315746));
    g.setColour(f.existsAsFile() ? Colours::white : Colour(0xffff785d));
    g.setFont(Font(15.f));
    g.drawText(String(e.favourite ? "*  " : "") + (f.existsAsFile() ? "" : "[MISSING] ") + f.getFileName() +
                   "   " + e.tags,
               10, 0, width - 20, height, Justification::centredLeft);
}
void LowlineWorkbench::selectedRowsChanged(int row)
{
    if (isPositiveAndBelow(row, int(visible.size())))
    {
        auto &e = items[size_t(visible[size_t(row)])];
        list.setTooltip(e.path);
        itemTags.setText(e.tags, false);
    }
}
void LowlineWorkbench::listBoxItemDoubleClicked(int, const MouseEvent &)
{
    activate();
}
void LowlineWorkbench::activate(int direction)
{
    if (visible.empty())
        return;
    int row = list.getSelectedRow();
    if (row < 0)
        row = direction < 0 ? int(visible.size()) - 1 : 0;
    else
        row = (row + direction + int(visible.size())) % int(visible.size());
    list.selectRow(row);
    auto &e = items[size_t(visible[size_t(row)])];
    File f = lowline::platform::relocate(File(e.path)); // iOS container moved after an update
    if (!f.existsAsFile())
        return;
    e.path = f.getFullPathName();
    if (lowline::platform::isPreset(f))
    {
        proc.openPreset(f);
        e.tags = proc.presetTags();
        e.title = proc.presetName();
    }
    else
    {
        int slot = target.getSelectedId() - 1;
        if (f.hasFileExtension("nam") && slot >= 2)
            slot = 1;
        if (!f.hasFileExtension("nam") && slot < 2)
            slot = 2;
        proc.loadFile(LowlineProcessor::Slot(slot), f);
    }
    e.recent = Time::currentTimeMillis();
    saveCatalog();
}
void LowlineWorkbench::addFiles(const Array<File> &files)
{
    for (auto &f : files)
    {
        if (!f.hasFileExtension("nam;wav;aiff;aif;radice;lowline"))
            continue;
        const auto path = f.getFullPathName();
        if (std::none_of(items.begin(), items.end(), [&](auto &e) { return e.path == path; }))
            items.push_back({path, "", "", false, 0});
    }
    saveCatalog();
    refresh();
}
void LowlineWorkbench::saveCatalog()
{
    XmlElement root("LowlineLibrary");
    for (auto &e : items)
    {
        auto *x = root.createNewChildElement("item");
        x->setAttribute("path", e.path);
        x->setAttribute("tags", e.tags);
        x->setAttribute("title", e.title);
        x->setAttribute("favourite", e.favourite);
        x->setAttribute("recent", String(e.recent));
    }
    auto f = catalog();
    if (f.getParentDirectory().createDirectory().failed())
        return;
    TemporaryFile tmp(f);
    if (root.writeTo(tmp.getFile()))
        tmp.overwriteTargetFileWithTemporary();
}
void LowlineWorkbench::chooseFiles(bool folder)
{
    if (folder && lowline::platform::sandboxed())
    {
        // iPad: the library folder is Radice's own Documents ("On My iPad > Radice" in Files).
        {
            std::lock_guard l(scanLock);
            scanFolder = lowline::platform::documents();
            ++requestedScan;
        }
        scanning = true;
        notify();
        return;
    }
    chooser = std::make_unique<FileChooser>(it() ? "Aggiungi alla libreria" : "Add to library", File{},
                                            lowline::platform::pickerFilter("*.nam;*.wav;*.aif;*.aiff;" + lowline::platform::presetPattern()));
    Component::SafePointer<LowlineWorkbench> safe(this);
    chooser->launchAsync(
        FileBrowserComponent::openMode |
            (folder ? FileBrowserComponent::canSelectDirectories
                    : FileBrowserComponent::canSelectFiles | FileBrowserComponent::canSelectMultipleItems),
        [safe, folder](const FileChooser &c) {
            if (!safe)
                return;
            if (folder && c.getResult().isDirectory())
            {
                {
                    std::lock_guard l(safe->scanLock);
                    safe->scanFolder = c.getResult();
                    ++safe->requestedScan;
                }
                safe->scanning = true;
                safe->notify();
            }
            else
                safe->addFiles(lowline::platform::adoptAll(c, "Library"));
        });
}
void LowlineWorkbench::run()
{
    while (!threadShouldExit())
    {
        File folder;
        int version;
        {
            std::lock_guard l(scanLock);
            folder = scanFolder;
            version = requestedScan;
        }
        if (version == finishedScan)
        {
            wait(100);
            continue;
        }
        std::vector<File> result;
        for (auto entry :
             RangedDirectoryIterator(folder, true, "*", File::findFiles, File::FollowSymlinks::no))
        {
            if (threadShouldExit())
                return;
            auto f = entry.getFile();
            if (f.hasFileExtension("nam;wav;aif;aiff;radice;lowline"))
                result.push_back(f);
            if (result.size() >= 10000)
                break;
        }
        {
            std::lock_guard l(scanLock);
            scanned = std::move(result);
            finishedScan = version;
        }
        scanReady = true;
        scanning = false;
    }
}
void LowlineWorkbench::chooseBacking()
{
    chooser = std::make_unique<FileChooser>(it() ? "Carica base" : "Load backing track", File{},
                                            lowline::platform::pickerFilter("*.wav;*.aiff;*.aif"));
    Component::SafePointer<LowlineWorkbench> safe(this);
    chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                         [safe](const FileChooser &c) {
                             const auto f = lowline::platform::adoptFirst(c, "Backing");
                             if (safe && f.existsAsFile())
                                 safe->proc.studio.load(f, 1);
                         });
}
void LowlineWorkbench::record()
{
    if (proc.studio.recording)
        return;
    if (lowline::platform::sandboxed())
    {
        // iPad: takes go to Documents/Recordings, no dialog (a save picker cannot stream a long take).
        proc.studio.startRecording(lowline::platform::folder("Recordings")
            .getChildFile("Radice-" + Time::getCurrentTime().formatted("%Y%m%d-%H%M%S") + ".wav"));
        return;
    }
    auto f = File::getSpecialLocation(File::userDocumentsDirectory)
                 .getChildFile("Radice-" + Time::getCurrentTime().formatted("%Y%m%d-%H%M%S") + ".wav");
    chooser = std::make_unique<FileChooser>(
        it() ? "Scegli nome e cartella della registrazione" : "Choose recording name and folder", f, "*.wav");
    Component::SafePointer<LowlineWorkbench> safe(this);
    chooser->launchAsync(FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles,
                         [safe](const FileChooser &c) {
                             if (safe && c.getResult() != File{})
                                 safe->proc.studio.startRecording(c.getResult());
                         });
}
void LowlineWorkbench::timerCallback()
{
    if (language != proc.getLanguage())
        translate();
    if (scanReady.exchange(false))
    {
        Array<File> files;
        {
            std::lock_guard l(scanLock);
            for (auto &f : scanned)
                files.add(f);
            scanned.clear();
        }
        addFiles(files);
    }
    const auto details = proc.presetName() + "\n" + proc.presetTags() + "\n" + proc.presetNotes();
    if (details != lastPresetDetails && !name.hasKeyboardFocus(true) && !tags.hasKeyboardFocus(true) &&
        !notes.hasKeyboardFocus(true))
    {
        lastPresetDetails = details;
        name.setText(proc.presetName(), false);
        tags.setText(proc.presetTags(), false);
        notes.setText(proc.presetNotes(), false);
    }
    buttons.at("undo")->setEnabled(proc.undo.canUndo());
    buttons.at("redo")->setEnabled(proc.undo.canRedo());
    buttons.at("recallA")->setEnabled(proc.hasComparison(0) && !proc.busy);
    buttons.at("recallB")->setEnabled(proc.hasComparison(1) && !proc.busy);
    buttons.at("loop")->setToggleState(proc.studio.looping, dontSendNotification);
    buttons.at("click")->setToggleState(proc.studio.click, dontSendNotification);
    buttons.at("beatOne")->setToggleState(proc.studio.beatOne.load() >= 0, dontSendNotification);
    buttons.at("play")->setToggleState(proc.studio.playing, dontSendNotification);
    buttons.at("record")->setToggleState(proc.studio.recording, dontSendNotification);
    buttons.at("render")->setEnabled(!proc.studio.loading);
    buttons.at("record")->setEnabled(!proc.studio.recording);
    buttons.at("endRecord")->setEnabled(proc.studio.recording);
    if (!sliders.at("position")->isMouseButtonDown())
        sliders.at("position")->setValue(proc.studio.position, dontSendNotification);
    sliders.at("trim")->setValue(proc.compareTrim, dontSendNotification);
    String title, body;
    if (page == 0)
    {
        title = (it() ? "Libreria personale" : "Personal library") + String(scanning ? " / SCANNING..." : "");
        body = it() ? "Doppio clic per caricare. I file restano nella posizione originale; rimuovere una "
                      "voce non cancella il file.\nI dati a destra descrivono il preset corrente: premi "
                      "APPLICA e poi SALVA nel rig."
                    : "Double-click to load. Files stay in their original location; removing a row does not "
                      "delete the file.\nDetails on the right describe the current preset: APPLY, then SAVE "
                      "in the rig.";
    }
    if (page == 1)
    {
        title =
            it() ? "Confronta due suoni, con un livello misurato" : "Compare two sounds with measured levels";
        body = (it() ? "1. MEMORIZZA A e suona la frase per 5 s: il livello di A si misura da solo. Ripeti per B.\n"
                       "2. Richiama A o B, risuona la stessa frase (misura automatica) e allinea al riferimento.\n"
                       "La compensazione e RMS, non LUFS; usa lo stesso materiale e livello di ingresso.\n"
                       "A/B include controlli e percorsi delle catture; ANNULLA torna al suono precedente.\n"
                     : "1. STORE A and play the phrase for 5 s: A's level is measured by itself. Repeat for B.\n"
                       "2. Recall A or B, play the same phrase again (measured automatically) and match.\n"
                       "Compensation is RMS, not LUFS; use the same material and input level.\n"
                       "A/B includes controls and capture paths; UNDO returns to the previous sound.\n") +
               String("A: ") + (proc.comparisonLevel(0) > -60 ? String(proc.comparisonLevel(0), 1) + " dB" : String("--")) +
               "   B: " + (proc.comparisonLevel(1) > -60 ? String(proc.comparisonLevel(1), 1) + " dB" : String("--")) +
               String("\nRMS: ") + String(proc.measuredRms.load(), 1) + " dB   /   " +
               String(proc.measureRemaining > 0 ? "MEASURING..." : "READY") + "   /   trim " +
               String(proc.compareTrim.load(), 1) + " dB";
    }
    if (page == 2)
    {
        title = proc.studio.status();
        body =
            String(proc.studio.loading ? "PREPARING...  " : "") +
            "Loop: " + String(proc.studio.loopStart.load(), 1) + " - " +
            String(proc.studio.loopEnd.load(), 1) + " s   /   " + String(proc.studio.speed.load(), 2) +
            "x   /   Beat " + String(proc.studio.beatDisplay.load()) +
            (proc.studio.beatOne.load() >= 0 ? String(it() ? "  (agganciato alla base, 1 = " : "  (locked to backing, 1 = ") + String(proc.studio.beatOne.load(), 2) + " s)" : String()) +
            "\nREC: " + String(proc.studio.recordedSamples.load() / proc.audioRate(), 1) +
            " s   /   dropped blocks: " + String(proc.studio.dropped.load()) + "\n" +
            (it() ? "DI = basso prima degli effetti; FX = basso elaborato. Base e metronomo non vengono "
                    "registrati."
                  : "DI = bass before effects; FX = processed bass. Backing and metronome are not recorded.");
    }
    if (page == 3)
    {
        title = it() ? "Palco - frecce: preset, T: accordatore, spazio: tap"
                     : "Stage - arrows: preset, T: tuner, space: tap";
        stageTitle.setText(proc.presetName() + (proc.isDirty() ? " *" : ""), dontSendNotification);
        float hz = proc.tuner.frequency;
        String tuning = "--";
        if (hz > 0 && proc.state.getRawParameterValue("tunerOn")->load())
        {
            double note = 69 + 12 * std::log2(hz / proc.state.getRawParameterValue("tunerRef")->load());
            int rounded = int(std::round(note));
            const char *names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
            tuning = String(names[(rounded % 12 + 12) % 12]) + String(rounded / 12 - 1) + "    " +
                     String((note - rounded) * 100, 1) + " ct";
        }
        stageTuner.setText(tuning, dontSendNotification);
        body =
            (it() ? "Seleziona Preset nella Libreria per la scaletta. I pulsanti seguono l'ordine filtrato.\n"
                  : "Select Preset in Library for your setlist. Buttons follow the filtered order.\n") +
            String("CPU ") + String(proc.cpuLoad.load(), 1) + "%   /   " +
            String(proc.effectiveTempo.load(), 1) + " BPM";
    }
    if (page == 4)
    {
        title = "Radice " + String(JucePlugin_VersionString) + " / " + SystemStats::getOperatingSystemName();
        diagnostic.setText("Audio: " + String(proc.audioRate(), 0) + " Hz / " + String(proc.audioBlock()) +
                               " samples\n" + (it() ? "Durata di un buffer: " : "One buffer duration: ") +
                               String(1000 * proc.audioBlock() / proc.audioRate(), 2) + " ms\n" +
                               (it() ? "Latenza dichiarata del plugin: " : "Reported plugin latency: ") +
                               String(1000 * proc.getLatencySamples() / proc.audioRate(), 2) +
                               " ms\nIN: " + String(lowline::toDb(proc.inputPeak), 1) +
                               " dBFS   /   OUT: " + String(lowline::toDb(proc.outputPeak), 1) + " dBFS\n" +
                               (proc.inputPeak >= .99f ? (it() ? "INGRESSO IN CLIPPING: riduci il gain fisico"
                                                               : "INPUT CLIPPING: reduce hardware gain")
                                : proc.inputPeak < .0001f
                                    ? (it() ? "Nessun segnale: controlla ingresso, INST e Mute audio input"
                                            : "No signal: check input, INST and Mute audio input")
                                    : (it() ? "Segnale in ingresso presente" : "Input signal detected")),
                           dontSendNotification);
        body = it() ? "1. Standalone: apri Options > Audio/MIDI Settings. Nel plugin usa le impostazioni "
                      "della DAW.\n2. Scegli la stessa interfaccia per ingresso e uscita, poi il canale del "
                      "basso nel rig.\n3. Parti da 48 kHz e 128/256 campioni; attiva INST e disattiva il "
                      "direct monitoring.\n4. Se non senti nulla, controlla Mute audio input nelle opzioni e "
                      "il mute dell'accordatore.\nLa durata del buffer NON e la latenza reale di andata e "
                      "ritorno.\nAggiornamenti: il pulsante apre la release piu recente nel browser; nessun "
                      "controllo automatico."
                    : "1. Standalone: open Options > Audio/MIDI Settings. Plugins use the DAW audio "
                      "settings.\n2. Select the same interface for input and output, then the bass channel "
                      "in the rig.\n3. Start with 48 kHz and 128/256 samples; enable INST and disable direct "
                      "monitoring.\n4. If silent, check Mute audio input in Options and the tuner "
                      "mute.\nBuffer duration is NOT actual round-trip latency.\nUpdates: the button opens "
                      "the latest release in your browser; no automatic checks.";
    }
    heading.setText(title, dontSendNotification);
    info.setText(body, dontSendNotification);
}
bool LowlineWorkbench::keyPressed(const KeyPress &k)
{
    if (page != 3)
        return false;
    if (k.getKeyCode() == KeyPress::leftKey)
    {
        activate(-1);
        return true;
    }
    if (k.getKeyCode() == KeyPress::rightKey)
    {
        activate(1);
        return true;
    }
    if (k.getKeyCode() == KeyPress::spaceKey)
    {
        proc.tapTempo();
        return true;
    }
    if (k.getTextCharacter() == 't' || k.getTextCharacter() == 'T')
    {
        buttons.at("tune")->triggerClick();
        return true;
    }
    return false;
}
