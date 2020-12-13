#include "engine.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "window.hpp"

namespace bty {

Engine::Engine(Window &window, Assets &assets)
    : game(window.handle, assets)
    , gfx_(std::make_unique<Gfx>(assets))
    , input_({.engine = this})
    , window_(&window)
{
    window_init_callbacks(window_, &input_);
}

void Engine::run()
{
    float dt = 1.0f / 60.0f;
    while (run_) {
        window_events(window_);
        game.update(dt);
        gfx_->clear();
        game.draw(*gfx_);
        window_swap(window_);
    }
}

void Engine::key(int key, int scancode, int action, int mods)
{
    switch (key) {
        case GLFW_KEY_Q:
            quit();
            break;
        default:
            game.key(key, action);
            break;
    }
}

void Engine::quit()
{
    run_ = false;
}

}    // namespace bty
