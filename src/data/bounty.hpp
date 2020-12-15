#ifndef BTY_DATA_BOUNTY_HPP_
#define BTY_DATA_BOUNTY_HPP_

#include <array>
#include <glm/vec4.hpp>
#include <string>

struct TownInfo {
    std::string name;
    int x;
    int y;
    int continent;
};

struct CastleInfo {
    std::string name;
    int x;
    int y;
    int continent;
};

extern const int kTownBoatX[26];
extern const int kTownBoatY[26];
extern const char *const kSpellNames[14];
extern const int kSpellCosts[14];
extern const TownInfo kTownInfo[];
extern const int kTownsPerContinent[4];
extern const int kTownIndices[4];
extern const CastleInfo kCastleInfo[];
extern const int kCastlesPerContinent[4];
extern const int kCastleIndices[4];

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

extern const int kVillainIndices[4];
extern const int kVillainsPerContinent[4];
extern const int kPuzzleVillainPositions[17];
extern const int kPuzzleArtifactPositions[8];
extern int const kDays[4];
extern char const *const kLoseMessage;
extern char const *const kContinents[4];
extern char const *const kVillains[17][4];
extern const int kVillainContinents[17];
extern int const kVillainRewards[17];
extern const int kTownsAlphabetical[26];    // ? has more than 26
extern const int kTownGateX[26];
extern const int kTownGateY[26];

namespace bty {

void sort_army(std::array<int, 5> &army, std::array<int, 5> &counts);
int check_morale(int me, int *army);
std::string get_descriptor(int count);
int random(int max);
int random(int min, int max);
std::string number_with_ks(int num);

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

extern const Unit kUnits[UnitId::UnitCount];

#endif    // BTY_DATA_BOUNTY_HPP_
