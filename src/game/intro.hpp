#ifndef BTY_GAME_INTRO_HPP_
#define BTY_GAME_INTRO_HPP_

#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "engine/textbox.hpp"
#include "game/ingame.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"


namespace bty {
class Assets;
class Gfx;
class SceneStack;
class DialogStack;
}    // namespace bty

class Intro : public bty::Scene {
public:
    Intro(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Ingame &ingame);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

private:
    void show_difficulty();

private:
    bty::SceneStack &ss;
    bty::DialogStack &ds;
    Ingame &ingame;
    int hero {0};
    bty::Sprite bg_;
    bty::TextBox name_box_;
    bty::TextBox help_box_;
};

#endif    // BTY_INTRO_INTRO_HPP_
