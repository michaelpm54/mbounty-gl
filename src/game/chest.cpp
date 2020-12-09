#include "game/chest.hpp"

#include <fmt/format.h>

#include "bounty.hpp"
#include "shared-state.hpp"

static constexpr int kChestChanceGold[] = {
    61,
    66,
    76,
    71,
};

static constexpr int kChestChanceCommission[] = {
    81,
    86,
    86,
    81,
};

static constexpr int kChestChanceSpellPower[] = {
    0,
    87,
    88,
    86,
};

static constexpr int kChestChanceSpellCapacity[] = {
    86,
    92,
    93,
    91,
};

static constexpr int kChestChanceAddSpell[] = {
    101,
    101,
    101,
    101,
};

void chest_gold(SharedState &state, std::function<void(const DialogDef &)> &show_dialog)
{
    static constexpr int kGoldBase[] = {
        5,
        16,
        21,
        31,
    };

    static constexpr int kGoldExtra[] = {
        0,
        4,
        9,
        19,
    };

    int roll = rand() % kGoldBase[state.continent];
    int gold = (kGoldExtra[state.continent] + (roll + 1)) * 100;
    int leadership = state.artifacts_found[ArtiRingOfHeroism] ? gold / 25 : gold / 50;

    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, "After scouring the area"},
            {1, 2, "you fall upon a hidden"},
            {1, 3, "treasure cache. You may:"},
        },
        .options = {
            {3, 4, fmt::format("Take the {} gold.", gold)},
            {3, 5, fmt::format("Distribute the gold to the\n peasants, increasing your\n leadership by {}.", leadership)},
        },
        .callbacks = {
            .confirm = [&state, gold, leadership](int opt) {
                if (opt == 0) {
                    state.gold += gold;
                }
                else {
                    state.leadership += leadership;
                    state.permanent_leadership += leadership;
                }
            },
        },
    });
}

void chest_commission(SharedState &state, std::function<void(const DialogDef &)> &show_dialog)
{
    static constexpr int kCommissionBase[] = {
        41,
        51,
        101,
        45,
    };

    static constexpr int kCommissionExtra[] = {
        9,
        49,
        99,
        199,
    };

    int roll = rand() % kCommissionBase[state.continent];
    int commission = kCommissionExtra[state.continent] + roll + 1;
    if (commission > 999) {
        commission = 999;
    }
    state.commission += commission;

    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {3, 1, "After surveying the area,"},
            {3, 2, "you discover that it is"},
            {3, 3, "rich in mineral deposits."},
            {3, 5, "The King rewards you for"},
            {3, 6, "your find by increasing"},
            {3, 7, fmt::format("your weekly income by {}.", commission)},
        },
    });
}

void chest_spell_power(SharedState &state, std::function<void(const DialogDef &)> &show_dialog)
{
    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {3, 1, "Traversing the area, you"},
            {3, 2, "stumble upon a timeworn"},
            {4, 3, "canister. Curious, you"},
            {6, 4, "unstop the bottle,"},
            {2, 5, "releasing a powerful genie,"},
            {4, 6, "who raises your Spell"},
            {3, 7, "Power by 1 and vanishes."},
        },
    });
}

void chest_spell_capacity(SharedState &state, std::function<void(const DialogDef &)> &show_dialog)
{
    static constexpr int kSpellCapacityBase[] = {
        1,
        1,
        2,
        2,
    };

    int capacity = state.artifacts_found[ArtiRingOfHeroism] ? kSpellCapacityBase[state.continent] * 2 : kSpellCapacityBase[state.continent];
    state.max_spells += capacity;

    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, "A tribe of nomads greet you"},
            {1, 2, "and your army warmly. Their"},
            {4, 3, "shaman, in awe of your"},
            {3, 4, "prowess, teaches you the"},
            {1, 5, "secret of his tribe's magic."},
            {1, 6, "Your maximum spell capacity"},
            {6, 7, fmt::format("is increased by {}.", capacity)},
        },
    });
}

void chest_spell(SharedState &state, std::function<void(const DialogDef &)> &show_dialog)
{
    int amount = (rand() % (state.continent + 1)) + 1;
    int spell = rand() % 14;
    state.spells[spell] += amount;

    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {5, 1, "You have captured a"},
            {2, 2, "mischievous imp which has"},
            {5, 3, "been terrorizing the"},
            {3, 4, "region. In exchange for"},
            {3, 5, "its release, you receive:"},
            {6, 7, fmt::format("{} {} spell{}.", amount, kSpellNames[spell], amount == 1 ? "" : "s")},
        },
    });
}

void chest_roll(SharedState &state, std::function<void(const DialogDef &)> show_dialog)
{
    int roll = rand() % 100;

    if (roll < kChestChanceGold[state.continent]) {
        chest_gold(state, show_dialog);
    }
    else if (roll < kChestChanceCommission[state.continent]) {
        chest_commission(state, show_dialog);
    }
    else if (roll < kChestChanceSpellPower[state.continent]) {
        chest_spell_power(state, show_dialog);
    }
    else if (roll < kChestChanceSpellCapacity[state.continent]) {
        chest_spell_capacity(state, show_dialog);
    }
    else {
        chest_spell(state, show_dialog);
    }
}
