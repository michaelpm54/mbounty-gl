#include "game/entity.hpp"

#include <spdlog/spdlog.h>

#include <unordered_set>

#include "data/tiles.hpp"
#include "game/dir-flags.hpp"
#include "gfx/gfx.hpp"

Entity::Entity(const bty::Texture *texture, const glm::vec2 &position)
    : bty::Sprite(texture, position)
{
}

glm::vec2 Entity::get_center() const
{
    if (!texture_) {
        return get_position();
    }

    return {position_.x + 24, position_.y + 16};
}

bool Entity::can_move(int id, int, int, int)
{
    return id == Tile_Grass;
}

void Entity::move_to_tile(const Tile &tile)
{
    tile_ = tile;
    set_position({tile.tx * 48.0f, tile.ty * 40.0f - 8.0f});
}

void Entity::set_tile_info(const Tile &tile)
{
    tile_ = tile;
}

const Tile &Entity::get_tile() const
{
    return tile_;
}

void Entity::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(*this, camera);
}

c2AABB Entity::get_aabb() const
{
    c2AABB aabb;

    aabb.min.x = position_.x + kEntityOffsetX;
    aabb.min.y = position_.y + kEntityOffsetY;
    aabb.max.x = aabb.min.x + kEntitySizeX;
    aabb.max.y = aabb.min.y + kEntitySizeY;

    return aabb;
}
