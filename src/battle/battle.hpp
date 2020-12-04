#ifndef BTY_BATTLE_BATTLE_HPP_
#define BTY_BATTLE_BATTLE_HPP_

#include "gfx/dialog.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "scene.hpp"

namespace bty {
class Assets;
class Gfx;
class SceneSwitcher;
}    // namespace bty

class Battle : public bty::Scene {
public:
    Battle(bty::SceneSwitcher &scene_switcher);
    bool load(bty::Assets &assets) override;
    void draw(bty::Gfx &gfx) override;
    void key(int key, int scancode, int action, int mods) override;
    bool loaded() override;
    void update(float dt) override;
    void enter(bool reset) override;

private:
    enum class BattleState {
        Moving,
    };

    bool loaded_ {false};
    bty::SceneSwitcher *scene_switcher_;
    BattleState state_ {BattleState::Moving};
    bty::Text text_;
    glm::mat4 camera_ {1.0f};
};

#endif    // BTY_INTRO_BATTLE_HPP_
