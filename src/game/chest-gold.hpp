#ifndef BTY_GAME_CHEST_GOLD_HPP_
#define BTY_GAME_CHEST_GOLD_HPP_

#include "engine/dialog.hpp"
#include "game/chest.hpp"

namespace bty {
class Engine;
}

class ChestGold : public bty::Dialog, public Chest {
public:
    ChestGold(bty::Engine &engine);
    void load() override;
    void show() override;
    int getChance() override;

private:
    bty::Engine &_engine;
    int _gold {0};
    int _leadership {0};
    std::vector<bty::Text *> _btOptions;
};

#endif    // BTY_GAME_CHEST_GOLD_HPP_
