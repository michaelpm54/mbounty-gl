#include "game/entity.hpp"

#include <spdlog/spdlog.h>

#include <unordered_set>

#include "game/move-flags.hpp"
#include "gfx/gfx.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.hpp"

Entity::CollisionManifold Entity::move(float dx, float dy, Map &map, int continent)
{
    if (debug_) {
        collided_rects_.clear();
    }

    CollisionManifold manifold;
    manifold.new_position = {position_.x, position_.y};
    manifold.collided = false;
    manifold.changed_tile = false;
    manifold.new_tile = {-1, -1, -1};

    std::vector<glm::ivec2> narrow_phase;

    glm::vec2 ent_offset {12.0f, 24.0f};
    glm::vec2 ent_size {16.0f, 8.0f};

    auto bb_position = position_ + glm::vec3 {ent_offset, 0} + glm::vec3 {8, 4, 0};

    auto collision_tile = map.get_tile(bb_position.x, bb_position.y, continent);
    auto &ct = collision_tile;

    if (dx > 0.5f) {
        narrow_phase.push_back({ct.tx + 1, ct.ty + 1});
        narrow_phase.push_back({ct.tx + 1, ct.ty});
        narrow_phase.push_back({ct.tx + 1, ct.ty - 1});
    }
    else if (dx < -0.5f) {
        narrow_phase.push_back({ct.tx - 1, ct.ty + 1});
        narrow_phase.push_back({ct.tx - 1, ct.ty});
        narrow_phase.push_back({ct.tx - 1, ct.ty - 1});
    }

    if (dy > 0.5f) {
        if (dx > -0.5f) {
            narrow_phase.push_back({ct.tx - 1, ct.ty + 1});
        }
        narrow_phase.push_back({ct.tx, ct.ty + 1});
        if (dx < 0.5f)
            narrow_phase.push_back({ct.tx + 1, ct.ty + 1});
    }
    else if (dy < -0.5f) {
        if (dx > -0.5f)
            narrow_phase.push_back({ct.tx - 1, ct.ty - 1});
        narrow_phase.push_back({ct.tx, ct.ty - 1});
        if (dx < 0.5f)
            narrow_phase.push_back({ct.tx + 1, ct.ty - 1});
    }

    bool collide_x {false};

    for (const auto &coord : narrow_phase) {
        if (debug_) {
            collided_rects_.push_back({
                {0.2f, 0.3f, 0.6f, 0.3f},
                {48.0f, 40.0f},
                {coord.x * 48.0f, coord.y * 40.0f},
            });
        }

        auto tile = map.get_tile(coord, continent);

        if (can_move(tile.id)) {
            continue;
        }

        float l = coord.x * 48.0f;
        float t = coord.y * 40.0f;
        float r = l + 48.0f;
        float b = t + 40.0f;

        c2AABB tile_shape {l, t, r, b};
        c2AABB rect_shape {
            manifold.new_position.x + ent_offset.x + dx,
            manifold.new_position.y + ent_offset.y,
            manifold.new_position.x + ent_offset.x + ent_size.x + dx,
            manifold.new_position.y + ent_offset.y + ent_size.y};

        if (c2AABBtoAABB(rect_shape, tile_shape)) {
            collide_x = true;

            if (debug_) {
                collided_rects_.push_back({
                    {0.9f, 0.3f, 0.6f, 0.9f},
                    {48.0f, 40.0f},
                    {coord.x * 48.0f, coord.y * 40.0f},
                });
            }

            manifold.collided_tiles.push_back(tile);

            break;
        }
        else if (debug_) {
            collided_rects_.push_back({
                {0.3f, 1.0f, 0.6f, 0.9f},
                {48.0f, 40.0f},
                {coord.x * 48.0f, coord.y * 40.0f},
            });
        }
    }

    if (!collide_x) {
        manifold.new_position.x += dx;
    }

    bool collide_y {false};

    for (const auto &coord : narrow_phase) {
        if (debug_) {
            collided_rects_.push_back({
                {0.2f, 0.3f, 0.6f, 0.3f},
                {48.0f, 40.0f},
                {coord.x * 48.0f, coord.y * 40.0f},
            });
        }

        auto tile = map.get_tile(coord, continent);

        if (can_move(tile.id)) {
            continue;
        }

        float l = coord.x * 48.0f;
        float t = coord.y * 40.0f;
        float r = l + 48.0f;
        float b = t + 40.0f;

        c2AABB tile_shape {l, t, r, b};
        c2AABB rect_shape {
            manifold.new_position.x + ent_offset.x,
            manifold.new_position.y + ent_offset.y + dy,
            manifold.new_position.x + ent_offset.x + ent_size.x,
            manifold.new_position.y + ent_offset.y + ent_size.y + dy};

        if (c2AABBtoAABB(rect_shape, tile_shape)) {
            collide_y = true;

            if (debug_) {
                collided_rects_.push_back({
                    {0.9f, 0.3f, 0.6f, 0.9f},
                    {48.0f, 40.0f},
                    {coord.x * 48.0f, coord.y * 40.0f},
                });
            }

            manifold.collided_tiles.push_back(tile);

            break;
        }
        else if (debug_) {
            collided_rects_.push_back({
                {0.3f, 1.0f, 0.6f, 0.9f},
                {48.0f, 40.0f},
                {coord.x * 48.0f, coord.y * 40.0f},
            });
        }
    }

    if (!collide_y) {
        manifold.new_position.y += dy;
    }

    if (debug_) {
        collision_rect_.set_position(manifold.new_position + ent_offset);
        collision_rect_.set_color({0.2f, 0.4f, 0.9f, 0.7f});
        collision_rect_.set_size(ent_size);
    }

    auto tile = map.get_tile(manifold.new_position + glm::vec2 {21, 16}, continent);

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
    return id == Tile_Grass || id == Tile_GrassInFrontOfCastle;
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

void Entity::set_debug(bool val)
{
    debug_ = val;
}

bool Entity::get_debug() const
{
    return debug_;
}

void Entity::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(*this, camera);
    if (debug_) {
        gfx.draw_rect(collision_rect_, camera);
        for (int i = 0; i < collided_rects_.size(); i++) {
            gfx.draw_rect(collided_rects_[i], camera);
        }
        collision_rect_.set_color(glm::vec4(0.2f, 0.4f, 0.9f, 0.7f));
    }
}
