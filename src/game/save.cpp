#include "game/save.hpp"

#include <spdlog/spdlog.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <filesystem>
#include <regex>

#include "engine/dialog-stack.hpp"

SaveManager::SaveManager(bty::DialogStack &ds, std::function<void(const std::string &)> save_cb, std::function<void(const std::string &)> load_cb)
    : ds(ds)
    , save_cb(save_cb)
    , load_cb(load_cb)
{
}

SaveManager::SavesFuture SaveManager::get_save_list() const
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

    std::regex slot_regex("^slot-[1-6]\\.sav$");

    for (const auto &file : dir) {
        if (std::regex_match(file.path().filename().generic_string(), slot_regex)) {
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
        char time_str[18];
        size_t result = strftime(time_str, 18, "%y-%m-%d %T", modified);

        if (result == 0) {
            spdlog::warn("strftime failed");
            continue;
        }

        save.name = entry.path().filename().generic_string();
        save.last_write = std::string(time_str);
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

void SaveManager::show_saves_dialog(bool load)
{
    ds.show_dialog({
        .x = 10,
        .y = 13,
        .w = 12,
        .h = 3,
        .strings = {
            {1, 1, "Loading..."},
        },
    });

    saves_future = std::async(&SaveManager::get_save_list, this);
    waiting_for_saves = true;
    loading = load;
}

void SaveManager::show()
{
    if (!ds.empty()) {
        ds.pop();
    }

    auto sf = saves_future.get();

    if (sf.failed) {
        ds.show_dialog({
            .x = 10,
            .y = 12,
            .w = 12,
            .h = 4,
            .strings = {
                {1, 1, "Failed to\nread saves"},
            },
        });
        return;
    }
    else if (sf.saves.empty()) {
        ds.show_dialog({
            .x = 9,
            .y = 12,
            .w = 13,
            .h = 3,
            .strings = {
                {1, 1, "Saves empty"},
            },
        });
        return;
    }

    std::vector<bty::DialogDef::StringDef> options;

    int i = 1;
    for (const auto save : sf.saves) {
        options.push_back({3, 1 + 3 * (save.slot - 1), fmt::format("{}\n\t{}", save.name, save.last_write)});
    }

    ds.show_dialog({
        .x = 4,
        .y = 5,
        .w = 24,
        .h = 20,
        .options = options,
        .callbacks = {
            .confirm = [this, sf](int save_index) {
                if (loading) {
                    if (!sf.saves[save_index].last_write.empty()) {
                        ds.show_dialog({
                            .x = 7,
                            .y = 12,
                            .w = 18,
                            .h = 6,
                            .strings = {
                                {2, 1, "Load this save?"},
                            },
                            .options = {
                                {3, 3, "Yes"},
                                {3, 4, "No"},
                            },
                            .callbacks = {
                                .confirm = [this, sf, save_index](int opt) {
                                    if (opt == 0) {
                                        load_cb(sf.saves[save_index].name);
                                        ds.pop();
                                    }
                                },
                            },
                        });
                    }
                }
                else {
                    if (!sf.saves[save_index].last_write.empty()) {
                        ds.show_dialog({
                            .x = 9,
                            .y = 12,
                            .w = 13,
                            .h = 6,
                            .strings = {
                                {2, 1, "Overwrite?"},
                            },
                            .options = {
                                {3, 3, "Yes"},
                                {3, 4, "No"},
                            },
                            .callbacks = {
                                .confirm = [this, sf, save_index](int opt) {
                                    if (opt == 0) {
                                        save_cb(sf.saves[save_index].name);
                                        ds.pop();
                                    }
                                },
                            },
                        });
                    }
                    else {
                        save_cb(fmt::format("slot-{}.sav", save_index + 1));
                        ds.pop();
                    }
                }
            },
        },
        .pop_on_confirm = false,
    });
}

void SaveManager::update()
{
    if (waiting_for_saves) {
        if (saves_future._Is_ready()) {
            waiting_for_saves = false;
            show();
        }
    }
}

bool SaveManager::waiting() const
{
    return waiting_for_saves;
}
