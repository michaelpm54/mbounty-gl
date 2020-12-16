#ifndef BTY_GAME_HERO_HPP_
#define BTY_GAME_HERO_HPP_

#include "game/entity.hpp"

namespace bty {
class Assets;
struct Texture;
}    // namespace bty

enum class Mount {
    Fly,
    Boat,
    Walk,
};

class Hero : public Entity {
public:
    Hero(int &boat_x, int &boat_y, int &boat_c);
    void load(bty::Assets &assets);
    bool can_move(int id, int x, int y, int c) override;
    void set_mount(Mount mount);
    Mount get_mount() const;
    void set_moving(bool val);

private:
    void update_texture();

private:
    bool moving_ {false};
    Mount mount_ {Mount::Walk};
    const bty::Texture *tex_walk_moving_;
    const bty::Texture *tex_walk_stationary_;
    const bty::Texture *tex_boat_moving_;
    const bty::Texture *tex_boat_stationary_;
    const bty::Texture *tex_flying;
    int &boat_x;
    int &boat_y;
    int &boat_c;
};

#endif    // BTY_GAME_HERO_HPP_
