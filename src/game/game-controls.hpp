#ifndef GAME_GAME_CONTROLS_HPP_
#define GAME_GAME_CONTROLS_HPP_

#include "engine/dialog.hpp"
#include "engine/scene.hpp"

namespace bty {
class Asssets;
class Gfx;
class SceneStack;
}    // namespace bty

struct GameOptions;

class GameControls : public bty::Scene {
public:
    GameControls(bty::SceneStack &ss, GameOptions &game_options);
    void key(int key, int action) override;
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void set_color(bty::BoxColor color);
    void set_battle(bool battle);

private:
    bty::SceneStack &ss;
    GameOptions &game_options;
    bty::Dialog dialog;
    bty::Option *t_music;
    bty::Option *t_sound;
    bty::Option *t_delay;

    void update_options();
};

#endif    // GAME_GAME_CONTROLS_HPP_
