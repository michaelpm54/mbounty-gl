#include "window-engine-interface.hpp"

#include "engine.hpp"

#include <GLFW/glfw3.h>

namespace bty::input {

void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine->key(key, scancode, action, mods);
}

void close(GLFWwindow* window)
{
    static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine->quit();
}

}    // namespace bty::input