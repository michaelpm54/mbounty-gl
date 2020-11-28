/* clang-format off */
#include "assets.hpp"
/* clang-format on */

#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "scene.hpp"
#include "window.hpp"

namespace bty {

Engine::Engine(Window &window)
    : assets_(std::make_unique<Assets>())
    , gfx_(std::make_unique<Gfx>())
    , input_({.engine = this})
    , window_(&window)
{
    window_init_callbacks(window_, &input_);
}

void Engine::run()
{
    while (run_) {
        window_events(window_);
        gfx_->clear();
        scene_->draw(*gfx_);
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
            scene_->key(key, scancode, action, mods);
            break;
    }
}

void Engine::quit()
{
    run_ = false;
}

bool Engine::set_scene(Scene *scene)
{
    if (!scene->loaded()) {
        if (!scene->load(*assets_)) {
            spdlog::error("Scene failed to load");
            return false;
        }
    }

    scene_ = scene;
    return true;
}

}    // namespace bty
