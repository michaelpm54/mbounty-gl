#include "game/chest-spell-capacity.hpp"

#include <spdlog/spdlog.h>

#include "engine/engine.hpp"
#include "game/state.hpp"

inline constexpr const int kSpellCapacityBase[] = {
    1,
    1,
    2,
    2,
};

static constexpr const char *const kMessage = {
    "A tribe of nomads greet you\n"
    "and your army warmly. Their\n"
    "   shaman, in awe of your\n"
    "  prowess, teaches you the\n"
    "secret of his tribe's magic.\n"
    "Your maximum spell capacity\n"
    "     is increased by {}.",
};

ChestSpellCapacity::ChestSpellCapacity(bty::Engine &engine)
    : _engine(engine)
{
}

void ChestSpellCapacity::load()
{
    create(1, 18, 30, 9);
    _btMessage = addString(1, 1);
    bind(Key::Enter, [this](int opt) {
        _engine.getGUI().popDialog();
    });
}

void ChestSpellCapacity::show()
{
    setColor(bty::getBoxColor(State::difficulty));
    int capacity = State::artifacts_found[ArtiRingOfHeroism] ? kSpellCapacityBase[State::continent] * 2 : kSpellCapacityBase[State::continent];
    State::max_spells += capacity;
    _btMessage->setString(fmt::format(kMessage, capacity));
    _engine.getGUI().pushDialog(*this);
}

int ChestSpellCapacity::getChance()
{
    static constexpr const int kChances[] = {86, 92, 93, 91};
    return kChances[State::continent];
}
