#ifndef BTY_GAME_SAVE_HPP_
#define BTY_GAME_SAVE_HPP_

#include <functional>
#include <future>

#include "engine/component.hpp"
#include "engine/dialog.hpp"

namespace bty {
class Engine;
}

class SaveManager : public Component {
private:
    struct SaveInfo {
        std::string name;
        std::string lastWriteTime;
        int slot {-1};
    };

    struct SavesFuture {
        std::vector<SaveInfo> saves;
        bool failed;
    };

public:
    SaveManager(bty::Engine &engine);

    void load() override;
    void enter() override;
    void renderLate() override;
    void update(float dt) override;
    bool isOverlay() const override;

    void onLoad(std::function<void(const std::string &)> cb);
    void onSave(std::function<void(const std::string &)> cb);

    void setMode(bool toLoad);

private:
    SavesFuture getSaves() const;
    void show();
    void loadState(SaveInfo save);
    void saveState(SaveInfo save);

private:
    bty::Engine &_engine;
    bty::Dialog _dlgLoading;
    std::future<SavesFuture> _savesFuture;
    bool _waitingForSaves {false};
    bool _modeIsLoading {false};
    std::function<void(const std::string &)> _loadCallback {nullptr};
    std::function<void(const std::string &)> _saveCallback {nullptr};
};

#endif    // BTY_GAME_SAVE_HPP_
