#include "game/chest-spell.hpp"

#include <spdlog/spdlog.h>

#include "data/spells.hpp"
#include "engine/engine.hpp"
#include "game/state.hpp"

static constexpr const char *const kMessage = {
    "    You have captured a\n"
    " mischievous imp which has\n"
    "    been terrorizing the\n"
    "  region. In exchange for\n"
    "  its release, you receive:\n"
    "\n"
    "     {} {} spell{}.",
};

ChestSpell::ChestSpell(bty::Engine &engine)
    : _engine(engine)
{
}

void ChestSpell::load()
{
    create(1, 18, 30, 9);
    _btMessage = addString(1, 1);
    bind(Key::Enter, [this](int opt) {
        _engine.getGUI().popDialog();
    });
}

void ChestSpell::show()
{
    setColor(bty::getBoxColor(State::difficulty));
    int amount = bty::random(1, State::continent + 1);
    int spell = bty::random(14);
    State::spells[spell] += amount;
    _btMessage->setString(fmt::format(kMessage, amount, kSpellNames[spell], amount == 1 ? "" : "s"));
    _engine.getGUI().pushDialog(*this);
}

int ChestSpell::getChance()
{
    static constexpr const int kChances[] = {101, 101, 101, 101};
    return kChances[State::continent];
}
