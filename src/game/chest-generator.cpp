#include "game/chest-generator.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "game/chest.hpp"
#include "game/state.hpp"

void ChestGenerator::addChest(Chest &chest, bool fallback)
{
    chest.load();
    _chests.push_back(&chest);
    if (fallback) {
        _fallback = &chest;
    }
}

void ChestGenerator::roll()
{
    int roll = bty::random(100);

    auto it = _chests.begin();

    for (it; it != _chests.end(); ++it) {
        if ((*it) != _fallback && roll < (*it)->getChance()) {
            break;
        }
    }

    if (it == _chests.end()) {
        assert(_fallback);
        _fallback->show();
    }
    else {
        (*it)->show();
    }
}
