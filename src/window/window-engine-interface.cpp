#include "window/window-engine-interface.hpp"

#include "engine/engine.hpp"
#include "engine/events.hpp"

namespace bty {

void key(GLFWwindow* window, int key, int, int action, int)
{
    Event event;
    event.key = static_cast<Key>(key);

    switch (action) {
        case GLFW_PRESS:
            event.id = EventId::KeyDown;
            break;
        case GLFW_RELEASE:
            event.id = EventId::KeyUp;
            break;
        default:
            return;
    }

    static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine->event(event);
}

void close(GLFWwindow* window)
{
    Event event;
    event.id = EventId::Quit;

    static_cast<InputHandler*>(glfwGetWindowUserPointer(window))->engine->event(event);
}

}    // namespace bty
