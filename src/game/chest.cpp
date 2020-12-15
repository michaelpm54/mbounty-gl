#include "game/chest.hpp"

#include <fmt/format.h>

#include "data/bounty.hpp"
#include "data/spells.hpp"
#include "engine/dialog-stack.hpp"
#include "game/gen-variables.hpp"
#include "game/variables.hpp"

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

void chest_gold(Variables &v, GenVariables &gen, bty::DialogStack &ds)
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

    int roll = rand() % kGoldBase[v.continent];
    int gold = (kGoldExtra[v.continent] + (roll + 1)) * 100;
    int leadership = gen.artifacts_found[ArtiRingOfHeroism] ? gold / 25 : gold / 50;

    ds.show_dialog({
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
            .confirm = [&v, gold, leadership](int opt) {
                if (opt == 0) {
                    v.gold += gold;
                }
                else {
                    v.leadership += leadership;
                    v.permanent_leadership += leadership;
                }
            },
        },
    });
}

void chest_commission(Variables &v, bty::DialogStack &ds)
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

    int roll = rand() % kCommissionBase[v.continent];
    int commission = kCommissionExtra[v.continent] + roll + 1;
    if (commission > 999) {
        commission = 999;
    }
    v.commission += commission;

    ds.show_dialog({
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

void chest_spell_power(Variables &v, bty::DialogStack &ds)
{
    v.spell_power++;

    ds.show_dialog({
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

void chest_spell_capacity(Variables &v, GenVariables &gen, bty::DialogStack &ds)
{
    static constexpr int kSpellCapacityBase[] = {
        1,
        1,
        2,
        2,
    };

    int capacity = gen.artifacts_found[ArtiRingOfHeroism] ? kSpellCapacityBase[v.continent] * 2 : kSpellCapacityBase[v.continent];
    v.max_spells += capacity;

    ds.show_dialog({
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

void chest_spell(Variables &v, bty::DialogStack &ds)
{
    int amount = (rand() % (v.continent + 1)) + 1;
    int spell = rand() % 14;
    v.spells[spell] += amount;

    ds.show_dialog({
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

void chest_roll(Variables &v, GenVariables &gen, bty::DialogStack &ds)
{
    int roll = rand() % 100;

    if (roll < kChestChanceGold[v.continent]) {
        chest_gold(v, gen, ds);
    }
    else if (roll < kChestChanceCommission[v.continent]) {
        chest_commission(v, ds);
    }
    else if (roll < kChestChanceSpellPower[v.continent]) {
        chest_spell_power(v, ds);
    }
    else if (roll < kChestChanceSpellCapacity[v.continent]) {
        chest_spell_capacity(v, gen, ds);
    }
    else {
        chest_spell(v, ds);
    }
}
