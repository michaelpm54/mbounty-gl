#include "bounty.hpp"

#include <fmt/format.h>

const int kVillainIndices[4] = {
    0,
    6,
    10,
    14,
};

const int kVillainsPerContinent[4] = {
    6, 4, 4, 3};

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
    /* Continentia */
    {"Bayside", 41, 58, -1},
    {"Fjord", 46, 35, -1},
    {"Hunterville", 12, 3, 9},
    {"Isla Vista", 57, 5, -1},
    {"King's Haven", 17, 21, -1},
    {"Lakeview", 17, 44, -1},
    {"Nyre", 50, 13, -1},
    {"Path's End", 38, 50, -1},
    {"Quiln Point", 14, 27, -1},
    {"Riverton", 29, 12, -1},
    {"Xoctan", 51, 28, -1},
    /* Forestria */
    {"Anomaly", 34, 23, -1},
    {"Dark Corner", 58, 60, -1},
    {"Elan's Landing", 3, 37, -1},
    {"Midland", 58, 33, -1},
    {"Underfoot", 58, 4, -1},
    {"Wood's End", 3, 8, -1},
    /* Archipelia */
    {"Centrapf", 9, 39, -1},
    {"Japper", 13, 7, -1},
    {"Overthere", 57, 57, -1},
    {"Simpleton", 13, 60, -1},
    {"Topshore", 5, 50, -1},
    {"Yakonia", 49, 8, -1},
    /* Saharia */
    {"Grimwold", 9, 60, -1},
    {"Vengeance", 7, 3, -1},
    {"Zaezoizu", 58, 48, -1},
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
    /* Continentia */
    {"Azram", 30, 27},
    {"Cancomar", 36, 49},
    {"Faxis", 22, 49},
    {"Irok", 11, 30},
    {"Kookamunga", 57, 58},
    {"Nilslag", 22, 24},
    {"Ophiraund", 6, 57},
    {"Portalis", 58, 23},
    {"Rythacon", 54, 6},
    {"Vutar", 40, 5},
    {"Wankelforte", 40, 41},
    /* Forestria */
    {"Basefit", 47, 6},
    {"Duvock", 30, 18},
    {"Jhan", 41, 34},
    {"Mooseweigh", 25, 39},
    {"Quinderwitch", 42, 56},
    {"Yeneverre", 19, 19},
    /* Archipelia */
    {"Endryx", 11, 46},
    {"Goobare", 41, 36},
    {"Hyppus", 43, 27},
    {"Lorshe", 52, 57},
    {"Tylitch", 9, 18},
    {"Xelox", 45, 6},
    /* Saharia */
    {"Spockana", 17, 39},
    {"Uzare", 41, 12},
    {"Zyzzarzaz", 46, 43},
};

const int kCastlesPerContinent[4] = {
    11, 6, 6, 3};

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
            {},
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
            {},
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
            {},
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
            {},
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
            2,
            75,
            7,
            kMoraleGroupC,
            {},
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
            14,
            250,
            25,
            kMoraleGroupB,
            {},
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
            {},
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
            {},
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
            {},
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
            {},
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
            {},
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
            {},
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
            {},
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
            {},
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
            {},
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
