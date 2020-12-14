#ifndef BTY_GAME_GARRISON_HPP_
#define BTY_GAME_GARRISON_HPP_

#include "bounty.hpp"
#include "game/recruit-input.hpp"
#include "game/scene.hpp"
#include "gfx/dialog.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Gfx;
struct Texture;
}    // namespace bty

class SceneStack;
class DialogStack;
class Assets;
class Hud;
struct Variables;
struct GenVariables;

class Garrison : public Scene {
public:
    Garrison(SceneStack &ss, DialogStack &ds, bty::Assets &assets, Hud &hud, Variables &v, GenVariables &gen);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

    void view(int castle_id);

private:
    void select(int opt);
    void next();
    void prev();
    void update_units(int opt);

private:
    SceneStack &ss;
    DialogStack &ds;
    Hud &hud;
    Variables &v;
    GenVariables &gen;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::Dialog dialog_;
    const bty::Texture *unit_textures_[5];

    bty::Text *t_castle_name;
    bty::Text *t_gp;
    bty::Text *t_units[5];

    int castle_id {-1};
};

#endif    // BTY_GAME_GARRISON_HPP_
