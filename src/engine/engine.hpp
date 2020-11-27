#ifndef BTY_ENGINE_ENGINE_HPP_
#define BTY_ENGINE_ENGINE_HPP_

#include <memory>

#include "input/window-engine-interface.hpp"

namespace bty {

struct Gfx;
struct Scene;
struct Window;
class Assets;

struct Engine {
    std::unique_ptr<Assets> assets;
    input::InputHandler input;
    Window *window {nullptr};
    Gfx *gfx;
    Scene *scene;
    bool run {true};
};

Engine *engine_init(Window *window);
void engine_free(Engine *engine);
void engine_run(Engine *engine);
void engine_key(Engine *engine, int key, int scancode, int action, int mods);
void engine_quit(Engine *engine);
bool engine_set_scene(Engine *engine, Scene *scene);

}    // namespace bty

#endif    // BTY_ENGINE_ENGINE_HPP_
