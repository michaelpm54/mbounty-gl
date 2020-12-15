#ifndef BTY_ENGINE_ENGINE_HPP_
#define BTY_ENGINE_ENGINE_HPP_

#include <memory>

#include "game/game.hpp"
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
    void key(int key, int scancode, int action, int mods);
    void quit();

private:
    Game game;
    std::unique_ptr<Gfx> gfx_;
    InputHandler input_;
    Window *window_ {nullptr};
    bool run_ {true};
};

}    // namespace bty

#endif    // BTY_ENGINE_ENGINE_HPP_
