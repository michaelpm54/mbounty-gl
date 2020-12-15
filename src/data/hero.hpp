#ifndef BTY_DATA_HERO_HPP_
#define BTY_DATA_HERO_HPP_

inline constexpr int const kStartingGold[4] = {
    7'500,
    10'000,
    10'000,
    7'500,
};

inline constexpr char *const kHeroRankNames[4][4] = {
    {
        "Knight",
        "General",
        "Marshal",
        "Lord",
    },
    {
        "Paladin",
        "Crusader",
        "Avenger",
        "Champion",
    },
    {
        "Sorceress",
        "Magician",
        "Mage",
        "Archmage",
    },
    {
        "Barbarian",
        "Chieftain",
        "Warlord",
        "Overlord",
    },
};

inline constexpr char *const kShortHeroNames[4] = {
    "Sir Crimsaun",
    "Lord Palmer",
    "Tynnestra",
    "Moham",
};

inline constexpr char *const kHeroNames[][4] =
    {
        {
            "Sir Crimsaun the Knight",
            "Sir Crimsaun the General",
            "Sir Crimsaun the Marshal",
            "Sir Crimsaun the Lord",
        },
        {
            "Lord Palmer the Paladin",
            "Lord Palmer the Crusader",
            "Lord Palmer the Avenger",
            "Lord Palmer the Champion",
        },
        {
            "Tynnestra the Sorceress",
            "Tynnestra the Magician",
            "Tynnestra the Mage",
            "Tynnestra the Archmage",
        },
        {
            "Mad Moham the Barbarian",
            "Mad Moham the Chieftain",
            "Mad Moham the Warlord",
            "Mad Moham the Overlord",
        },
};

inline constexpr int const kRankCommission[4][4] = {
    {
        1000,
        2000,
        4000,
        8000,
    },
    {
        1000,
        2000,
        4000,
        8000,
    },
    {
        3000,
        4000,
        5000,
        6000,
    },
    {
        2000,
        4000,
        6000,
        8000,
    },
};

inline constexpr int const kRankSpells[4][4] = {
    {
        2,
        5,
        9,
        14,
    },
    {
        3,
        7,
        12,
        18,
    },
    {
        5,
        13,
        23,
        35,
    },
    {
        2,
        4,
        7,
        10,
    },
};

inline constexpr int const kRankSpellPower[4][4] = {
    {
        1,
        2,
        3,
        5,
    },
    {
        1,
        3,
        5,
        7,
    },
    {
        2,
        5,
        10,
        15,
    },
    {
        1,
        2,
        3,
        4,
    },
};

inline constexpr int const kRankLeadership[4][4] = {
    {
        100,
        200,
        500,
        1000,
    },
    {
        80,
        160,
        400,
        900,
    },
    {
        80,
        160,
        400,
        900,
    },
    {
        100,
        200,
        500,
        1000,
    },
};

inline constexpr int const kRankVillainsCaptured[4][4] = {
    {
        0,
        2,
        8,
        14,
    },
    {
        0,
        2,
        7,
        13,
    },
    {
        0,
        3,
        6,
        12,
    },
    {
        0,
        1,
        5,
        10,
    },
};

#endif    // BTY_DATA_HERO_HPP_
