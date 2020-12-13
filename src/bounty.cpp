#include "bounty.hpp"

#include <fmt/format.h>

const int kVillainIndices[4] = {
    0,
    6,
    10,
    14,
};

const int kVillainsPerContinent[4] = {
    6,
    4,
    4,
    3,
};

const char *const kSpellNames[14] = {
    "Clone",
    "Teleport",
    "Fireball",
    "Lightning",
    "Freeze",
    "Resurrect",
    "Turn Undead",
    "Bridge",
    "Time Stop",
    "Find Villain",
    "Castle Gate",
    "Town Gate",
    "Instant Army",
    "Raise Control",
};

const int kSpellCosts[14] = {
    2000,
    500,
    1500,
    500,
    300,
    5000,
    2000,
    100,
    200,
    1000,
    1000,
    500,
    1000,
    500,
};

const TownInfo kTownInfo[] = {
    {"Riverton", 29, 12, 0},
    {"Underfoot", 58, 4, 1},
    {"Path's End", 38, 50, 0},
    {"Anomaly", 34, 23, 1},
    {"Topshore", 5, 50, 2},
    {"Lakeview", 17, 44, 0},
    {"Simpleton", 13, 60, 2},
    {"Centrapf", 9, 39, 2},
    {"Quiln Point", 14, 27, 0},
    {"Midland", 58, 33, 1},
    {"Xoctan", 51, 28, 0},
    {"Overthere", 57, 57, 2},
    {"Elan's Landing", 3, 37, 1},
    {"King's Haven", 17, 21, 0},
    {"Bayside", 41, 58, 0},
    {"Nyre", 50, 13, 0},
    {"Dark Corner", 58, 60, 1},
    {"Isla Vista", 57, 5, 0},
    {"Grimwold", 9, 60, 3},
    {"Japper", 13, 7, 2},
    {"Vengeance", 7, 3, 3},
    {"Hunterville", 12, 3, 0},
    {"Fjord", 46, 35, 0},
    {"Yakonia", 49, 8, 2},
    {"Wood's End", 3, 8, 1},
    {"Zaezoizu", 58, 48, 3},
};

const int kTownsPerContinent[4] = {
    11, 6, 6, 3};

const int kTownIndices[4] = {
    0,
    11,
    17,
    23,
};

const CastleInfo kCastleInfo[] = {
    {"Azram", 30, 27, 0},
    {"Basefit", 47, 6, 1},
    {"Cancomar", 36, 49, 0},
    {"Duvock", 30, 18, 1},
    {"Endryx", 11, 46, 3},
    {"Faxis", 22, 49, 0},
    {"Goobare", 41, 36, 2},
    {"Hyppus", 43, 27, 2},
    {"Irok", 11, 30, 0},
    {"Jhan", 41, 34, 1},
    {"Kookamunga", 57, 58, 0},
    {"Lorshe", 52, 57, 2},
    {"Mooseweigh", 25, 39, 1},
    {"Nilslag", 22, 24, 0},
    {"Ophiraund", 6, 57, 0},
    {"Portalis", 58, 23, 0},
    {"Quinderwitch", 42, 56, 1},
    {"Rythacon", 54, 6, 0},
    {"Spockana", 17, 39, 3},
    {"Tylitch", 9, 18, 2},
    {"Uzare", 41, 12, 3},
    {"Vutar", 40, 5, 0},
    {"Wankelforte", 40, 41, 0},
    {"Xelox", 45, 6, 2},
    {"Yeneverre", 19, 19, 1},
    {"Zyzzarzaz", 46, 43, 3},
};

const int kCastlesPerContinent[4] = {
    11,
    6,
    6,
    3,
};

const int kCastleIndices[4] = {
    0,
    11,
    17,
    23,
};

const int kPuzzleVillainPositions[17] = {
    15,
    3,
    23,
    5,
    19,
    1,
    9,
    21,
    7,
    18,
    11,
    8,
    17,
    6,
    13,
    16,
    12,
};

const int kPuzzleArtifactPositions[8] = {
    10,
    2,
    4,
    14,
    20,
    24,
    22,
    0,
};

const char *const kHeroRankNames[4][4] = {
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

char const *const kHeroNames[][4] =
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

int const kRankCommission[4][4] = {
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

int const kRankSpells[4][4] = {
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

int const kRankSpellPower[4][4] = {
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

int const kRankLeadership[4][4] = {
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

int const kRankVillainsCaptured[4][4] = {
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

int const kStartingGold[4] = {
    7'500,
    10'000,
    10'000,
    7'500,
};

static constexpr unsigned char kBoxAccents[5][2][3] =
    {
        // Easy, green
        {
            {0, 32, 0},
            {66, 130, 66},
        },
        // Normal, blue
        {
            {0, 0, 33},
            {65, 65, 132},
        },
        // Hard, red
        {
            {33, 0, 0},
            {132, 65, 65},
        },
        // Impossible, grey
        {
            {33, 32, 33},
            {132, 130, 132},
        },
        // Intro, light blue
        {
            {0, 32, 99},
            {33, 163, 232},
        }};

static constexpr glm::vec4 kBoxAccentVecs[5][2] = {
    {
        {kBoxAccents[0][0][0] / 255.0f, kBoxAccents[0][0][1] / 255.0f, kBoxAccents[0][0][2] / 255.0f, 1.0f},
        {kBoxAccents[0][1][0] / 255.0f, kBoxAccents[0][1][1] / 255.0f, kBoxAccents[0][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[1][0][0] / 255.0f, kBoxAccents[1][0][1] / 255.0f, kBoxAccents[1][0][2] / 255.0f, 1.0f},
        {kBoxAccents[1][1][0] / 255.0f, kBoxAccents[1][1][1] / 255.0f, kBoxAccents[1][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[2][0][0] / 255.0f, kBoxAccents[2][0][1] / 255.0f, kBoxAccents[2][0][2] / 255.0f, 1.0f},
        {kBoxAccents[2][1][0] / 255.0f, kBoxAccents[2][1][1] / 255.0f, kBoxAccents[2][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[3][0][0] / 255.0f, kBoxAccents[3][0][1] / 255.0f, kBoxAccents[3][0][2] / 255.0f, 1.0f},
        {kBoxAccents[3][1][0] / 255.0f, kBoxAccents[3][1][1] / 255.0f, kBoxAccents[3][1][2] / 255.0f, 1.0f},
    },
    {
        {kBoxAccents[4][0][0] / 255.0f, kBoxAccents[4][0][1] / 255.0f, kBoxAccents[4][0][2] / 255.0f, 1.0f},
        {kBoxAccents[4][1][0] / 255.0f, kBoxAccents[4][1][1] / 255.0f, kBoxAccents[4][1][2] / 255.0f, 1.0f},
    },
};

char const *const kContinents[4] = {
    "Continentia",
    "Forestria",
    "Archipelia",
    "Saharia",
};

char const *const kVillains[17][4] = {
    {"Murray the Miser",
     "None",
     R"raw(
	Threadbare clothes, bald
	patch with hair combed to
	cover it. incessant cough.)raw",
     R"raw(Murray is wanted for
	various petty crimes as
	well as for treason. He
	allowed a group of pirates
	to enter the castle and
	free criminals.)raw"},
    {"Hack the Rogue",
     "The Spitter",
     R"raw(
	Bushy ebon beard stained
	with tobacco juice,
	numerous battle scars,
	brash, arrogant behavior.)raw",
     R"raw(Along with many
	minor infractions, Hack is
	wanted for conspiracy
	against the Crown and
	grave-robbing.)raw"},
    {"Princess Almola",
     "Lady Deceit",
     R"raw(
	Excessive use of make-up
	to hide aging features,
	ever-present lace
	handkerchief.)raw",
     R"raw(The Princess
	violated her status as a
	visiting dignitary by
	encouraging a murder and
	joining the conspiracy
	against the Crown.)raw"},
    {"Baron Johnno Makahl",
     "Johnno",
     R"raw(
	Expensive and gaudy
	clothes, overweight, and
	a scruffy beard.)raw",
     R"raw(Johnno is wanted for
	various crimes against the
	Kingdom, including leading
	a direct assault against
	the Crown and conspiracy.)raw"},
    {"Dread Pirate Rob",
     "Terror of the Sea",
     R"raw(
	Pencil thin mustasche
	and elegant trimmed
	beard never without a
	rapier.)raw",
     R"raw(Rob is wanted for
	piracy as well as
	conspiracy and for
	breaking out five traitors
	sentenced to death in the
	Royal Dungeons.)raw"},
    {"Caneghor the Mystic",
     "The Majestic Sage",
     R"raw(
	Voluminous robes, bald
	head, magic symbols
	engraved on body,
	levitating ability.)raw",
     R"raw(Caneghor is wanted
	for grave-robbing,
	conspiracy against the
	Crown and for plundering
	the Royal Library.)raw"},
    {"Sir Moradon the Cruel",
     "None",
     R"raw(
	Always wearing armor and
	concealed weapons, has two
	prominent front teeth and
	an unkept beard.)raw",
     R"raw(Sir Moradon, from
	another land is wanted for
	his part in a conspiracy
	to topple the kingdom.)raw"},
    {"Prince Barrowpine",
     "The Elf Lord",
     R"raw(
	Pointed ears, sharp elfin
	features, pale blue eyes
	with no whites, glimmering
	enchanted coin.)raw",
     R"raw(The prince is one of
	the leaders of the
	conspiracy against the
	Crown. He also traffics
	stolen artifacts.)raw"},
    {"Bargash Eyesore",
     "Old One Eye",
     R"raw(
	Single eye in the center
	of head, over ten feet
	tall, only hair on body
	is beard.)raw",
     R"raw(Bargash is wanted
	for conspiracy against
	the Crown and for leading
	an outright attack
	against the king.)raw"},
    {"Rinaldus Drybone",
     "The Death Lord",
     R"raw(
	Rinaldus is a magically
	animated skeleton, an
	undead, he is easily
	identified by the crown
	he wears.)raw",
     R"raw(Rinaldus is wanted
	for leading a conspiracy
	against the Crown and
	leading a rebellion on
	the continent of Saharia.)raw"},
    {"Ragface",
     "None",
     R"raw(
	Ragface is an undead,
	he is covered from head
	to toe in moldering green
	strips of cloth, a rotting
	smell follows him.)raw",
     R"raw(Conspiracy against
	the Crown and leading an
	insurrection in Saharia.)raw"},
    {"Mahk Bellowspeak",
     "Bruiser",
     R"raw(
	Bright orange body hair
	on a fluorescent green
	body. A tendency to shout
	for no apparent reason.)raw",
     R"raw(Mahk is wanted
	for the conspiracy against
	the Crown, leading a jail
	break and for piracy on
	the open seas.)raw"},
    {"Auric Whiteskin",
     "The Barbarian",
     R"raw(
	Auric is heavily muscled
	and wears a protective
	skin made from the hides
	of baby lambs.)raw",
     R"raw(Auric is wanted
	for conspiracy and for
	leading the rebellion of
	the continent of Saharia.)raw"},
    {"Czar Nikolai the Mad",
     "The Mad Czar",
     R"raw(
	The czar has eyes which
	change colour constantly,
	he also has a sulphur
	smell emmanating from his
	body.)raw",
     R"raw(The czar is wanted
	for leading a conspiracy
	against the Crown for
	violating diplomatic
	immunity and for murder.)raw"},
    {"Magus Deathspell",
     "None",
     R"raw(
	Pupil-less eyes, white
	beard, always wears
	crimson robes and a
	matching Skull cap.)raw",
     R"raw(Magus is wanted for
	conspiracy against the
	Crown and for practising
	forbidden magics.)raw"},
    {"Urthrax Killspite",
     "The Demon King",
     R"raw(
	Green, scaly skin, over
	7 feet tall, horns
	protruding.)raw",
     R"raw(Urthrax is wanted
	for conspiracy against
	the Crown.)raw"},
    {"Arech Dragonbreath",
     "Mastermind",
     R"raw(
	Arech is an immense
	dragon with a green
	body and blue wings,
	he breathes fire.)raw",
     R"raw(Arech is wanted for
	the conspiracy against
	the Crown, arranging
	jailbreaks, formenting
	rebellion, stealing the
	sceptre of order.)raw"},
};

const int kVillainContinents[17] = {
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    2,
    2,
    2,
    2,
    3,
    3,
    3,
};

int const kVillainRewards[17] = {
    5000,
    6000,
    7000,
    8000,
    9000,
    10'000,
    12'000,
    14'000,
    16'000,
    18'000,
    20'000,
    25'000,
    30'000,
    35'000,
    40'000,
    45'000,
    50'000,
};

namespace bty {

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

glm::vec4 get_color(BoxColor color, bool outline)
{
    int o = static_cast<int>(!outline);
    switch (color) {
        case BoxColor::Easy:
            return kBoxAccentVecs[0][o];
        case BoxColor::Normal:
            return kBoxAccentVecs[1][o];
        case BoxColor::Hard:
            return kBoxAccentVecs[2][o];
        case BoxColor::Impossible:
            return kBoxAccentVecs[3][o];
        case BoxColor::Intro:
            return kBoxAccentVecs[4][o];
        default:
            break;
    }
    return {1, 1, 1, 1};
}

BoxColor get_box_color(int difficulty)
{
    switch (difficulty) {
        case 0:
            return BoxColor::Easy;
        case 1:
            return BoxColor::Normal;
        case 2:
            return BoxColor::Hard;
        case 3:
            return BoxColor::Impossible;
        default:
            break;
    }
    return BoxColor::None;
}

}    // namespace bty

int const kDays[4] =
    {
        900,
        600,
        400,
        200,
};

char const *const kLoseMessage =
    {
        "Oh, %s,\n"
        "\n"
        "you have failed to\n"
        "recover the\n"
        "Sceptre of Order\n"
        "in time to save\n"
        "the land! Beloved\n"
        "King Maximus has\n"
        "died and the Demon\n"
        "King  Urthrax\n"
        "Killspite rules in\n"
        "his place. The\n"
        "Four Continents\n"
        "lay in ruin about\n"
        "you, its people\n"
        "doomed to a life\n"
        "of misery and\n"
        "oppression because\n"
        "you could not find\n"
        "the Sceptre."};

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

const int kTownsAlphabetical[26] = {
    3,
    14,
    7,
    16,
    12,
    22,
    18,
    21,
    17,
    19,
    13,
    5,
    9,
    15,
    11,
    2,
    8,
    0,
    6,
    4,
    1,
    20,
    24,
    10,
    23,
    25,
};

const int kTownGateX[26] = {
    0x1d,
    0x39,
    0x26,
    0x23,
    0x05,
    0x10,
    0x0c,
    0x09,
    0x0d,
    0x39,
    0x33,
    0x39,
    0x03,
    0x10,
    0x28,
    0x32,
    0x3a,
    0x38,
    0x09,
    0x0d,
    0x06,
    0x0c,
    0x2f,
    0x32,
    0x03,
    0x3a,
};

const int kTownGateY[26] = {
    0x0b,
    0x04,
    0x31,
    0x17,
    0x31,
    0x2c,
    0x3c,
    0x26,
    0x1b,
    0x21,
    0x1d,
    0x38,
    0x24,
    0x15,
    0x3a,
    0x0e,
    0x3b,
    0x05,
    0x3b,
    0x08,
    0x03,
    0x04,
    0x23,
    0x08,
    0x09,
    0x2f,
};
