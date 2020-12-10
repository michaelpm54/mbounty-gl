#ifndef BTY_GAME_ARMY_GEN_HPP_
#define BTY_GAME_ARMY_GEN_HPP_

#include <array>

void gen_villain_army(int villain, std::array<int, 5> &army, std::array<int, 5> &counts);
void gen_mob_army(int continent, std::array<int, 5> &army, std::array<int, 5> &counts);
int gen_mob_unit(int continent);
int gen_mob_count(int continent, int unit);

#endif    // BTY_GAME_ARMY_GEN_HPP_
