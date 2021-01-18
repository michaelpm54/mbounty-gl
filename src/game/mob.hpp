#ifndef BTY_GAME_MOB_HPP_
#define BTY_GAME_MOB_HPP_

#include <array>
#include <glm/vec2.hpp>

#include "game/entity.hpp"

struct Mob {
    bool dead {true};
    Entity entity;
    glm::ivec2 tile;
    std::array<int, 5> army;
    std::array<int, 5> counts;
    int id {-1};
    int spriteId {-1};
};

#endif    // BTY_GAME_MOB_HPP_
