#include "window/window-engine-interface.hpp"

#include "engine/engine.hpp"

namespace bty {

void key(GLFWwindow* window, int key, int, int action, int)
{
    static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine->key(key, action);
}

void close(GLFWwindow* window)
{
    static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine->quit();
}

}    // namespace bty
