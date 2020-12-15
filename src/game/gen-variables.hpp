#ifndef BTY_GEN_VARIABLES_HPP_
#define BTY_GEN_VARIABLES_HPP_

#include <array>
#include <glm/vec2.hpp>
#include <vector>

#include "game/mob.hpp"
#include "game/shop-info.hpp"

struct GenVariables {
    std::array<std::array<Mob, 40>, 4> mobs;
    std::array<std::vector<unsigned int>, 4> friendly_mobs;
    std::array<bool, 17> villains_found;
    std::array<bool, 17> villains_captured;
    std::array<bool, 4> sail_maps_found;
    std::array<bool, 4> continent_maps_found;
    std::array<int, 26> town_units;
    std::array<int, 26> town_spells;
    std::array<int, 26> castle_occupants;
    std::array<std::array<int, 5>, 26> garrison_armies;
    std::array<std::array<int, 5>, 26> garrison_counts;
    std::array<std::array<int, 5>, 26> castle_armies;
    std::array<std::array<int, 5>, 26> castle_counts;
    std::array<glm::ivec2, 3> sail_map_tiles;
    std::array<glm::ivec2, 4> continent_map_tiles;
    std::array<std::array<glm::ivec2, 2>, 4> teleport_cave_tiles;
    std::array<std::vector<ShopInfo>, 4> shops;
    std::array<bool, 8> artifacts_found;
};

#endif    // BTY_GEN_VARIABLES_HPP_
