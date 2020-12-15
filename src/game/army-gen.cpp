#include "game/army-gen.hpp"

#include "data/bounty.hpp"

static constexpr int kVillainArmies[5][17] = {
    {
        Peasants,
        Nomads,
        Sprites,
        Wolves,
        Militias,
        Sprites,
        Militias,
        Elves,
        Orcs,
        Skeletons,
        Skeletons,
        Giants,
        Gnomes,
        Archers,
        Demons,
        Demons,
        Dragons,
    },
    {
        Wolves,
        Militias,
        Sprites,
        Orcs,
        Militias,
        Ghosts,
        Archers,
        Archmages,
        Ogres,
        Zombies,
        Zombies,
        Dragons,
        Barbarians,
        Pikemen,
        Vampires,
        Dragons,
        Dragons,
    },
    {
        Militias,
        Militias,
        Skeletons,
        Archers,
        Archers,
        Knights,
        Pikemen,
        Druids,
        Trolls,
        Ghosts,
        Ghosts,
        Ogres,
        Giants,
        Cavalries,
        Archmages,
        Cavalries,
        Dragons,
    },
    {
        Peasants,
        Peasants,
        Zombies,
        Trolls,
        Elves,
        Archmages,
        Cavalries,
        Pikemen,
        Giants,
        Vampires,
        Vampires,
        Orcs,
        Nomads,
        Knights,
        Gnomes,
        Knights,
        Demons,
    },
    {
        Peasants,
        Peasants,
        Ogres,
        Dwarves,
        Barbarians,
        Archmages,
        Knights,
        Sprites,
        Wolves,
        Demons,
        Demons,
        Gnomes,
        Peasants,
        Dragons,
        Peasants,
        Archmages,
        Vampires,
    },
};

static constexpr int kVillainArmyCounts[5][17] = {
    {
        50,
        10,
        70,
        30,
        50,
        250,
        100,
        30,
        150,
        500,
        600,
        30,
        300,
        35,
        30,
        50,
        100,
    },
    {
        20,
        30,
        50,
        20,
        50,
        10,
        20,
        30,
        20,
        100,
        200,
        5,
        40,
        100,
        50,
        10,
        25,
    },
    {
        25,
        20,
        20,
        10,
        10,
        10,
        20,
        10,
        10,
        30,
        50,
        30,
        20,
        80,
        100,
        200,
        25,
    },
    {
        30,
        60,
        20,
        2,
        10,
        4,
        16,
        30,
        5,
        10,
        25,
        200,
        100,
        60,
        500,
        250,
        100,
    },
    {
        25,
        40,
        4,
        6,
        5,
        4,
        16,
        300,
        80,
        6,
        10,
        200,
        700,
        5,
        5000,
        60,
        100,
    },
};

void gen_villain_army(int villain, std::array<int, 5> &army, std::array<int, 5> &counts)
{
    for (int i = 0; i < 5; i++) {
        army[i] = kVillainArmies[i][villain];
        int count = kVillainArmyCounts[i][villain];
        count += bty::random(kVillainArmyCounts[i][villain] / 16);
        count += bty::random(kVillainArmyCounts[i][villain] / 32);
        count += bty::random(4);
        counts[i] = count;
    }
}

static constexpr int kMobRollChances[16] = {
    60,
    20,
    10,
    3,
    90,
    70,
    20,
    6,
    100,
    95,
    50,
    10,
    101,
    100,
    90,
    40,
};

static constexpr int kMobIdRange[16] = {
    Peasants,
    Sprites,
    Orcs,
    Skeletons,
    Wolves,
    Gnomes,
    Dwarves,
    Zombies,
    Nomads,
    Elves,
    Ogres,
    Ghosts,
    Barbarians,
    Trolls,
    Giants,
    Vampires,
};

static constexpr int kMobIdRndHigh[4] = {
    Archmages,
    Druids,
    Dragons,
    Demons,
};

static constexpr int kMaxMobCounts[4][UnitId::UnitCount] = {
    {
        10,
        20,
        10,
        5,
        5,
        5,
        10,
        5,
        0,
        5,
        0,
        4,
        4,
        2,
        0,
        2,
        2,
        2,
        0,
        1,
        1,
        2,
        1,
        1,
        1,
    },
    {
        20,
        50,
        20,
        15,
        10,
        10,
        25,
        15,
        0,
        10,
        0,
        8,
        10,
        4,
        0,
        4,
        4,
        4,
        0,
        3,
        2,
        4,
        2,
        2,
        1,
    },
    {
        50,
        100,
        50,
        30,
        25,
        25,
        50,
        30,
        0,
        25,
        0,
        15,
        20,
        10,
        0,
        8,
        10,
        8,
        0,
        6,
        4,
        10,
        5,
        4,
        1,
    },
    {
        100,
        127,
        100,
        80,
        50,
        75,
        100,
        80,
        0,
        50,
        0,
        30,
        50,
        20,
        0,
        15,
        20,
        15,
        0,
        10,
        8,
        25,
        10,
        8,
        2,
    },
};

int gen_mob_count(int continent, int unit)
{
    int bVar1 = kMaxMobCounts[continent][unit];
    int cVar3 = bty::random(bVar1 / 8);
    int uVar2 = bty::random(2);
    return (uVar2 & 0xffffff00) | (bVar1 + uVar2 + cVar3);
}

int gen_mob_unit(int continent)
{
    int id = rand() % 11;
    int chance = rand() % 100;
    int tries = 0;
    while (true) {
        if (3 < tries) {
            return kMobIdRndHigh[id >> 2];
        }
        if ((chance + 1 & 0xff) < kMobRollChances[continent + tries * 4])
            break;
        tries++;
    }
    return ((tries * 4) & 0xffffff00) | kMobIdRange[((id >> 2) & 0xff) + tries * 4];
}

void gen_mob_army(int continent, std::array<int, 5> &army, std::array<int, 5> &counts)
{
    for (int i = 0; i < 5; i++) {
        army[i] = -1;
        counts[i] = 0;
    regen:
        int id = gen_mob_unit(continent);
        for (int j = 0; j < 5; j++) {
            if (army[j] == id) {
                goto regen;
            }
        }
        army[i] = id;
        counts[i] = gen_mob_count(continent, army[i]);
    }

    /* Cut 1-2 of them in half, or chop the last two off. */
    int roll = bty::random(3);
    if (roll == 0) {
        int n = bty::random(4);
        counts[n] /= 2;
        if (counts[n] == 0) {
            counts[n] = std::max(1, bty::random(2));
        }

        /* Cut 1 of them in half, or chop the last one off. */
        /* Note: modified by me to give a higher chance of chopping one off. */
        /* I seemed to be getting 5 unit armies too often. */
        /* was: rand % 3 */
        int a = bty::random(4);
        if (a == 0) {
            n = bty::random(5);
            counts[n] /= 2;
            if (counts[n] == 0) {
                counts[n] = std::max(1, bty::random(2));
            }
        }
        /* Chop the last one off. */
        else {
            army[4] = -1;
            counts[4] = 0;
        }
    }
    /* Chop the last two off. */
    else {
        army[3] = -1;
        army[4] = -1;
        counts[3] = 0;
        counts[4] = 0;
    }
}

void gen_castle_army(int continent, std::array<int, 5> &army, std::array<int, 5> &counts)
{
    for (int i = 0; i < 5; i++) {
        army[i] = -1;
        counts[i] = 0;
    regen:
        int id = gen_mob_unit(continent);
        for (int j = 0; j < 5; j++) {
            if (army[j] == id) {
                goto regen;
            }
        }
        army[i] = id;
        counts[i] = gen_mob_count(continent, army[i]);
    }
}
