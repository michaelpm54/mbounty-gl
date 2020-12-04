#ifndef BTY_ENGINE_HPP_
#define BTY_ENGINE_HPP_

#include <memory>

#include "gfx/gfx.hpp"
#include "window-engine-interface.hpp"

namespace bty {

class Scene;
struct Window;
class Assets;
class SceneSwitcher;

class Engine {
public:
    Engine(Window &window, SceneSwitcher &scene_switcher);
    void run();
    void key(int key, int scancode, int action, int mods);
    void quit();

private:
    std::unique_ptr<Gfx> gfx_;
    input::InputHandler input_;
    Window *window_ {nullptr};
    SceneSwitcher *scene_switcher_;
    bool run_ {true};
};

}    // namespace bty

#endif    // BTY_ENGINE_HPP_
