#include "game/chest-commission.hpp"

#include <spdlog/spdlog.h>

#include "engine/engine.hpp"
#include "game/state.hpp"

inline constexpr const int kCommissionBase[] = {
    41,
    51,
    101,
    45,
};

inline constexpr const int kCommissionExtra[] = {
    9,
    49,
    99,
    199,
};

static constexpr const char *const kMessage = {
    "After surveying the area,\n"
    "you discover that it is\n"
    "rich in mineral deposits.\n"
    "\n"
    "The King rewards you for\n"
    "your find by increasing\n"
    "your weekly income by {}.",
};

ChestCommission::ChestCommission(bty::Engine &engine)
    : _engine(engine)
{
}

void ChestCommission::load()
{
    create(1, 18, 30, 9);
    _btMessage = addString(3, 1);
    bind(Key::Enter, [this](int opt) {
        _engine.getGUI().popDialog();
    });
}

void ChestCommission::show()
{
    setColor(bty::getBoxColor(State::difficulty));
    int roll = rand() % kCommissionBase[State::continent];
    int commission = kCommissionExtra[State::continent] + roll + 1;
    if (commission > 999) {
        commission = 999;
    }
    State::commission += commission;
    _btMessage->setString(fmt::format(kMessage, commission));
    _engine.getGUI().pushDialog(*this);
}

int ChestCommission::getChance()
{
    static constexpr const int kChances[] = {81, 86, 86, 81};
    return kChances[State::continent];
}
