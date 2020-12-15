#ifndef BTY_WINDOW_WINDOW_ENGINE_INTERFACE_HPP_
#define BTY_WINDOW_WINDOW_ENGINE_INTERFACE_HPP_

#include "glfw.hpp"

namespace bty {

class Engine;

void key(GLFWwindow *window, int key, int scancode, int action, int mods);
void close(GLFWwindow *window);

/* GLFW -> static key callback -> InputHandler.key -> Engine.key */
struct InputHandler {
    Engine *engine;
};

}    // namespace bty

#endif    // BTY_WINDOW_WINDOW_ENGINE_INTERFACE_HPP_
