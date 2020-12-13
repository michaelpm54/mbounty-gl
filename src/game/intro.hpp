#ifndef BTY_GAME_INTRO_HPP_
#define BTY_GAME_INTRO_HPP_

#include "game/dialog-stack.hpp"
#include "game/scene.hpp"
#include "gfx/dialog.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"

namespace bty {
class Assets;
class Gfx;
class SceneSwitcher;
}    // namespace bty

class SceneStack;

class Intro : public Scene {
public:
    Intro(SceneStack &ss, DialogStack &ds, bty::Assets &assets, int &hero_id, int &difficulty);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

private:
    void show_difficulty();

private:
    SceneStack &ss;
    DialogStack &ds;
    int &hero_id_;
    int &difficulty_;
    bool chosen_hero_ {false};
    int hero_ {0};
    bty::Sprite bg_;
    bty::TextBox name_box_;
    bty::TextBox help_box_;
};

#endif    // BTY_INTRO_INTRO_HPP_
