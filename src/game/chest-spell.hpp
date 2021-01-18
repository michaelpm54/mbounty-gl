#ifndef BTY_GAME_CHEST_SPELL_HPP_
#define BTY_GAME_CHEST_SPELL_HPP_

#include "engine/dialog.hpp"
#include "game/chest.hpp"

namespace bty {
class Engine;
}

class ChestSpell : public bty::Dialog, public Chest {
public:
    ChestSpell(bty::Engine &engine);
    void load() override;
    void show() override;
    int getChance() override;

private:
    bty::Engine &_engine;
    bty::Text *_btMessage;
};

#endif    // BTY_GAME_CHEST_SPELL_HPP_
