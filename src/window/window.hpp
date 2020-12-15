#ifndef BTY_WINDOW_WINDOW_HPP_
#define BTY_WINDOW_WINDOW_HPP_

#include "window/glfw.hpp"

namespace bty {

struct Window {
    GLFWwindow *handle;
};

struct InputHandler;

Window *window_init();
void window_free(Window *window);
void window_events(Window *window);
void window_init_callbacks(Window *window, InputHandler *input);
void window_swap(Window *window);
int window_width(Window *window);
int window_height(Window *window);

}    // namespace bty

#endif    // BTY_WINDOW_WINDOW_HPP_
