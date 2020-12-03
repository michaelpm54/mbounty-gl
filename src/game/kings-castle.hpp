#ifndef BTY_GAME_KINGS_CASTLE_HPP_
#define BTY_GAME_KINGS_CASTLE_HPP_

#include "gfx/sprite.hpp"
#include "gfx/dialog.hpp"
#include "bounty.hpp"

struct SharedState;
struct Tile;

namespace bty {
class Gfx;
struct Texture;
}

class KingsCastle {
public:
    void load(bty::Assets &assets, bty::BoxColor color, SharedState &state);
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
    SharedState *state_;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::Dialog dialog_;
    bty::Dialog recruit_;
    const bty::Texture *unit_textures_[5];

    bool show_recruit_{false};
    bool show_audience_{false};
    bool show_recruit_amount_{false};

    bty::Text *amount_{nullptr};
    bty::Text *to_buy_{nullptr};

    bool increasing_amt_ {false};
    bool decreasing_amt_ {false};
    int add_amt_{0};
    int current_amt_{0};
    float update_timer_{0};

    int added_while_holding_{0};
};

#endif // BTY_GAME_KINGS_CASTLE_HPP_
