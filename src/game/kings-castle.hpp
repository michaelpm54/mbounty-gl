#ifndef BTY_GAME_KINGS_CASTLE_HPP_
#define BTY_GAME_KINGS_CASTLE_HPP_

#include "bounty.hpp"
#include "game/recruit-input.hpp"
#include "gfx/dialog.hpp"
#include "gfx/sprite.hpp"

struct SharedState;
struct Tile;
class Hud;

namespace bty {
class Gfx;
struct Texture;
}    // namespace bty

class KingsCastle {
public:
    void load(bty::Assets &assets, bty::BoxColor color, SharedState &state, Hud &hud_);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view();
    void update(float dt);
    int key(int key, int action);
    void update_gold();
    void set_color(bty::BoxColor color);

private:
    void recruit_opt();
    void main_opt();

private:
    Hud *hud_;
    SharedState *state_;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::Dialog dialog_;
    bty::Dialog recruit_;
    const bty::Texture *unit_textures_[5];

    bool show_recruit_ {false};
    bool show_audience_ {false};
    bool show_recruit_amount_ {false};

    bty::Text *may_get_ {nullptr};
    bty::Text *how_many_ {nullptr};

    RecruitInput recruit_input_;
};

#endif    // BTY_GAME_KINGS_CASTLE_HPP_
