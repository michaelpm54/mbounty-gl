#ifndef BTY_GAME_CHEST_SPELLPOWER_HPP_
#define BTY_GAME_CHEST_SPELLPOWER_HPP_

#include "engine/dialog.hpp"
#include "game/chest.hpp"

namespace bty {
class Engine;
}

class ChestSpellPower : public bty::Dialog, public Chest {
public:
    ChestSpellPower(bty::Engine &engine);
    void load() override;
    void show() override;
    int getChance() override;

private:
    bty::Engine &_engine;
};

#endif    // BTY_GAME_CHEST_SPELLPOWER_HPP_
