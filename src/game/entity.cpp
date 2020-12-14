#include "game/entity.hpp"

#include <spdlog/spdlog.h>

#include <unordered_set>

#include "game/dir-flags.hpp"
#include "gfx/gfx.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.hpp"

bool is_event_tile(int tile_id)
{
    static constexpr int kEventTiles[] = {
        Tile_Sign,
        Tile_GenSign,
        Tile_Town,
        Tile_GenTown,
        Tile_Chest,
        Tile_AfctRing,
        Tile_AfctAmulet,
        Tile_AfctAnchor,
        Tile_AfctCrown,
        Tile_AfctScroll,
        Tile_AfctShield,
        Tile_AfctSword,
        Tile_AfctBook,
    };

    for (int i = 0; i < 13; i++) {
        if (kEventTiles[i] == tile_id) {
            return true;
        }
    }

    return false;
}

c2AABB get_collision_rect(int tile_id, int x, int y)
{
    static constexpr float kSmallEntitySize = 12.0f;
    static constexpr float kSmallEntityOffsetX = (48.0f / 2.0f) - (kSmallEntitySize / 2.0f);
    static constexpr float kSmallEntityOffsetY = (40.0f / 2.0f) - (kSmallEntitySize / 2.0f);

    static constexpr float kMediumEntitySize = 20.0f;
    static constexpr float kMediumEntityOffsetX = (48.0f / 2.0f) - (kMediumEntitySize / 2.0f);
    static constexpr float kMediumEntityOffsetY = (40.0f / 2.0f) - (kMediumEntitySize / 2.0f);

    c2AABB box;
    box.min.x = x * 48.0f;
    box.min.y = y * 40.0f;

    switch (tile_id) {
        case Tile_AfctRing:
            [[fallthrough]];
        case Tile_AfctAmulet:
            [[fallthrough]];
        case Tile_AfctAnchor:
            [[fallthrough]];
        case Tile_AfctCrown:
            [[fallthrough]];
        case Tile_AfctScroll:
            [[fallthrough]];
        case Tile_AfctShield:
            [[fallthrough]];
        case Tile_AfctSword:
            [[fallthrough]];
        case Tile_AfctBook:
        case Tile_Chest:
            [[fallthrough]];
        case Tile_Sign:
            [[fallthrough]];
        case Tile_GenSign:
            box.min.x += kSmallEntityOffsetX;
            box.min.y += kSmallEntityOffsetY;
            box.max.x = box.min.x + kSmallEntitySize;
            box.max.y = box.min.y + kSmallEntitySize;
            break;
        case Tile_ShopCave:
            [[fallthrough]];
        case Tile_ShopDungeon:
            [[fallthrough]];
        case Tile_ShopTree:
            [[fallthrough]];
        case Tile_ShopWagon:
            [[fallthrough]];
        case Tile_GenWizardCave:
            [[fallthrough]];
        case Tile_Town:
            [[fallthrough]];
        case Tile_GenTown:
            box.min.x += kMediumEntityOffsetX;
            box.min.y += kMediumEntityOffsetY;
            box.max.x = box.min.x + kMediumEntitySize;
            box.max.y = box.min.y + kMediumEntitySize;
            break;
        default:
            box.max.x = box.min.x + 48.0f;
            box.max.y = box.min.y + 40.0f;
            break;
    }

    return box;
}

Entity::Entity(const bty::Texture *texture, const glm::vec2 &position)
    : bty::Sprite(texture, position)
{
}

Entity::CollisionManifold Entity::move(float dx, float dy, Map &map, int continent)
{
    static constexpr glm::vec4 kCheckedColour {0.4f, 0.8f, 0.7f, 0.8f};
    static constexpr glm::vec4 kCollidableColour {0.4f, 0.4f, 0.8f, 0.8f};
    static constexpr glm::vec4 kCollidedColour {0.8f, 0.3f, 0.2f, 0.8f};
    static constexpr glm::vec4 kEntColliderColour {0.6f, 0.5f, 0.7f, 0.88f};

    static constexpr float kEntitySizeX = 8.0f;
    static constexpr float kEntitySizeY = 8.0f;
    static constexpr float kEntityOffsetX = (44.0f / 2.0f) - (kEntitySizeX / 2.0f);
    static constexpr float kEntityOffsetY = 8.0f + (32.0f / 2.0f) - (kEntitySizeY / 2.0f);

    c2AABB ent_shape;
    ent_shape.min.x = position_.x + kEntityOffsetX;
    ent_shape.min.y = position_.y + kEntityOffsetY;
    ent_shape.max.x = ent_shape.min.x + kEntitySizeX;
    ent_shape.max.y = ent_shape.min.y + kEntitySizeY;

    if (debug_) {
        checked_rects_.clear();
        collided_rects_.clear();
    }

    CollisionManifold manifold;
    manifold.new_position = {position_.x, position_.y};
    manifold.collided = false;
    manifold.changed_tile = false;
    manifold.new_tile = {-1, -1, -1};

    std::vector<glm::ivec2> narrow_phase;

    auto collision_tile = map.get_tile(ent_shape.min.x, ent_shape.min.y, continent);
    auto &ct = collision_tile;

    narrow_phase.push_back({ct.tx, ct.ty});

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

    bool any_overlap {false};

    bool collide_x {false};

    for (const auto &coord : narrow_phase) {
        /* Get the tile ID of the checked tile. */
        auto tile = map.get_tile(coord, continent);

        /* Skip collision if we can move on this tile. */
        if (can_move(tile.id, tile.tx, tile.ty, continent)) {
            if (debug_) {
                auto tile_shape = get_collision_rect(tile.id, tile.tx, tile.ty);

                collided_rects_.push_back({
                    kCheckedColour,
                    {tile_shape.max.x - tile_shape.min.x, tile_shape.max.y - tile_shape.min.y},
                    {tile_shape.min.x, tile_shape.min.y},
                });
            }
            continue;
        }

        /* Generate a bounding box for the tile. */
        auto tile_shape = get_collision_rect(tile.id, tile.tx, tile.ty);

        /* Generate a bounding box for the entity. */
        c2AABB collider_shape {
            ent_shape.min.x + dx,
            ent_shape.min.y,
            ent_shape.max.x + dx,
            ent_shape.max.y,
        };

        /* Check if the bounding boxes collide. */
        if (collision_enabled && c2AABBtoAABB(collider_shape, tile_shape)) {
            any_overlap = true;

            if (debug_) {
                collided_rects_.push_back({
                    kCollidedColour,
                    {tile_shape.max.x - tile_shape.min.x, tile_shape.max.y - tile_shape.min.y},
                    {tile_shape.min.x, tile_shape.min.y},
                });
            }

            if (is_event_tile(tile.id)) {
                if (!collided_event_on_this_tile) {
                    manifold.collided_tiles.push_back(tile);
                }
                collided_event_on_this_tile = true;
            }
            else {
                manifold.collided_tiles.push_back(tile);
                collide_x = true;
            }

            /* Exit the loop on collision. */
            break;
        }
        else if (debug_) {
            collided_rects_.push_back({
                kCollidableColour,
                {tile_shape.max.x - tile_shape.min.x, tile_shape.max.y - tile_shape.min.y},
                {tile_shape.min.x, tile_shape.min.y},
            });
        }
    }

    if (!collide_x) {
        manifold.new_position.x += dx;
        ent_shape.min.x += dx;
        ent_shape.max.x += dx;
    }
    else {
        any_overlap = true;
    }

    bool collide_y {false};

    for (const auto &coord : narrow_phase) {
        auto tile = map.get_tile(coord, continent);

        if (can_move(tile.id, tile.tx, tile.ty, continent)) {
            if (debug_) {
                auto tile_shape = get_collision_rect(tile.id, tile.tx, tile.ty);

                collided_rects_.push_back({
                    kCheckedColour,
                    {tile_shape.max.x - tile_shape.min.x, tile_shape.max.y - tile_shape.min.y},
                    {tile_shape.min.x, tile_shape.min.y},
                });
            }
            continue;
        }

        auto tile_shape = get_collision_rect(tile.id, tile.tx, tile.ty);

        c2AABB collider_shape {
            ent_shape.min.x,
            ent_shape.min.y + dy,
            ent_shape.max.x,
            ent_shape.max.y + dy,
        };

        if (c2AABBtoAABB(collider_shape, tile_shape)) {
            any_overlap = true;

            if (debug_) {
                collided_rects_.push_back({
                    kCollidedColour,
                    {tile_shape.max.x - tile_shape.min.x, tile_shape.max.y - tile_shape.min.y},
                    {tile_shape.min.x, tile_shape.min.y},
                });
            }

            if (is_event_tile(tile.id)) {
                if (!collided_event_on_this_tile) {
                    manifold.collided_tiles.push_back(tile);
                }
                collided_event_on_this_tile = true;
            }
            else {
                manifold.collided_tiles.push_back(tile);
                collide_y = true;
            }

            break;
        }
        else if (debug_) {
            collided_rects_.push_back({
                kCollidableColour,
                {tile_shape.max.x - tile_shape.min.x, tile_shape.max.y - tile_shape.min.y},
                {tile_shape.min.x, tile_shape.min.y},
            });
        }
    }

    if (!collide_y) {
        manifold.new_position.y += dy;
        ent_shape.min.y += dy;
        ent_shape.max.y += dy;
    }
    else {
        any_overlap = true;
    }

    if (debug_) {
        collision_rect_.set_position(ent_shape.min.x, ent_shape.min.y);
        collision_rect_.set_color(kEntColliderColour);
        collision_rect_.set_size(ent_shape.max.x - ent_shape.min.x, ent_shape.max.y - ent_shape.min.y);
    }

    auto tile = map.get_tile(manifold.new_position + glm::vec2 {21, 16}, continent);

    if (tile.id == -1) {
        manifold.out_of_bounds = true;
        return manifold;
    }

    if (tile.tx != tile_.tx || tile.ty != tile_.ty && !any_overlap) {
        collided_event_on_this_tile = false;
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

bool Entity::can_move(int id, int x, int y, int c)
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
        for (auto &rect : checked_rects_) {
            gfx.draw_rect(rect, camera);
        }
        for (auto &rect : collided_rects_) {
            gfx.draw_rect(rect, camera);
        }
        gfx.draw_rect(collision_rect_, camera);
    }
}

void Entity::set_collision_enabled(bool val)
{
    collision_enabled = val;
}
