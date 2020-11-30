#ifndef BTY_GAME_ENTITY_HPP_
#define BTY_GAME_ENTITY_HPP_

#include <vector>

#include "gfx/sprite.hpp"

#include "game/map.hpp"

class Entity : public bty::Sprite {
public:
    struct CollisionManifold {
        glm::vec2 new_position;
        bool collided;
        bool changed_tile;
        Tile new_tile;
        std::vector<Tile> collided_tiles;
    };

    Entity::CollisionManifold move(float dx, float dy, Map &map);
    void set_tile_info(const Tile &tile);
    void move_to_tile(const Tile &tile);
    const Tile &get_tile() const;
    glm::vec2 get_center() const;

protected:
    virtual bool can_move(int id);

protected:
    Tile tile_{-1,-1,-1};
};

#endif // BTY_GAME_ENTITY_HPP_
