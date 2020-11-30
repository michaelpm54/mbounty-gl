#ifndef BTY_GAME_ENTITY_HPP_
#define BTY_GAME_ENTITY_HPP_

#include "gfx/sprite.hpp"
#include "game/entity-collider.hpp"

#include "game/map.hpp"

class Entity : public bty::Sprite {
public:
    EntityCollider::CollisionManifold move(float dt, uint8_t axes, Map &map);
    void set_position(float x, float y);
    void set_tile(int tx, int ty);
    glm::vec2 get_center() const;

private:
    EntityCollider collider;
    Tile tile_{};
};

#endif // BTY_GAME_ENTITY_HPP_
