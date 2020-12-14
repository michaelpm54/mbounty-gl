#include "window.hpp"

#include <spdlog/spdlog.h>

#include "window-engine-interface.hpp"

namespace bty {

void window_center(Window *window)
{
    auto *monitor = glfwGetPrimaryMonitor();

    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    if (!mode)
        return;

    int monitor_x, monitor_y;
    glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);

    int window_width, window_height;
    glfwGetWindowSize(window->handle, &window_width, &window_height);

    glfwSetWindowPos(window->handle,
                     monitor_x + (mode->width - window_width) / 2,
                     monitor_y + (mode->height - window_height) / 2 - 125);
}

void window_error(int error_code, const char *description)
{
    (void)error_code;
    spdlog::error("glfw: {}", description);
}

Window *window_init()
{
    glfwSetErrorCallback(window_error);

    if (glfwInit() == GLFW_FALSE) {
        return nullptr;
    }

    Window *window = new Window();

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    window->handle = glfwCreateWindow(320 * 3, 224 * 3, "Bounty", nullptr, nullptr);
    if (!window->handle) {
        glfwTerminate();
        return nullptr;
    }

    window_center(window);

    glfwMakeContextCurrent(window->handle);

    glfwSwapInterval(1);

    spdlog::info("Window created with OpenGL version {}", glGetString(GL_VERSION));

    glewExperimental = GL_TRUE;
    auto err = glewInit();
    if (err != GLEW_OK) {
        spdlog::error("glewInit failed: {}", glewGetErrorString(err));
        glfwTerminate();
        return nullptr;
    }

    return window;
}

void window_free(Window *window)
{
    if (window) {
        glfwDestroyWindow(window->handle);
        delete window;
    }
    glfwTerminate();
}

void window_events(Window *window)
{
    (void)window;
    glfwPollEvents();
}

void window_init_callbacks(Window *window, input::InputHandler *input)
{
    glfwSetWindowUserPointer(window->handle, input);
    glfwSetKeyCallback(window->handle, bty::input::key);
    glfwSetWindowCloseCallback(window->handle, bty::input::close);
}

void window_swap(Window *window)
{
    glfwSwapBuffers(window->handle);
}

int window_width(Window *window)
{
    int w, h;
    glfwGetWindowSize(window->handle, &w, &h);
    return w;
}

int window_height(Window *window)
{
    int w, h;
    glfwGetWindowSize(window->handle, &w, &h);
    return h;
}

}    // namespace bty
