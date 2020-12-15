#ifndef BTY_GAME_WIZARD_HPP_
#define BTY_GAME_WIZARD_HPP_

#include "data/bounty.hpp"
#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Gfx;
struct Texture;
class DialogStack;
class SceneStack;
}    // namespace bty

struct Variables;
class Hud;

class Wizard : public bty::Scene {
public:
    Wizard(bty::SceneStack &ss, bty::Assets &assets, Variables &v, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void view(bty::DialogStack &ds);

private:
    bty::SceneStack &ss;
    Variables &v;
    Hud &hud;
    bty::Sprite bg_;
    bty::Sprite unit_;
};

#endif    // BTY_GAME_WIZARD_HPP_
