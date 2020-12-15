#ifndef BTY_GAME_KINGS_CASTLE_HPP_
#define BTY_GAME_KINGS_CASTLE_HPP_

#include "data/bounty.hpp"
#include "data/color.hpp"
#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "game/recruit-input.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Gfx;
struct Texture;
class SceneStack;
class DialogStack;
class Assets;
}    // namespace bty

class Hud;
struct Variables;
struct GenVariables;

class KingsCastle : public bty::Scene {
public:
    KingsCastle(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Hud &hud, Variables &v, GenVariables &gen);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

    void view();
    void set_gold(int gold);
    void set_color(bty::BoxColor color);

private:
    void recruit_opt();
    void main_opt();
    void show_audience();

private:
    bty::SceneStack &ss;
    bty::DialogStack &ds;
    Hud &hud;
    Variables &v;
    GenVariables &gen;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::Dialog dialog_;
    bty::Dialog recruit_;
    const bty::Texture *unit_textures_[5];

    bool show_recruit_ {false};
    bool show_recruit_amount_ {false};

    bty::Text *may_get_ {nullptr};
    bty::Text *how_many_ {nullptr};

    RecruitInput recruit_input_;
};

#endif    // BTY_GAME_KINGS_CASTLE_HPP_
