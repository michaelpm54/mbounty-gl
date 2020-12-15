#ifndef BTY_GAME_DEFEAT_HPP_
#define BTY_GAME_DEFEAT_HPP_

#include <glm/mat4x4.hpp>

#include "engine/scene.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Assets;
class Gfx;
class SceneStack;
class DialogStack;
}    // namespace bty

class Hud;

class Defeat : public bty::Scene {
public:
    Defeat(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action);
    void update(float dt);

    void show(int hero);

private:
    bty::SceneStack &ss;
    bty::DialogStack &ds;
    Hud &hud;
    bty::Sprite lose_pic;
};

#endif    // BTY_GAME_DEFEAT_HPP_
