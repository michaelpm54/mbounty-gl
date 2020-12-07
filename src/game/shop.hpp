#ifndef BTY_GAME_SHOP_HPP_
#define BTY_GAME_SHOP_HPP_

#include "bounty.hpp"
#include "game/recruit-input.hpp"
#include "gfx/dialog.hpp"
#include "gfx/sprite.hpp"

struct SharedState;
struct Tile;
struct ShopInfo;
class Hud;

namespace bty {
class Gfx;
struct Texture;
}    // namespace bty

class Shop {
public:
    void load(bty::Assets &assets, bty::BoxColor color, SharedState &state, Hud &hud);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(ShopInfo &shop);
    void update(float dt);
    int key(int key, int action);
    void set_color(bty::BoxColor color);

private:
    void confirm();

private:
    ShopInfo *info_ {nullptr};
    SharedState *state_;
    Hud *hud_;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::TextBox box_;
    const bty::Texture *unit_textures_[25];
    const bty::Texture *dwelling_textures_[4];
    RecruitInput recruit_input_;
};

#endif    // BTY_GAME_SHOP_HPP_
