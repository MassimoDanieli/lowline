#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// iPad / iPhone files. An iOS app lives in a sandbox: a file picked in the Files app is only
// borrowed (security-scoped URL) and the app's own container path changes on every update.
// So on iOS Lowline copies what you pick into its Documents folder (visible in Files as
// "On My iPad > Lowline"), saves presets and recordings there directly, and re-roots stored
// paths that point into an older container. On the desktop everything stays as it was.
namespace lowline::platform
{
inline bool &sandboxedForTests()
{
    static bool simulate = false;
    return simulate;
}
inline bool sandboxed()
{
#if JUCE_IOS
    return true;
#else
    return sandboxedForTests();
#endif
}
inline juce::File &documentsForTests()
{
    static juce::File folder;
    return folder;
}
// The app's Documents folder on iOS (root of "On My iPad > Lowline").
inline juce::File documents()
{
    if (documentsForTests() != juce::File{})
        return documentsForTests();
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
}
inline juce::File folder(const juce::String &name)
{
    auto f = documents().getChildFile(name);
    f.createDirectory();
    return f;
}
// The iOS picker does not know ".nam": accept any file there and check the extension ourselves.
inline juce::String pickerFilter(const juce::String &desktopPattern)
{
    return sandboxed() ? juce::String() : desktopPattern;
}
// A stored path that no longer exists but points into some (older) app container's Documents
// folder is re-rooted onto the current one.
inline juce::File relocate(const juce::File &stored)
{
    if (!sandboxed() || stored == juce::File{} || stored.exists())
        return stored;
    const auto path = stored.getFullPathName().replaceCharacter('\\', '/');
    const int at = path.lastIndexOf("/Documents/");
    if (at < 0)
        return stored;
    auto moved = documents().getChildFile(path.substring(at + 11));
    return moved.exists() ? moved : stored;
}
inline bool sameContent(const juce::File &a, juce::InputStream &b)
{
    juce::FileInputStream in(a);
    if (!in.openedOk())
        return false;
    juce::MemoryBlock x, y;
    in.readIntoMemoryBlock(x);
    b.readIntoMemoryBlock(y);
    return x == y;
}
// Bring a picked file within reach: on the desktop the file itself; on iOS a copy inside
// Documents/<subfolder> (an identical earlier copy is reused, a different file with the same
// name gets a new name). Returns File{} if it cannot be read.
inline juce::File adopt(const juce::URL &url, const juce::String &subfolder)
{
    if (url.isEmpty())
        return {};
    const auto local = url.isLocalFile() ? url.getLocalFile() : juce::File{};
    if (!sandboxed())
        return local;
    if (local != juce::File{} && local.isAChildOf(documents()))
        return local; // already ours
    const juce::URL::InputStreamOptions options(juce::URL::ParameterHandling::inAddress);
    auto name = juce::File::createLegalFileName(juce::URL::removeEscapeChars(url.getFileName()));
    if (name.isEmpty())
        name = "imported";
    auto destination = folder(subfolder).getChildFile(name);
    if (destination.existsAsFile())
    {
        if (auto again = url.createInputStream(options); again && sameContent(destination, *again))
            return destination;
        destination = destination.getNonexistentSibling(false);
    }
    auto in = url.createInputStream(options);
    if (!in)
        return {};
    juce::TemporaryFile temporary(destination);
    {
        juce::FileOutputStream out(temporary.getFile());
        if (!out.openedOk() || out.writeFromInputStream(*in, -1) < 0)
            return {};
    }
    return temporary.overwriteTargetFileWithTemporary() ? destination : juce::File{};
}
inline juce::Array<juce::File> adoptAll(const juce::FileChooser &chooser, const juce::String &subfolder)
{
    juce::Array<juce::File> files;
    for (const auto &url : chooser.getURLResults())
        if (auto f = adopt(url, subfolder); f.existsAsFile())
            files.add(f);
    return files;
}
inline juce::File adoptFirst(const juce::FileChooser &chooser, const juce::String &subfolder)
{
    return chooser.getURLResults().isEmpty() ? juce::File{} : adopt(chooser.getURLResults()[0], subfolder);
}
// Ask for a name, then call back with a file in Documents/<subfolder> (iOS has no save dialog
// that suits long recordings or quick preset saves).
inline void askName(juce::Component *parent, const juce::String &title, const juce::String &suggestion,
                    const juce::String &subfolder, const juce::String &extension,
                    std::function<void(const juce::File &)> done)
{
    auto *window = new juce::AlertWindow(title, {}, juce::MessageBoxIconType::NoIcon, parent);
    window->addTextEditor("name", suggestion);
    window->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    window->enterModalState(true, juce::ModalCallbackFunction::create([window, subfolder, extension, done](int result) {
                                const auto text = juce::File::createLegalFileName(window->getTextEditorContents("name").trim());
                                if (result == 1 && text.isNotEmpty())
                                    done(folder(subfolder).getChildFile(text).withFileExtension(extension));
                            }),
                            true);
}
} // namespace lowline::platform
