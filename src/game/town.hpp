#ifndef BTY_GAME_TOWN_HPP_
#define BTY_GAME_TOWN_HPP_

#include "bounty.hpp"
#include "gfx/dialog.hpp"
#include "gfx/sprite.hpp"

struct SharedState;
struct Tile;

namespace bty {
class Gfx;
struct Texture;
}    // namespace bty

class Town {
public:
    void load(bty::Assets &assets, bty::BoxColor color, SharedState &state);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(int town, const Tile &tile, int continent, int unit_id, int spell);
    void update(float dt);
    int key(int key);
    int get_town() const;
    void update_gold();
    void set_color(bty::BoxColor color);

private:
    SharedState *state_;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::Dialog dialog_;
    const bty::Texture *unit_textures_[25];
    bty::TextBox gather_information_;
    bool show_gather_information_ {false};
    int current_info_contract_ {-1};
    int town_ {-1};
};

#endif    // BTY_GAME_TOWN_HPP_
