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

glm::vec2 Entity::getCenter() const
{
    if (!_texture) {
        return getPosition();
    }

    return {_position.x + 24, _position.y + 16};
}

bool Entity::canMove(int id, int, int, int)
{
    return id == Tile_Grass;
}

void Entity::moveToTile(const Tile &tile)
{
    _tile = tile;
    setPosition({tile.tx * 48.0f, tile.ty * 40.0f - 8.0f});
}

void Entity::setTileInfo(const Tile &tile)
{
    _tile = tile;
}

const Tile &Entity::getTile() const
{
    return _tile;
}

void Entity::draw()
{
    GFX::instance().drawSprite(*this);
}

c2AABB Entity::getAABB() const
{
    c2AABB aabb;

    aabb.min.x = _position.x + kEntityOffsetX;
    aabb.min.y = _position.y + kEntityOffsetY;
    aabb.max.x = aabb.min.x + kEntitySizeX;
    aabb.max.y = aabb.min.y + kEntitySizeY;

    return aabb;
}
