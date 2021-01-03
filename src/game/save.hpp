#ifndef BTY_GAME_SAVE_HPP_
#define BTY_GAME_SAVE_HPP_

#include <functional>
#include <future>

namespace bty {
class DialogStack;
}

class SaveManager {
private:
    struct SaveInfo {
        std::string name;
        std::string last_write;
        int slot {-1};
    };

    struct SavesFuture {
        std::vector<SaveInfo> saves;
        bool failed;
    };

public:
    SaveManager(bty::DialogStack &ds, std::function<void(const std::string &)> save_cb, std::function<void(const std::string &)> load_cb);

    void show_saves_dialog(bool load);
    void update();
    bool waiting() const;

private:
    SavesFuture get_save_list() const;
    void show();
    void choose_name();

private:
    bty::DialogStack &ds;
    std::future<SavesFuture> saves_future;
    bool waiting_for_saves {false};
    bool loading {false};
    std::string chosen_name {""};
    std::function<void(std::string)> load_cb;
    std::function<void(std::string)> save_cb;
};

#endif    // BTY_GAME_SAVE_HPP_
