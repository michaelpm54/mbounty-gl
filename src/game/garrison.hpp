#ifndef BTY_GAME_GARRISON_HPP_
#define BTY_GAME_GARRISON_HPP_

#include "data/bounty.hpp"
#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "game/recruit-input.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Gfx;
struct Texture;
class SceneStack;
class DialogStack;
}    // namespace bty

class Hud;
struct Variables;
struct GenVariables;

class Garrison : public bty::Scene {
public:
    Garrison(bty::SceneStack &ss, bty::DialogStack &ds, Hud &hud, Variables &v, GenVariables &gen);
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
    bty::SceneStack &ss;
    bty::DialogStack &ds;
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
