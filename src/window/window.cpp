/* clang-format off */
#include <GL/glew.h>
/* clang-format on */

#include "window/window.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "input/window-engine-interface.hpp"
#include "window/error.hpp"

namespace bty {

Window *window_init()
{
    glfwSetErrorCallback(window_error);

    if (glfwInit() == GLFW_FALSE) {
        return nullptr;
    }

    Window *window = new Window();

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

    glfwMakeContextCurrent(window->handle);

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

}    // namespace bty
