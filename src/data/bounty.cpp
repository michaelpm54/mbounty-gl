#include "bounty.hpp"

#include <fmt/format.h>

char const *const kContinents[4] = {
    "Continentia",
    "Forestria",
    "Archipelia",
    "Saharia",
};

namespace bty {

void sort_army(std::array<int, 5> &army, std::array<int, 5> &counts)
{
    int last_free = -1;
    for (int i = 0; i < 5; i++) {
        if (last_free == -1 && army[i] == -1) {
            last_free = i;
        }
        else if (last_free != -1 && army[i] != -1) {
            army[last_free] = army[i];
            army[i] = -1;
            counts[last_free] = counts[i];
            counts[i] = -1;
            last_free = i;
        }
    }
}

/* clang-format off */
static constexpr int kMoraleGroups[25] = {
	0, 2, 0, 3, 4,
	4, 2, 3, 1, 2,
	1, 2, 2, 4, 1,
	3, 2, 3, 1, 2,
	2, 4, 2, 4, 3,
};

/* 0 = normal
   1 = high
   2 = low
*/
static constexpr int kMoraleChart[25] = {
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 0, 0,
	2, 0, 2, 1, 0,
	2, 2, 2, 0, 0,
};
/* clang-format on */

int check_morale(int me, int *army)
{
    bool normal = false;
    for (int i = 0; i < 5; i++) {
        if (army[i] == -1) {
            continue;
        }
        int result = kMoraleChart[kMoraleGroups[army[me]] + kMoraleGroups[army[i]] * 5];
        if (result == 2) {
            return 1;
        }
        if (result == 0) {
            normal = true;
        }
    }
    if (normal) {
        return 0;
    }
    return 2;
}

std::string get_descriptor(int count)
{
    static constexpr const char *const kDescriptors[] = {
        "A few",
        "Some",
        "Many",
        "A lot of",
        "A horde of",
        "A multitude of",
    };

    static constexpr int kThresholds[] = {
        10,
        20,
        50,
        100,
        500,
    };

    int descriptor = 0;
    while (count >= kThresholds[descriptor] && descriptor < 5) {
        descriptor++;
    }

    return kDescriptors[descriptor];
}

int random(int max)
{
    return max == 0 ? 0 : rand() % max;
}

int random(int min, int max)
{
    return min + random(max);
}

std::string number_with_ks(int num)
{
    if (num < 10'000) {
        return std::to_string(num);
    }

    if (num >= 1'000'000) {
        return fmt::format("{}KK", num / 1'000'000);
    }
    else if (num >= 1'000) {
        return fmt::format("{}K", num / 1'000);
    }

    return "???";
}

}    // namespace bty

int const kDays[4] =
    {
        900,
        600,
        400,
        200,
};

const Unit kUnits[UnitId::UnitCount] =
    {
        {
            "Peasant",
            "Peasants",
            1,                // hp
            1,                // skill_level
            1,                // moves
            1,                // dmg_min
            1,                // dmg_max
            0,                // shot_dmg_min
            0,                // shot_dmg_max
            0,                // num_shots
            10,               // recruit_cost
            1,                // weekly_cost
            kMoraleGroupA,    // morale_group
            0,
        },
        {
            "Sprite",
            "Sprites",
            1,
            1,
            1,
            1,
            2,
            0,
            0,
            0,
            15,
            1,
            kMoraleGroupC,
            AbilityFly,
        },
        {
            "Militia",
            "Militias",
            2,
            2,
            2,
            1,
            2,
            0,
            0,
            0,
            50,
            5,
            kMoraleGroupA,
            0,
        },
        {
            "Wolf",
            "Wolves",
            3,
            2,
            3,
            1,
            3,
            0,
            0,
            0,
            40,
            4,
            kMoraleGroupD,
            0,
        },
        {
            "Skeleton",
            "Skeletons",
            3,
            2,
            2,
            1,
            2,
            0,
            0,
            0,
            50,
            4,
            kMoraleGroupE,
            AbilityUndead,
        },
        {
            "Zombie",
            "Zombies",
            5,
            2,
            1,
            2,
            2,
            0,
            0,
            0,
            50,
            5,
            kMoraleGroupE,
            AbilityUndead,
        },
        {
            "Gnome",
            "Gnomes",
            5,
            2,
            1,
            1,
            3,
            0,
            0,
            0,
            60,
            6,
            kMoraleGroupC,
            0,
        },
        {
            "Orc",
            "Orcs",
            5,
            2,
            2,
            2,
            3,
            1,
            2,
            6,
            75,
            7,
            kMoraleGroupC,
            0,
        },
        {
            "Archer",
            "Archers",
            10,
            2,
            2,
            1,
            2,
            1,
            3,
            12,
            250,
            25,
            kMoraleGroupB,
            0,
        },
        {
            "Elf",
            "Elves",
            10,
            3,
            3,
            1,
            2,
            2,
            4,
            2,
            200,
            20,
            kMoraleGroupC,
            0,
        },
        {
            "Pikeman",
            "Pikemen",
            10,
            3,
            2,
            2,
            4,
            0,
            0,
            0,
            300,
            30,
            kMoraleGroupB,
            0,
        },
        {
            "Nomad",
            "Nomads",
            15,
            3,
            2,
            2,
            4,
            0,
            0,
            0,
            300,
            30,
            kMoraleGroupC,
            0,
        },
        {
            "Dwarf",
            "Dwarves",
            20,
            3,
            1,
            2,
            4,
            0,
            0,
            0,
            350,
            30,
            kMoraleGroupC,
            0,
        },
        {
            "Ghost",
            "Ghosts",
            10,
            4,
            3,
            3,
            4,
            0,
            0,
            0,
            400,
            40,
            kMoraleGroupE,
            AbilityAbsorb | AbilityUndead,
        },
        {
            "Knight",
            "Knights",
            35,
            5,
            1,
            6,
            10,
            0,
            0,
            0,
            1000,
            100,
            kMoraleGroupB,
            0,
        },
        {
            "Ogre",
            "Ogres",
            40,
            4,
            1,
            3,
            5,
            0,
            0,
            0,
            750,
            75,
            kMoraleGroupD,
            0,
        },
        {
            "Barbarian",
            "Barbarians",
            40,
            4,
            3,
            1,
            6,
            0,
            0,
            0,
            750,
            75,
            kMoraleGroupC,
            0,
        },
        {
            "Troll",
            "Trolls",
            50,
            4,
            1,
            2,
            5,
            0,
            0,
            0,
            1000,
            100,
            kMoraleGroupD,
            AbilityRegen,
        },
        {
            "Cavalry",
            "Cavalries",
            20,
            4,
            4,
            3,
            5,
            0,
            0,
            0,
            800,
            80,
            kMoraleGroupB,
            0,
        },
        {
            "Druid",
            "Druids",
            25,
            5,
            2,
            2,
            3,
            2,
            3,
            2,
            700,
            70,
            kMoraleGroupC,
            AbilityMagic,
        },
        {
            "Archmage",
            "Archmages",
            25,
            5,
            1,
            2,
            3,
            25,
            25,
            2,
            1200,
            120,
            kMoraleGroupC,
            AbilityFly | AbilityMagic,
        },
        {
            "Vampire",
            "Vampires",
            30,
            5,
            1,
            3,
            6,
            0,
            0,
            0,
            1500,
            150,
            kMoraleGroupE,
            AbilityLeech | AbilityFly | AbilityUndead,
        },
        {
            "Giant",
            "Giants",
            60,
            5,
            3,
            10,
            20,
            5,
            10,
            2,
            2000,
            200,
            kMoraleGroupC,
            0,
        },
        {
            "Demon",
            "Demons",
            200,
            6,
            1,
            5,
            7,
            0,
            0,
            0,
            3000,
            300,
            kMoraleGroupE,
            AbilityFly | AbilityScythe,
        },
        {
            "Dragon",
            "Dragons",
            200,
            6,
            1,
            25,
            50,
            0,
            0,
            0,
            5000,
            500,
            kMoraleGroupD,
            AbilityFly | AbilityImmune,
        },
};
