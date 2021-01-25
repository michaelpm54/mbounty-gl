#include "game/save.hpp"

#include <spdlog/spdlog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <filesystem>
#include <regex>

#include "engine/engine.hpp"
#include "game/state.hpp"

SaveManager::SaveManager(bty::Engine &engine)
    : _engine(engine)
{
}

void SaveManager::load()
{
    _dlgLoading.create(10, 13, 12, 3);
    _dlgLoading.addString(1, 1, "Loading...");
}

void SaveManager::enter()
{
    _dlgLoading.setColor(bty::getBoxColor(State::difficulty));
    _waitingForSaves = true;
    _savesFuture = std::async(&SaveManager::getSaves, this);
}

void SaveManager::renderLate()
{
    if (_waitingForSaves) {
        _dlgLoading.render();
    }
}

SaveManager::SavesFuture SaveManager::getSaves() const
{
    bool failed {false};

    if (!std::filesystem::exists("./saves")) {
        if (!std::filesystem::create_directories("./saves")) {
            failed = true;
        }
    }

    /* Put entries into a vector for sorting by slot index. */
    std::vector<std::filesystem::directory_entry> entries;

    auto dir = std::filesystem::directory_iterator("./saves");

    std::regex slotRegex("^slot-[1-6]\\.sav$");

    for (const auto &file : dir) {
        if (std::regex_match(file.path().filename().generic_string(), slotRegex)) {
            entries.push_back(file);
        }
    }

    std::sort(entries.begin(), entries.end());

    std::vector<SaveManager::SaveInfo> saves(6);

    for (const auto &entry : entries) {
        int slot = atoi(&entry.path().filename().generic_string()[5]);

        auto &save = saves[slot - 1];

        /* stat */
        struct stat info;
        if (stat(entry.path().generic_string().c_str(), &info)) {
            failed = true;
            spdlog::warn("stat: failed for '{}'", entry.path().generic_string());
            break;
        }

        /* Modified time */
        const auto modified = std::localtime(&info.st_mtime);
        char timeStr[18];
        size_t result = strftime(timeStr, 18, "%y-%m-%d %T", modified);

        if (result == 0) {
            spdlog::warn("strftime failed");
            continue;
        }

        save.name = entry.path().filename().generic_string();
        save.lastWriteTime = std::string(timeStr);
        save.slot = slot;
    }

    for (int i = 0; i < 6; i++) {
        if (saves[i].slot == -1) {
            saves[i].name = fmt::format("-- Empty Slot {} --", i + 1);
            saves[i].slot = i + 1;
        }
    }

    return {saves, failed};
}

void SaveManager::setMode(bool toLoad)
{
    _modeIsLoading = toLoad;
}

void SaveManager::show()
{
    auto sf = _savesFuture.get();

    if (sf.failed) {
        _engine.getGUI().showMessage(10, 12, 12, 4, "Failed to\nread saves");
        return;
    }
    else if (sf.saves.empty()) {
        _engine.getGUI().showMessage(9, 12, 13, 3, "Saves empty");
        return;
    }

    auto dialog = _engine.getGUI().makeDialog(4, 5, 24, 20, false);

    int i = 1;
    for (const auto save : sf.saves) {
        auto *opt = dialog->addOption(3, 1 + 3 * (save.slot - 1), fmt::format("{}\n\t{}", save.name, save.lastWriteTime));
        if (_modeIsLoading && save.lastWriteTime.empty()) {
            opt->disable();
        }
    }

    dialog->onKey([this, dialog, sf](Key key) {
        switch (key) {
            case Key::Backspace:
                _engine.getGUI().popDialog();
                SceneMan::instance().back();
                break;
            case Key::Enter:
                if (_modeIsLoading) {
                    loadState(sf.saves[dialog->getSelection()]);
                }
                else {
                    saveState(sf.saves[dialog->getSelection()]);
                }
                break;
            default:
                return false;
        }
        return true;
    });
}

void SaveManager::onLoad(std::function<void(const std::string &)> cb)
{
    _loadCallback = cb;
}

void SaveManager::onSave(std::function<void(const std::string &)> cb)
{
    _saveCallback = cb;
}

bool SaveManager::isOverlay() const
{
    return true;
}

void SaveManager::update(float dt)
{
    if (_waitingForSaves) {
        if (_savesFuture._Is_ready()) {
            _waitingForSaves = false;
            show();
        }
        return;
    }
}

void SaveManager::loadState(SaveInfo save)
{
    if (save.lastWriteTime.empty()) {
        spdlog::warn("Invalid save state");
        return;
    }

    auto dialog = _engine.getGUI().makeDialog(7, 12, 18, 6);
    dialog->addString(2, 1, "Load this save?");
    dialog->addOption(3, 3, "Yes");
    dialog->addOption(3, 4, "No");
    dialog->onKey([this, dialog, save](Key key) {
        switch (key) {
            case Key::Backspace:
                _engine.getGUI().popDialog();
                break;
            case Key::Enter:
                _engine.getGUI().popDialog();
                if (dialog->getSelection() == 0) {
                    _loadCallback(save.name);
                    _engine.getGUI().popDialog();
                    SceneMan::instance().back();
                }
                break;
            default:
                return false;
        }
        return true;
    });
}

void SaveManager::saveState(SaveInfo save)
{
    if (save.lastWriteTime.empty()) {
        _saveCallback(fmt::format("slot-{}.sav", save.slot));
    }
    else {
        auto dialog = _engine.getGUI().makeDialog(9, 12, 13, 6);
        dialog->addString(2, 1, "Overwrite?");
        dialog->addOption(3, 3, "Yes");
        dialog->addOption(3, 4, "No");
        dialog->onKey([this, dialog, save](Key key) {
            switch (key) {
                case Key::Backspace:
                    _engine.getGUI().popDialog();
                    break;
                case Key::Enter:
                    _engine.getGUI().popDialog();
                    if (dialog->getSelection() == 0) {
                        _saveCallback(save.name);
                        _engine.getGUI().popDialog();
                        SceneMan::instance().setScene("ingame");
                    }
                    break;
                default:
                    return false;
            }
            return true;
        });
    }
}
