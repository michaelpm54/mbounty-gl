#include "engine.hpp"
#include "window.hpp"
#include "scene-switcher.hpp"
#include "intro/intro.hpp"
#include "game/game.hpp"
#include "scene-id.hpp"

#include <spdlog/spdlog.h>

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *message,
                            const void *userParam);

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    spdlog::default_logger()->set_level(spdlog::level::debug);

    bty::Window *window = bty::window_init();
    if (!window) {
        return 1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    {
        bty::Assets assets;
        bty::SceneSwitcher scene_switcher(window_width(window), window_height(window), assets);
        bty::Engine engine(*window, scene_switcher);

        Intro intro(scene_switcher);
        Game game(scene_switcher);

        scene_switcher.add_scene(SceneId::Intro, intro);
        scene_switcher.add_scene(SceneId::Game, game);

        if (scene_switcher.set_scene(SceneId::Intro))
            engine.run();
    }

    window_free(window);

    return 0;
}

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char *msg,
                            const void *user_param)
{
    (void)length;
    (void)user_param;

    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::string message = "Debug message (" + std::to_string(id) + "):\n" + msg + '\n';

    switch (source) {
        case GL_DEBUG_SOURCE_API:
            message += "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            message += "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            message += "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            message += "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            message += "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            message += "Source: Other";
            break;
    };

    message += '\n';

    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            message += "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            message += "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            message += "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            message += "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            message += "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            message += "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            message += "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            message += "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            message += "Type: Other";
            break;
    }

    message += '\n';

    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            message += "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            message += "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            message += "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            message += "Severity: notification";
            break;
    }

    spdlog::debug("{}\n", message);
}
