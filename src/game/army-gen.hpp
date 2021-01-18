#ifndef BTY_GAME_ARMY_GEN_HPP_
#define BTY_GAME_ARMY_GEN_HPP_

#include <array>

void genVillainArmy(int villain, std::array<int, 5> &army, std::array<int, 5> &counts);
void genMobArmy(int continent, std::array<int, 5> &army, std::array<int, 5> &counts);
void genCastleArmy(int continent, std::array<int, 5> &army, std::array<int, 5> &counts);
int genMobUnit(int continent);
int genMobCount(int continent, int unit);

#endif    // BTY_GAME_ARMY_GEN_HPP_
