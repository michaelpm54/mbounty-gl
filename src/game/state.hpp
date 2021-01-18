#ifndef BTY_GAME_STATE_HPP
#define BTY_GAME_STATE_HPP

#include <array>
#include <glm/vec2.hpp>
#include <vector>

#include "game/mob.hpp"
#include "game/shop-info.hpp"
#include "game/town-gen.hpp"

class State {
public:
    static int days;
    static int hero;
    static int rank;
    static int difficulty;
    static int contract;
    static int continent;
    static int score;
    static int gold;
    static int leadership;
    static int permanent_leadership;
    static int commission;
    static int spell_power;
    static int known_spells;
    static int max_spells;
    static int followers_killed;
    static int weeks_passed;
    static int days_passed_this_week;
    static int boat_x;
    static int boat_y;
    static int boat_c;
    static bool magic;
    static bool siege;
    static bool boat_rented;
    static bool combat;
    static std::array<bool, 26> visited_castles;
    static std::array<int, 14> spells;
    static std::array<std::vector<unsigned char>, 4> visited_tiles;
    static std::array<int, 5> army;
    static std::array<int, 5> counts;
    static std::array<int, 5> morales;
    static bool auto_move;
    static glm::ivec2 auto_move_dir;
    static std::array<const unsigned char *, 4> tiles;
    static int x;
    static int y;
    static int timestop;
    static std::array<std::array<Mob, 40>, 4> mobs;
    static std::array<std::vector<int>, 4> friendly_mobs;
    static std::array<bool, 17> villains_found;
    static std::array<bool, 17> villains_captured;
    static std::array<bool, 4> sail_maps_found;
    static std::array<bool, 4> continent_maps_found;
    static std::array<int, 26> castle_occupants;
    static std::array<std::array<int, 5>, 26> garrison_armies;
    static std::array<std::array<int, 5>, 26> garrison_counts;
    static std::array<std::array<int, 5>, 26> castle_armies;
    static std::array<std::array<int, 5>, 26> castle_counts;
    static std::array<glm::ivec2, 3> sail_map_tiles;
    static std::array<glm::ivec2, 4> continent_map_tiles;
    static std::array<std::array<glm::ivec2, 2>, 4> teleport_cave_tiles;
    static std::array<std::vector<ShopInfo>, 4> shops;
    static std::array<TownGen, 26> towns;
    static std::array<bool, 8> artifacts_found;
};

#endif    // BTY_GAME_STATE_HPP
