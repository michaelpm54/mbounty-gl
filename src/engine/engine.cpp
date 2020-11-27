/* clang-format off */
#include "assets.hpp"
/* clang-format on */

#include "engine/engine.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "scene/scene.hpp"
#include "window/window.hpp"

namespace bty {

Engine *engine_init(Window *window)
{
    Engine *engine = new Engine();

    engine->assets = std::make_unique<Assets>();

    engine->input.engine = engine;

    engine->window = window;
    window_init_callbacks(window, &engine->input);

    engine->gfx = gfx_init();
    if (!engine->gfx) {
        spdlog::error("Engine init failed");
        return nullptr;
    }

    return engine;
}

void engine_free(Engine *engine)
{
    gfx_free(engine->gfx);
    delete engine;
}

void engine_run(Engine *engine)
{
    while (engine->run) {
        window_events(engine->window);
        gfx_clear(engine->gfx);
        engine->scene->draw(*engine->gfx);
        window_swap(engine->window);
    }
}

void engine_key(Engine *engine, int key, int scancode, int action, int mods)
{
    switch (key) {
        case GLFW_KEY_Q:
            engine_quit(engine);
            break;
        default:
            engine->scene->key(key, scancode, action, mods);
            break;
    }
}

void engine_quit(Engine *engine)
{
    engine->run = false;
}

bool engine_set_scene(Engine *engine, Scene *scene)
{
    if (!scene->loaded()) {
        if (!scene->load(*engine->assets)) {
            spdlog::error("Scene failed to load");
            return false;
        }
    }

    engine->scene = scene;
    return true;
}

}    // namespace bty
