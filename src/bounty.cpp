#include "bounty.hpp"

char const* const kHeroNames[][4] =
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

static constexpr unsigned char kBoxAccents[5][2][3] =
{
	// Easy, green
	{
		{ 0, 32, 0 },
		{ 66, 130, 66 },
	},
	// Normal, blue
	{
		{ 0, 0, 33 },
		{ 65, 65, 132 },
	},
	// Hard, red
	{
		{ 33, 0, 0 },
		{ 132, 65, 65 },
	},
	// Impossible, grey
	{
		{ 33, 32, 33 },
		{ 132, 130, 132 },
	},
    // Intro, light blue
    {
        { 0, 32, 99 },
        { 33, 163, 232 },
    }
};

static constexpr glm::vec4 kBoxAccentVecs[5][2] = {
    {
        { kBoxAccents[0][0][0] / 255.0f, kBoxAccents[0][0][1] / 255.0f, kBoxAccents[0][0][2] / 255.0f, 1.0f },
        { kBoxAccents[0][1][0] / 255.0f, kBoxAccents[0][1][1] / 255.0f, kBoxAccents[0][1][2] / 255.0f, 1.0f },
    },
    {
        { kBoxAccents[1][0][0] / 255.0f, kBoxAccents[1][0][1] / 255.0f, kBoxAccents[1][0][2] / 255.0f, 1.0f },
        { kBoxAccents[1][1][0] / 255.0f, kBoxAccents[1][1][1] / 255.0f, kBoxAccents[1][1][2] / 255.0f, 1.0f },
    },
    {
        { kBoxAccents[2][0][0] / 255.0f, kBoxAccents[2][0][1] / 255.0f, kBoxAccents[2][0][2] / 255.0f, 1.0f },
        { kBoxAccents[2][1][0] / 255.0f, kBoxAccents[2][1][1] / 255.0f, kBoxAccents[2][1][2] / 255.0f, 1.0f },
    },
    {
        { kBoxAccents[3][0][0] / 255.0f, kBoxAccents[3][0][1] / 255.0f, kBoxAccents[3][0][2] / 255.0f, 1.0f },
        { kBoxAccents[3][1][0] / 255.0f, kBoxAccents[3][1][1] / 255.0f, kBoxAccents[3][1][2] / 255.0f, 1.0f },
    },
    {
        { kBoxAccents[4][0][0] / 255.0f, kBoxAccents[4][0][1] / 255.0f, kBoxAccents[4][0][2] / 255.0f, 1.0f },
        { kBoxAccents[4][1][0] / 255.0f, kBoxAccents[4][1][1] / 255.0f, kBoxAccents[4][1][2] / 255.0f, 1.0f },
    },
};

char const *const kContinents[4] = {
	"Continentia",
	"Forestria",
	"Archipelia",
	"Saharia",
};

namespace bty {

glm::vec4 get_color(BoxColor color, bool outline) {
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
    return {1,1,1,1};
}

BoxColor get_box_color(int difficulty) {
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

}

int const kDays[4] =
{
	900,
	600,
	400,
	200,
};

char const* const kLoseMessage =
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
	"the Sceptre."
};

const Unit kUnits[UnitId::UnitCount] =
{
	{
		"Peasant",
		"Peasants",
		1,    // hp
		1,    // skill_level
		1,    // moves
		1,    // dmg_min
		1,    // dmg_max
		0,    // shot_dmg_min
		0,    // shot_dmg_max
		0,    // num_shots
		10,   // recruit_cost
		1,    // weekly_cost
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
		{"Fly"},
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
		{"Undead"},
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
		{"Undead"},
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
		{"Absorb", "Undead"},
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
		{"Regen"},
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
		{"Magic"},
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
		{"Fly", "Magic"},
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
		{"Leech","Fly","Undead"},
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
		{"Fly", "Scythe"},
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
		{"Fly", "Immune"},
	},
};
