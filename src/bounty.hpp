#ifndef BTY_BOUNTY_HPP_
#define BTY_BOUNTY_HPP_

#include <string>
#include <vector>

#include <glm/vec4.hpp>

extern char const* const kHeroNames[][4];
extern int const kDays[4];
extern char const* const kLoseMessage;
extern char const *const kContinents[4];
extern char const *const kVillains[17][4];
extern int const kVillainRewards[17];

namespace bty {

enum class BoxColor {
    Easy,
    Normal,
    Hard,
    Impossible,
    Intro,
    None,
};

glm::vec4 get_color(BoxColor color, bool outline = false);
BoxColor get_box_color(int difficulty);

}

enum UnitId
{
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

struct Unit
{
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
	std::vector<std::string> abilities;
};

extern const Unit kUnits[UnitId::UnitCount];

#endif // BTY_BOUNTY_HPP_
