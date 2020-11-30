#ifndef BTY_ENTITY_COLLIDER_HPP_
#define BTY_ENTITY_COLLIDER_HPP_

#include <vector>

#include <glm/vec2.hpp>

#include "cute_c2.hpp"

#include "game/map.hpp"

class Map;

class EntityCollider {
public:
    struct CollisionManifold {
        glm::vec2 new_position;
        bool collided;
        Tile new_tile;
        std::vector<Tile> collided_tiles;
    };

    void set_position(float x, float y);
    CollisionManifold resolve(float dt, uint8_t axes, Map &map);

private:
    void move(float dx, float dy);
    bool collides(float x, float y) const;
    bool resolve_collision(float dx, float dy, Map &map);

private:
    c2AABB shape_{0.0f, 0.0f, 0.0f, 0.0f};
    glm::vec2 offset_{14.0f, 24.0f};
    glm::vec2 size_{8.0f, 8.0f};
};

#endif // BTY_ENTITY_COLLIDER_HPP_
