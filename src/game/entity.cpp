#include "game/entity.hpp"

#include <spdlog/spdlog.h>

#include "game/move-flags.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.hpp"

Entity::CollisionManifold Entity::move(float dx, float dy, Map &map) {
    CollisionManifold manifold;
    manifold.new_position = {position_.x, position_.y};
    manifold.collided = false;
    manifold.changed_tile = false;
    manifold.new_tile = {-1,-1,-1};

    Tile tile{};
    c2AABB entity_shape{};
    c2AABB tile_shape{};
    glm::vec2 next_pos{0};

    /* Create a collision shape */
    glm::vec2 rect_offset{16.0f, 24.0f};
    glm::vec2 rect_size{12.0f, 8.0f};

    entity_shape = {
        position_.x + rect_offset.x,
        position_.y + rect_offset.y,
        position_.x + rect_offset.x + rect_size.x,
        position_.y + rect_offset.y + rect_size.y,
    };

    /* Move along X */
    entity_shape.min.x += dx;
    entity_shape.max.x += dx;
    /* Get the new tile */
    tile = map.get_tile({entity_shape.min.x, entity_shape.min.y});
    tile_shape = {
    	tile.tx * 48.0f,
        tile.ty * 40.0f,
		tile.tx * 48.0f + 48.0f,
        tile.ty * 40.0f + 40.0f,
	};
    if (tile.id == -1) {
        manifold.collided = true;
        return manifold;
    }
    /* Test new tile for collision */
    if (!can_move(tile.id) && c2AABBtoAABB(entity_shape, tile_shape)) {
        manifold.collided = true;
        manifold.collided_tiles.push_back(tile);
        entity_shape.min.x -= dx;
        entity_shape.max.x -= dx;
    }
    else {
        /* This move is okay, proceed with it */
        manifold.new_position.x += dx;
    }

    /* Move along Y */
    entity_shape.min.y += dy;
    entity_shape.max.y += dy;
    /* Get the new tile */
    tile = map.get_tile({entity_shape.min.x, entity_shape.min.y});
    tile_shape = {
    	tile.tx * 48.0f,
        tile.ty * 40.0f,
		tile.tx * 48.0f + 48.0f,
        tile.ty * 40.0f + 40.0f,
	};
    if (tile.id == -1) {
        manifold.collided = true;
        return manifold;
    }
    /* Test new tile for collision */
    if (!can_move(tile.id) && c2AABBtoAABB(entity_shape, tile_shape)) {
        manifold.collided = true;
        manifold.collided_tiles.push_back(tile);
    }
    else {
        /* This move is okay, proceed with it */
        manifold.new_position.y += dy;
    }

    auto centre = c2Mulvs(c2Add(entity_shape.min, entity_shape.max), 0.5f);
    int tx = centre.x / 48.0f;
    int ty = centre.y / 40.0f;

    manifold.new_tile = map.get_tile(centre.x, centre.y);

    if (tile_.tx != manifold.new_tile.tx || tile_.ty != manifold.new_tile.ty) {
        manifold.changed_tile = true;
    }

    return manifold;
}

glm::vec2 Entity::get_center() const
{
    if (!texture_) {
        return get_position();
    }

    return {position_.x + 16, position_.y + 16};
}

bool Entity::can_move(int id)
{
    return id == Grass || id == GrassInFrontOfCastle;
}

void Entity::move_to_tile(const Tile &tile) {
    tile_ = tile;
    set_position({tile.tx * 48.0f + 0.0f, tile.ty * 40.0f - 8.0f});
}

void Entity::set_tile_info(const Tile &tile) {
    tile_ = tile;
}

const Tile &Entity::get_tile() const {
    return tile_;
}
