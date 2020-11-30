#include "game/entity.hpp"

#include <spdlog/spdlog.h>

#include "game/move-flags.hpp"

EntityCollider::CollisionManifold Entity::move(float dt, uint8_t axes, Map &map) {
    auto manifold = collider.resolve(dt, axes, map);
    Transformable::set_position(manifold.new_position);
    return manifold;
}

void Entity::set_position(float x, float y)
{
    collider.set_position(x, y);
    Transformable::set_position({x, y});
}

void Entity::set_tile(int tx, int ty)
{
    float x = tx * 48.0f + 8;
    float y = ty * 40.0f + 8;

    collider.set_position(x, y);

    Transformable::set_position({x, y});
}

glm::vec2 Entity::get_center() const
{
    if (!texture_) {
        return get_position();
    }

    return {position_.x + 16, position_.y + 16};
}
