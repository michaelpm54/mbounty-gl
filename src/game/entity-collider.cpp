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

bool EntityCollider::resolve_collision(float dx, float dy, Map &map)
{
    glm::vec2 next_pos { shape_.min.x, shape_.min.y };

    bool block { false };

    int tile {map.get_tile(next_pos.x, next_pos.y).id};

    // Out of bounds, treat as collision
    if (tile == -1)
        return true;

    float tx = next_pos.x / 48.0f;
    float ty = next_pos.y / 40.0f;

    // regular collision
    if (tile > 1 && collides(next_pos.x, next_pos.y))
        block = true;

    if (block)
    	move(-dx, -dy);

	return block;
}

EntityCollider::CollisionManifold EntityCollider::resolve(float dt, std::uint8_t axes, Map &map)
{
	CollisionManifold manifold;

	if (axes == MOVE_FLAGS_NONE) {
		manifold.new_position = {shape_.min.x - offset_.x, shape_.min.y - offset_.y};
		manifold.collided = false;
		manifold.new_tile = {-1,-1,-1};
		return manifold;
	}

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

	Tile new_tile;

    /* Resolve X move */
	move(dx, 0);
	new_tile = map.get_tile({ shape_.min.x, shape_.min.y });
	if (resolve_collision(dx, 0, map)) {
		manifold.collided = true;
		manifold.collided_tiles.push_back(new_tile);
	}

    /* Resolve Y move */
	move(0, dy);
	new_tile = map.get_tile({ shape_.min.x, shape_.min.y });
	if (resolve_collision(0, dy, map)) {
		manifold.collided = true;
		manifold.collided_tiles.push_back(new_tile);
	}

	auto centre = c2Mulvs(c2Add(shape_.min, shape_.max), 0.5f);
	int tx = centre.x / 48.0f;
	int ty = centre.y / 40.0f;

	manifold.new_tile = {tx, ty, map.get_tile(tx, ty)};
	manifold.new_position = {shape_.min.x - offset_.x, shape_.min.y - offset_.y};

    return manifold;
}

void EntityCollider::move(float dx, float dy)
{
    shape_.min.x += dx;
    shape_.min.y += dy;
    shape_.max.x += dx;
    shape_.max.y += dy;
}
