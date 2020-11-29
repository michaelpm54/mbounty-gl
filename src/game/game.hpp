#ifndef BTY_GAME_GAME_HPP_
#define BTY_GAME_GAME_HPP_

#include "scene.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "gfx/dialog.hpp"

namespace bty {
class Assets;
struct Gfx;
class SceneSwitcher;
}    // namespace bty


class Game : public bty::Scene {
public:
    Game(bty::SceneSwitcher &scene_switcher);
    bool load(bty::Assets &assets) override;
    void draw(bty::Gfx &gfx) override;
    void key(int key, int scancode, int action, int mods) override;
    bool loaded() override;
    void update(float dt) override;

private:
    enum class GameState {
        Unpaused,
        Paused,
    };
    
    bty::SceneSwitcher *scene_switcher_;
    GameState state_{GameState::Unpaused};
    bool loaded_ {false};
    bty::Font font_;
    bty::Sprite hud_;
    glm::mat4 camera_ {1.0f};
};

#endif    // BTY_GAME_GAME_HPP_
