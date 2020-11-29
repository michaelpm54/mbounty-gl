#ifndef BTY_GAME_GAME_HPP_
#define BTY_GAME_GAME_HPP_

#include "scene.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "gfx/dialog.hpp"

#include "game/hud.hpp"
#include "game/map.hpp"
#include "game/move-flags.hpp"
#include "game/entity.hpp"

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
    void update_camera();

private:
    enum class GameState {
        Unpaused,
        Paused,
    };
    
    bty::SceneSwitcher *scene_switcher_;
    GameState state_{GameState::Unpaused};
    bool loaded_ {false};
    bty::Font font_;
    glm::mat4 camera_ {1.0f};

    Hud hud_;
    Map map_;

    glm::vec3 camera_pos_{0};
    glm::mat4 game_camera_{1};
    glm::mat4 zoom_{0};

    uint8_t move_flags_{MOVE_FLAGS_NONE};

    Entity hero_;
};

#endif    // BTY_GAME_GAME_HPP_
