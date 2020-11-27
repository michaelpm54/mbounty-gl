#ifndef BTY_WINDOW_WINDOW_HPP_
#define BTY_WINDOW_WINDOW_HPP_

#include <GLFW/glfw3.h>

struct GLFWwindow;

namespace bty {

struct Window {
    GLFWwindow *handle;
};

namespace input {
struct InputHandler;
}

Window *window_init();
void window_free(Window *window);
void window_events(Window *window);
void window_init_callbacks(Window *window, input::InputHandler *input);
void window_swap(Window *window);

}    // namespace bty

#endif    // BTY_WINDOW_WINDOW_HPP_
