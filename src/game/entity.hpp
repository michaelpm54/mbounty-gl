#ifndef BTY_GAME_ENTITY_HPP_
#define BTY_GAME_ENTITY_HPP_

#include <vector>

#include "game/cute_c2.hpp"
#include "game/map.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"

inline constexpr float kEntitySizeX = 8.0f;
inline constexpr float kEntitySizeY = 8.0f;
inline constexpr float kEntityOffsetX = (44.0f / 2.0f) - (kEntitySizeX / 2.0f);
inline constexpr float kEntityOffsetY = 8.0f + (32.0f / 2.0f) - (kEntitySizeY / 2.0f);

namespace bty {
class Gfx;
class Rect;
}    // namespace bty

class Entity : public bty::Sprite {
public:
    struct CollisionManifold {
        glm::vec2 new_position;
        bool collided;
        bool changed_tile;
        Tile new_tile;
        std::vector<Tile> collided_tiles;
        bool out_of_bounds {false};
    };

    Entity() = default;
    Entity(const Entity &) = default;
    Entity(const bty::Texture *texture, const glm::vec2 &position);
    void set_tile_info(const Tile &tile);
    void move_to_tile(const Tile &tile);
    const Tile &get_tile() const;
    glm::vec2 get_center() const;
    void set_debug(bool val);
    bool get_debug() const;
    void draw(bty::Gfx &gfx, glm::mat4 &camera);

    c2AABB get_aabb() const;

protected:
    virtual bool can_move(int id, int x, int y, int c);

protected:
    Tile tile_ {-1, -1, -1};
};

#endif    // BTY_GAME_ENTITY_HPP_
