#include "engine.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "game/ingame.hpp"
#include "game/intro.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"
#include "window/window.hpp"

namespace bty {

Engine::Engine(Window &window, Assets &assets)
    : input_({.engine = this})
    , window_(&window)
    , assets(assets)
    , gfx_(std::make_unique<Gfx>(assets))
    , dialog_stack(assets)
    , hud(assets)
    , view(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
{
    window_init_callbacks(window_, &input_);
}

void Engine::run()
{
    Ingame ingame(window_->handle, scene_stack, dialog_stack, assets, hud);
    Intro intro(scene_stack, dialog_stack, assets, ingame);

    scene_stack.push(&intro, nullptr);

    float dt = 1.0f / 60.0f;
    while (run_) {
        window_events(window_);
        hud.update(dt);
        scene_stack.update(dt);
        dialog_stack.update(dt);
        gfx_->clear();
        scene_stack.draw(*gfx_, view);
        dialog_stack.draw(*gfx_, view);
        window_swap(window_);
    }
}

void Engine::key(int key, int action)
{
    switch (key) {
        case GLFW_KEY_Q:
            quit();
            break;
        default:
            if (hud.get_error()) {
                if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
                    hud.clear_error();
                    return;
                }
            }

            if (!dialog_stack.key(key, action)) {
                scene_stack.key(key, action);
            }
            break;
    }
}

void Engine::quit()
{
    run_ = false;
}

}    // namespace bty
