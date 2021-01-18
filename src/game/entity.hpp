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
    Entity() = default;
    Entity(const Entity &) = default;
    Entity(const bty::Texture *texture, const glm::vec2 &position);
    void setTileInfo(const Tile &tile);
    void moveToTile(const Tile &tile);
    const Tile &getTile() const;
    glm::vec2 getCenter() const;
    void setDebug(bool val);
    bool getDebug() const;
    void draw();

    c2AABB getAABB() const;

protected:
    virtual bool canMove(int id, int x, int y, int c);

protected:
    Tile _tile {-1, -1, -1};
};

#endif    // BTY_GAME_ENTITY_HPP_
