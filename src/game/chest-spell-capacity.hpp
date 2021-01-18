#ifndef BTY_GAME_CHEST_SPELLCAPACITY_HPP_
#define BTY_GAME_CHEST_SPELLCAPACITY_HPP_

#include "engine/dialog.hpp"
#include "game/chest.hpp"

namespace bty {
class Engine;
}

class ChestSpellCapacity : public bty::Dialog, public Chest {
public:
    ChestSpellCapacity(bty::Engine &engine);
    void load() override;
    void show() override;
    int getChance() override;

private:
    bty::Engine &_engine;
    bty::Text *_btMessage;
};

#endif    // BTY_GAME_CHEST_SPELLCAPACITY_HPP_
