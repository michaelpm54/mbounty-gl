#ifndef BTY_GAME_SHOP_HPP_
#define BTY_GAME_SHOP_HPP_

#include "data/bounty.hpp"
#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "game/recruit-input.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class SceneStack;
class Gfx;
struct Texture;
}    // namespace bty

struct Variables;
struct GenVariables;
struct Tile;
struct ShopInfo;
class Hud;

class Shop : public bty::Scene {
public:
    Shop(bty::SceneStack &ss, bty::Assets &assets, Variables &v, GenVariables &gen, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void set_color(bty::BoxColor color);
    void view(ShopInfo &shop);

private:
    void confirm();

private:
    bty::SceneStack &ss;
    Variables &v;
    GenVariables &gen;
    Hud &hud;
    ShopInfo *info_ {nullptr};
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::TextBox box_;
    const bty::Texture *unit_textures_[25];
    const bty::Texture *dwelling_textures_[4];
    RecruitInput recruit_input_;
};

#endif    // BTY_GAME_SHOP_HPP_
