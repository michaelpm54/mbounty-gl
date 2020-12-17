#ifndef BTY_ENGINE_ENGINE_HPP_
#define BTY_ENGINE_ENGINE_HPP_

#include <memory>

#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "game/game-options.hpp"
#include "game/hud.hpp"
#include "gfx/gfx.hpp"
#include "window/window-engine-interface.hpp"

namespace bty {

class Scene;
struct Window;
class Assets;

class Engine {
public:
    Engine(Window &window, Assets &assets);
    void run();
    void key(int key, int action);
    void quit();

private:
    void intro_pop(int ret);

private:
    InputHandler input_;
    Window *window_ {nullptr};
    Assets &assets;
    std::unique_ptr<Gfx> gfx_;
    SceneStack scene_stack;
    DialogStack dialog_stack;
    Hud hud;
    glm::mat4 view;
    bool run_ {true};

    bool debug {false};
    Text fps_label;
    Text fps;
    Text frame_time;

    GameOptions game_options;
};

}    // namespace bty

#endif    // BTY_ENGINE_ENGINE_HPP_
