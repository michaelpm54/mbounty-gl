#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "scene-switcher.hpp"
#include "window.hpp"

namespace bty {

Engine::Engine(Window &window, SceneSwitcher &scene_switcher)
    : gfx_(std::make_unique<Gfx>())
    , input_({.engine = this})
    , window_(&window)
    , scene_switcher_(&scene_switcher)
{
    window_init_callbacks(window_, &input_);
}

void Engine::run()
{
    float dt = 1.0f / 60.0f;
    while (run_) {
        window_events(window_);
        scene_switcher_->update(dt);
        gfx_->clear();
        scene_switcher_->draw(*gfx_);
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
            scene_switcher_->key(key, scancode, action, mods);
            break;
    }
}

void Engine::quit()
{
    run_ = false;
}

}    // namespace bty
