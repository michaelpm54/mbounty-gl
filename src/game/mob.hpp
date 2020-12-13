#ifndef BTY_GAME_MOB_HPP_
#define BTY_GAME_MOB_HPP_

#include <array>
#include <glm/vec2.hpp>

#include "game/entity.hpp"

struct Mob {
    bool dead {false};
    Entity entity;
    glm::ivec2 tile;
    std::array<int, 5> army;
    std::array<int, 5> counts;
};

#endif    // BTY_GAME_MOB_HPP_
