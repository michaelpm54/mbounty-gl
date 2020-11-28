#ifndef BTY_ENGINE_HPP_
#define BTY_ENGINE_HPP_

#include <memory>

#include "gfx/gfx.hpp"
#include "window-engine-interface.hpp"

namespace bty {

struct Scene;
struct Window;
class Assets;

class Engine {
public:
    Engine(Window &window);
    void run();
    void key(int key, int scancode, int action, int mods);
    void quit();
    bool set_scene(Scene *scene);

private:
    std::unique_ptr<Assets> assets_;
    std::unique_ptr<Gfx> gfx_;
    input::InputHandler input_;
    Window *window_ {nullptr};
    Scene *scene_;
    bool run_ {true};
};

}    // namespace bty

#endif    // BTY_ENGINE_HPP_
