#ifndef BTY_DATA_BOUNTY_HPP_
#define BTY_DATA_BOUNTY_HPP_

#include <fmt/format.h>

#include <array>
#include <glm/vec4.hpp>
#include <string>

#include "data/morale.hpp"

inline constexpr char *const kContinentNames[4] = {
    "Continentia",
    "Forestria",
    "Archipelia",
    "Saharia",
};

enum ArtifactId {
    ArtiArticlesOfNobility,
    ArtiShieldOfProtection,
    ArtiCrownOfCommand,
    ArtiAmuletOfAugmentation,
    ArtiRingOfHeroism,
    ArtiAnchorOfAdmirality,
    ArtiBookOfNecros,
    ArtiSwordOfProwess,
};

namespace bty {

inline constexpr void sort_army(std::array<int, 5> &army, std::array<int, 5> &counts)
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

inline std::string get_descriptor(int count)
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

inline constexpr int random(int max)
{
    return max == 0 ? 0 : rand() % max;
}

inline constexpr int random(int min, int max)
{
    return min + random(max);
}

inline std::string number_with_ks(int num)
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

enum UnitId {
    Peasants,
    Sprites,
    Militias,
    Wolves,
    Skeletons,
    Zombies,
    Gnomes,
    Orcs,
    Archers,
    Elves,
    Pikemen,
    Nomads,
    Dwarves,
    Ghosts,
    Knights,
    Ogres,
    Barbarians,
    Trolls,
    Cavalries,
    Druids,
    Archmages,
    Vampires,
    Giants,
    Demons,
    Dragons,
    UnitCount
};

int constexpr kMoraleGroupA = 0;
int constexpr kMoraleGroupB = 1;
int constexpr kMoraleGroupC = 2;
int constexpr kMoraleGroupD = 3;
int constexpr kMoraleGroupE = 4;

using MoraleGroup = int;

enum Ability {
    AbilityFly = 1 << 0,
    AbilityScythe = 1 << 1,
    AbilityLeech = 1 << 2,
    AbilityUndead = 1 << 3,
    AbilityAbsorb = 1 << 4,
    AbilityImmune = 1 << 5,
    AbilityRegen = 1 << 6,
    AbilityMagic = 1 << 7,
};

struct Unit {
    std::string name_singular;
    std::string name_plural;
    int hp;
    int skill_level;
    int initial_moves;
    int melee_damage_min;
    int melee_damage_max;
    int ranged_damage_min;
    int ranged_damage_max;
    int initial_ammo;
    int recruit_cost;
    int weekly_cost;
    MoraleGroup morale_group;
    uint16_t abilities;
};

inline const Unit kUnits[UnitId::UnitCount] =
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

#endif    // BTY_DATA_BOUNTY_HPP_
