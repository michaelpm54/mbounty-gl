#ifndef BTY_SCENE_SWITCHER_HPP_
#define BTY_SCENE_SWITCHER_HPP_

#include <chrono>

#include "gfx/rect.hpp"
#include "scene-id.hpp"
#include "shared-state.hpp"
#include "window.hpp"

namespace bty {

class Assets;
class Gfx;
class Scene;

class SceneSwitcher {
public:
    SceneSwitcher(Window *window, Assets &assets);
    void update(float dt);
    void draw(Gfx &gfx);
    void fade_to(SceneId id, bool reset);
    void add_scene(SceneId id, Scene &scene);
    bool set_scene(SceneId id);
    SharedState &state();
    void key(int key, int scancode, int action, int mods);
    bool get_key(int key) const;

private:
    void update_fade_out(float dt);
    void update_fade_in(float dt);

private:
    enum class SwitchState {
        FadingOut,
        FadingIn,
        None,
    };

    glm::mat4 camera_ {1.0f};
    bty::Assets *assets_ {nullptr};
    std::unordered_map<SceneId, Scene *> scene_map_;
    SharedState shared_state_;
    SwitchState state_ {SwitchState::None};
    Scene *scene_ {nullptr};
    Scene *next_scene_ {nullptr};
    Rect fade_rect_;
    float fade_elapsed_ {0};
    float fade_alpha_ {0};
    Window *window_ {nullptr};
};

}    // namespace bty

#endif    // BTY_SCENE_SWITCHER_HPP_
