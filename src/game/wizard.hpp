#ifndef BTY_GAME_WIZARD_HPP_
#define BTY_GAME_WIZARD_HPP_

#include "bounty.hpp"
#include "game/scene.hpp"
#include "gfx/dialog.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Gfx;
struct Texture;
}    // namespace bty

class DialogStack;
struct Variables;
class SceneStack;
class Hud;

class Wizard : public Scene {
public:
    Wizard(SceneStack &ss, bty::Assets &assets, Variables &v, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void view(DialogStack &ds);

private:
    SceneStack &ss;
    Variables &v;
    Hud &hud;
    bty::Sprite bg_;
    bty::Sprite unit_;
};

#endif    // BTY_GAME_WIZARD_HPP_
