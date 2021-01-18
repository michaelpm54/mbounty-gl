#include "game/chest-spell-power.hpp"

#include <spdlog/spdlog.h>

#include "engine/engine.hpp"
#include "game/state.hpp"

static constexpr const char *const kMessage = {
    "  Traversing the area, you\n"
    "  stumble upon a timeworn\n"
    "   canister. Curious, you\n"
    "     unstop the bottle,\n"
    " releasing a powerful genie,\n"
    "   who raises your Spell\n"
    "  Power by 1 and vanishes.",
};

ChestSpellPower::ChestSpellPower(bty::Engine &engine)
    : _engine(engine)
{
}

void ChestSpellPower::load()
{
    create(1, 18, 30, 9);
    addString(1, 1, kMessage);
    bind(Key::Enter, [this](int opt) {
        _engine.getGUI().popDialog();
    });
}

void ChestSpellPower::show()
{
    setColor(bty::getBoxColor(State::difficulty));
    State::spell_power++;
    _engine.getGUI().pushDialog(*this);
}

int ChestSpellPower::getChance()
{
    static constexpr const int kChances[] = {0, 87, 88, 86};
    return kChances[State::continent];
}
