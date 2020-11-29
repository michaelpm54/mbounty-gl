#include "game/entity-collider.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.hpp"

#include <spdlog/spdlog.h>

#include "game/move-flags.hpp"
#include "game/map.hpp"

void EntityCollider::set_position(float x, float y)
{
    shape_.min.x = x + offset_.x;
	shape_.min.y = y + offset_.y;
	shape_.max.x = shape_.min.x + size_.x;
	shape_.max.y = shape_.min.y + size_.y;
}

bool EntityCollider::collides(float x, float y) const
{
    c2AABB tileAABB{
    	{ x,  y },
		{x + 48.0f, y + 40.0f }
	};

    return c2AABBtoAABB(shape_, tileAABB);
}

void EntityCollider::resolve_collision(float dx, float dy, Map &map)
{
    glm::vec2 next_pos { shape_.min.x + dx, shape_.min.y + dy };

    bool block { false };

    int tile {map.get_tile(next_pos.x, next_pos.y)};

    // Out of bounds, treat as collision
    if (tile == -1)
        block = true;

    float tx = next_pos.x / 48.0f;
    float ty = next_pos.y / 40.0f;

    // regular collision
    if (tile > 1 && collides(next_pos.x, next_pos.y))
        block = true;

    if (block)
    {
    	move(-dx, -dy);
    	// mCollisions.insert({tile, int(tx), int(ty)});
    }
    // else if (isEventTile(tile))
    {
    	// mCollisions.insert({tile, int(tx), int(ty)});
    }
}

glm::vec2 EntityCollider::move(float dt, std::uint8_t axes, Map &map)
{
	if (axes == MOVE_FLAGS_NONE)
		return {shape_.min.x - offset_.x, shape_.min.y - offset_.y};

	glm::vec2 dir{0.0f};

	if (axes & MOVE_FLAGS_UP)
		dir.y -= 1.0f;
	if (axes & MOVE_FLAGS_DOWN)
		dir.y += 1.0f;
	if (axes & MOVE_FLAGS_LEFT)
		dir.x -= 1.0f;
	if (axes & MOVE_FLAGS_RIGHT)
		dir.x += 1.0f;

	static constexpr float speed = 100.0f;
	float vel = speed * dt;
	float dx = dir.x * vel;
	float dy = dir.y * vel;

	// const auto oldTile = mTile;

    /* Resolve X move */
	move(dx, 0);
	resolve_collision(dx, 0, map);

    /* Resolve Y move */
	move(0, dy);
	resolve_collision(0, dy, map);

    /*
	auto centre = c2Mulvs(c2Add(shape_.min, shape_.max), 0.5f);
    
	int tx = centre.x / 48.0f;
	int ty = centre.y / 40.0f;
	mTile = {map->get_tile(tx, ty), tx, ty};

	const auto newTile = mTile;

	if (oldTile != newTile)
	{
		mChangedTile = true;
		mNewTile = newTile;
	}
    */

    return {shape_.min.x - offset_.x, shape_.min.y - offset_.y};
}

void EntityCollider::move(float dx, float dy)
{
    shape_.min.x += dx;
    shape_.min.y += dy;
    shape_.max.x += dx;
    shape_.max.y += dy;
}
