#ifndef BTY_GAME_VARIABLES_HPP_
#define BTY_GAME_VARIABLES_HPP_

#include <array>
#include <glm/vec2.hpp>
#include <vector>

struct Variables {
    int days;
    int hero;
    int rank;
    int diff;
    int contract;
    int continent;
    int num_spells;
    int score;
    int gold;
    int leadership;
    int permanent_leadership;
    int commission;
    int spell_power;
    int known_spells;
    int max_spells;
    int followers_killed;
    int weeks_passed;
    int days_passed_this_week;
    int boat_x;
    int boat_y;
    int boat_c;
    bool magic;
    bool siege;
    bool boat_rented;
    std::array<bool, 26> visited_castles;
    std::array<bool, 26> visited_towns;
    std::array<int, 14> spells;
    std::array<std::vector<unsigned char>, 4> visited_tiles;
    std::array<int, 5> army;
    std::array<int, 5> counts;
    std::array<int, 5> morales;
    bool auto_move;
    float auto_move_timer;
    glm::ivec2 auto_move_dir;
    std::array<const unsigned char *, 4> tiles;
    int x;
    int y;
    int timestop;
};

#endif    // BTY_GAME_VARIABLES_HPP_
