#ifndef BTY_GAME_ENTITY_HPP_
#define BTY_GAME_ENTITY_HPP_

#include <vector>

#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"

#include "game/map.hpp"

namespace bty {
class Gfx;
class Rect;
}

class Entity : public bty::Sprite {
public:
    struct CollisionManifold {
        glm::vec2 new_position;
        bool collided;
        bool changed_tile;
        Tile new_tile;
        std::vector<Tile> collided_tiles;
        bool out_of_bounds{false};
    };

    Entity::CollisionManifold move(float dx, float dy, Map &map, int continent);
    void set_tile_info(const Tile &tile);
    void move_to_tile(const Tile &tile);
    const Tile &get_tile() const;
    glm::vec2 get_center() const;
    void set_debug(bool val);
    bool get_debug() const;
    void draw(bty::Gfx &gfx, glm::mat4 &camera);

protected:
    virtual bool can_move(int id);

protected:
    Tile tile_{-1,-1,-1};

private:
    bty::Rect collision_rect_;
    bool debug_{false};
    std::vector<bty::Rect> collided_rects_;
};

#endif // BTY_GAME_ENTITY_HPP_
