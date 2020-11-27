#include "input/window-engine-interface.hpp"

#include <GLFW/glfw3.h>

#include "engine/engine.hpp"

namespace bty::input {

void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    engine_key(static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine, key, scancode, action, mods);
}

void close(GLFWwindow* window)
{
    engine_quit(static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine);
}

}    // namespace bty::input
