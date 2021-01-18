#include "game/chest-gold.hpp"

#include <spdlog/spdlog.h>

#include "engine/engine.hpp"
#include "game/state.hpp"

inline constexpr const int kGoldBase[] = {
    5,
    16,
    21,
    31,
};

inline constexpr const int kGoldExtra[] = {
    0,
    4,
    9,
    19,
};

static constexpr const char *const kMessage = {
    "After scouring the area\n"
    "you fall upon a hidden\n"
    "treasure cache. You may:",
};

inline constexpr const char *const kOptions[] = {
    {
        "Take the {} gold.",
    },
    {
        "Distribute the gold to the\n"
        " peasants, increasing your\n"
        " leadership by {}.",
    },
};

ChestGold::ChestGold(bty::Engine &engine)
    : _engine(engine)
{
}

void ChestGold::load()
{
    create(1, 18, 30, 9);
    addString(1, 1, kMessage);
    for (int i = 0; i < sizeof(kOptions) / sizeof(kOptions[0]); i++) {
        _btOptions.push_back(addOption(3, 4 + i));
    }
    bind(Key::Enter, [this](int opt) {
        if (opt == 0) {
            State::gold += _gold;
            _engine.getGUI().getHUD().setGold(State::gold);
        }
        else {
            State::leadership += _leadership;
            State::permanent_leadership += _leadership;
        }
        _engine.getGUI().popDialog();
    });
}

void ChestGold::show()
{
    setColor(bty::getBoxColor(State::difficulty));
    int roll = bty::random(kGoldBase[State::continent]);
    _gold = (kGoldExtra[State::continent] + (roll + 1)) * 100;
    _leadership = State::artifacts_found[ArtiRingOfHeroism] ? _gold / 25 : _gold / 50;
    _btOptions[0]->setString(fmt::format(kOptions[0], _gold));
    _btOptions[1]->setString(fmt::format(kOptions[1], _leadership));
    _engine.getGUI().pushDialog(*this);
}

int ChestGold::getChance()
{
    static constexpr const int kChances[] = {61, 66, 76, 71};
    return kChances[State::continent];
}
