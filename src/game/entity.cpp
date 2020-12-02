#include "game/entity.hpp"

#include <unordered_set>

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"

#include "game/move-flags.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.hpp"

Entity::CollisionManifold Entity::move(float dx, float dy, Map &map, int continent) {
    CollisionManifold manifold;
    manifold.new_position = {position_.x, position_.y};
    manifold.collided = false;
    manifold.changed_tile = false;
    manifold.new_tile = {-1,-1,-1};

    std::vector<glm::ivec2> narrow_phase;
    
    if (dx > 0.5f) {
        narrow_phase.push_back({tile_.tx + 1, tile_.ty + 1});
        narrow_phase.push_back({tile_.tx + 1, tile_.ty});
        narrow_phase.push_back({tile_.tx + 1, tile_.ty - 1});
    }
    else if (dx < -0.5f) {
        narrow_phase.push_back({tile_.tx - 1, tile_.ty + 1});
        narrow_phase.push_back({tile_.tx - 1, tile_.ty});
        narrow_phase.push_back({tile_.tx - 1, tile_.ty - 1});
    }

    if (dy > 0.5f) {
        if (dx > -0.5f) {
            narrow_phase.push_back({tile_.tx - 1, tile_.ty + 1});
        }
        narrow_phase.push_back({tile_.tx, tile_.ty + 1});
        if (dx < 0.5f)
            narrow_phase.push_back({tile_.tx + 1, tile_.ty + 1});
    }
    else if (dy < -0.5f) {
        if (dx > -0.5f)
            narrow_phase.push_back({tile_.tx - 1, tile_.ty - 1});
        narrow_phase.push_back({tile_.tx, tile_.ty - 1});
        if (dx < 0.5f)
            narrow_phase.push_back({tile_.tx + 1, tile_.ty - 1});
    }

    bool collide_x {false};

    for (const auto &coord : narrow_phase) {
        collided_rects_[num_collided_rects_].set_color({0.2f, 0.3f, 0.6f, 0.3f});
        collided_rects_[num_collided_rects_].set_size({48.0f, 40.0f});
        collided_rects_[num_collided_rects_].set_position({coord.x*48.0f,coord.y*40.0f});
        num_collided_rects_++;

        auto tile = map.get_tile(coord, continent);

        if (can_move(tile.id)) {
            continue;
        }

        float l = coord.x * 48.0f;
        float t = coord.y * 40.0f;
        float r = l + 48.0f;
        float b = t + 40.0f;

        c2AABB tile_shape { l, t, r, b };
        c2AABB rect_shape {
            manifold.new_position.x + dx,
            manifold.new_position.y,
            manifold.new_position.x + dx + 42.0f,
            manifold.new_position.y + 32.0f
        };

        if (c2AABBtoAABB(rect_shape, tile_shape)) {
            collide_x = true;

            collided_rects_[num_collided_rects_].set_color({0.9f, 0.3f, 0.6f, 0.9f});
            collided_rects_[num_collided_rects_].set_size({48.0f, 40.0f});
            collided_rects_[num_collided_rects_].set_position({coord.x*48.0f,coord.y*40.0f});
            num_collided_rects_++;

            manifold.collided_tiles.push_back(tile);

            break;
        }
    }

    if (!collide_x) {
        manifold.new_position.x += dx;
    }

    bool collide_y {false};

    for (const auto &coord : narrow_phase) {
        collided_rects_[num_collided_rects_].set_color({0.9f, 0.3f, 0.6f, 0.3f});
        collided_rects_[num_collided_rects_].set_size({48.0f, 40.0f});
        collided_rects_[num_collided_rects_].set_position({coord.x*48.0f,coord.y*40.0f});
        num_collided_rects_++;

        auto tile = map.get_tile(coord, continent);

        if (can_move(tile.id)) {
            continue;
        }

        float l = coord.x * 48.0f;
        float t = coord.y * 40.0f;
        float r = l + 48.0f;
        float b = t + 40.0f;

        c2AABB tile_shape { l, t, r, b };
        c2AABB rect_shape {
            manifold.new_position.x,
            manifold.new_position.y + dy,
            manifold.new_position.x + 42.0f,
            manifold.new_position.y + dy + 32.0f
        };

        if (c2AABBtoAABB(rect_shape, tile_shape)) {
            collide_y = true;

            collided_rects_[num_collided_rects_].set_color({0.9f, 0.3f, 0.6f, 0.9f});
            collided_rects_[num_collided_rects_].set_size({48.0f, 40.0f});
            collided_rects_[num_collided_rects_].set_position({coord.x*48.0f,coord.y*40.0f});
            num_collided_rects_++;

            manifold.collided_tiles.push_back(tile);

            break;
        }
    }

    if (!collide_y) {
        manifold.new_position.y += dy;
    }

    collision_rect_.set_position(manifold.new_position);
    collision_rect_.set_color({0.2f, 0.4f, 0.9f, 0.7f});
    collision_rect_.set_size({42.0f, 32.0f});

    auto tile = map.get_tile(manifold.new_position + glm::vec2{21, 16}, continent);

    if (tile.id == -1) {
        manifold.out_of_bounds = true;
        return manifold;
    }

    if (tile.tx != tile_.tx || tile.ty != tile_.ty) {
        manifold.changed_tile = true;
        manifold.new_tile = tile;
        tile_ = tile;
    }

    manifold.collided = collide_x || collide_y;

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
    set_position({tile.tx * 48.0f + 2.0f, tile.ty * 40.0f - 8.0f});
}

void Entity::set_tile_info(const Tile &tile) {
    tile_ = tile;
}

const Tile &Entity::get_tile() const {
    return tile_;
}

void Entity::set_collision_rect_visible(bool val) {
    collision_rect_visible_ = val;
}

bool Entity::get_collision_rect_visible() const {
    return collision_rect_visible_;
}

void Entity::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    gfx.draw_sprite(*this, camera);
    if (collision_rect_visible_) {
        gfx.draw_rect(collision_rect_, camera);
        for (int i = 0; i < num_collided_rects_; i++) {
            gfx.draw_rect(collided_rects_[i], camera);
        }
        collision_rect_.set_color(glm::vec4(0.2f, 0.4f, 0.9f, 0.7f));
    }
    num_collided_rects_ = 0;
}
