#ifndef BTY_GAME_CHEST_COMMISSION_HPP_
#define BTY_GAME_CHEST_COMMISSION_HPP_

#include "engine/dialog.hpp"
#include "game/chest.hpp"

namespace bty {
class Engine;
}

class ChestCommission : public bty::Dialog, public Chest {
public:
    ChestCommission(bty::Engine &engine);
    void load() override;
    void show() override;
    int getChance() override;

private:
    bty::Engine &_engine;
    int _commission {0};
    bty::Text *_btMessage;
};

#endif    // BTY_GAME_CHEST_COMMISSION_HPP_
