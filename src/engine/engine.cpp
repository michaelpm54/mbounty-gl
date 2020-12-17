#include "engine.hpp"

#include <spdlog/spdlog.h>

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

#include "game/ingame.hpp"
#include "game/intro.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"
#include "window/window.hpp"

namespace bty {

Engine::Engine(Window &window, Assets &assets)
    : input_({.engine = this})
    , window_(&window)
    , assets(assets)
    , gfx_(std::make_unique<Gfx>(assets))
    , dialog_stack(assets)
    , hud(assets)
    , view(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
{
    window_init_callbacks(window_, &input_);
    fps_label.create(1, 3, "FPS: ", assets.get_font());
    fps.create(5, 3, "", assets.get_font());
    frame_time.create(1, 4, "", assets.get_font());
}

void Engine::run()
{
    using namespace std::chrono;

    Ingame ingame(window_->handle, scene_stack, dialog_stack, assets, hud, game_options);
    Intro intro(scene_stack, dialog_stack, assets, ingame);

    scene_stack.push(&intro, nullptr);

    static auto time_now = steady_clock::now();
    static int frame_count = 0;
    static int frame_rate = 0;

    while (run_) {
        auto prev_time = time_now;
        time_now = steady_clock::now();
        ++frame_count;
        if (time_point_cast<seconds>(time_now) != time_point_cast<seconds>(prev_time)) {
            frame_rate = frame_count;
            frame_count = 0;
        }

        float dt = duration<float>(time_now - prev_time).count();

        window_events(window_);
        hud.update(dt);
        scene_stack.update(dt);
        dialog_stack.update(dt);

        if (debug) {
            fps.set_string(std::to_string(frame_rate));
            frame_time.set_string(std::to_string(dt));
        }

        gfx_->clear();
        scene_stack.draw(*gfx_, view);
        dialog_stack.draw(*gfx_, view);

        if (debug) {
            gfx_->draw_text(fps_label, view);
            gfx_->draw_text(fps, view);
            gfx_->draw_text(frame_time, view);
        }

        window_swap(window_);
    }
}

void Engine::key(int key, int action)
{
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        debug = !debug;
    }

    switch (key) {
        case GLFW_KEY_Q:
            quit();
            break;
        default:
            if (hud.get_error()) {
                if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
                    hud.clear_error();
                    return;
                }
            }

            if (!dialog_stack.key(key, action)) {
                scene_stack.key(key, action);
            }
            break;
    }
}

void Engine::quit()
{
    run_ = false;
}

}    // namespace bty
