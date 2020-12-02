#include "scene-switcher.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "scene.hpp"
#include "gfx/gfx.hpp"

namespace bty {

SceneSwitcher::SceneSwitcher(int window_width, int window_height, Assets &assets)
    : camera_(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
    , assets_(&assets)
{
    fade_rect_.set_size(window_width, window_height);
}

void SceneSwitcher::fade_to(SceneId id)
{
    if (!scene_map_.contains(id)) {
        spdlog::warn("SceneSwitcher::fade_to: No scene by ID {}", id);
    }
    else {
        if (!scene_map_[id]->loaded()) {
            if (!scene_map_[id]->load(*assets_)) {
                spdlog::warn("SceneSwitcher::set_scene: Loading scene ID {} failed", id);
            }
        }
        next_scene_ = scene_map_[id];
        next_scene_->enter();
    }

    fade_alpha_ = 0;
    state_ = SwitchState::FadingOut;
}

void SceneSwitcher::update(float dt) {
    if (!scene_) {
        return;
    }

    switch (state_) {
        case SwitchState::FadingOut:
            update_fade_out(dt);
            break;
        case SwitchState::FadingIn:
            update_fade_in(dt);
            break;
        default:
            break;
    }

    scene_->update(dt);
}

void SceneSwitcher::draw(Gfx &gfx) {
    if (!scene_) {
        return;
    }

    switch (state_) {
        case SwitchState::FadingOut:
            scene_->draw(gfx);
            gfx.draw_rect(fade_rect_, camera_);
            break;
        case SwitchState::FadingIn:
            scene_->draw(gfx);
            gfx.draw_rect(fade_rect_, camera_);
            break;
        default:
            scene_->draw(gfx);
            break;
    }
}

static constexpr float kFadeTimeS { 1 };

void SceneSwitcher::update_fade_out(float dt) {
    fade_elapsed_ += dt;

    fade_alpha_ = fade_elapsed_ / kFadeTimeS;

    if (fade_elapsed_ >= kFadeTimeS) {
        state_ = SwitchState::FadingIn;

        if (next_scene_) {
            scene_ = next_scene_;
            next_scene_ = nullptr;
        }

        fade_alpha_ = 1;
        fade_elapsed_ = 0;
    }

    fade_rect_.set_color({0.0f, 0.0f, 0.0f, fade_alpha_});
}

void SceneSwitcher::update_fade_in(float dt) {
    fade_elapsed_ += dt;

    fade_alpha_ = 1.0f - fade_elapsed_ / kFadeTimeS;

    if (fade_elapsed_ >= kFadeTimeS) {
        state_ = SwitchState::None;
        fade_elapsed_ = 0;
        fade_alpha_ = 0;
    }

    fade_rect_.set_color({0.0f, 0.0f, 0.0f, fade_alpha_});
}

void SceneSwitcher::add_scene(SceneId id, Scene &scene)
{
    scene_map_[id] = &scene;
}

bool SceneSwitcher::set_scene(SceneId id)
{
    if (!scene_map_.contains(id)) {
        spdlog::warn("SceneSwitcher::set_scene: No scene by ID {}", id);
        return false;
    }

    if (!scene_map_[id]->loaded()) {
        if (!scene_map_[id]->load(*assets_)) {
            spdlog::warn("SceneSwitcher::set_scene: Loading scene ID {} failed", id);
            return false;
        }
    }

    scene_ = scene_map_[id];
    scene_->enter();

    return true;
}

SharedState &SceneSwitcher::state() {
    return shared_state_;
}

void SceneSwitcher::key(int key, int scancode, int action, int mods)
{
    scene_->key(key, scancode, action, mods);
}

}
