#include "battle/battle.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

Battle::Battle(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Battle::load(bty::Assets &assets)
{
    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    text_.create(10, 10, "Welcome to battle!!!", assets.get_font());

    loaded_ = true;
    return true;
}

void Battle::draw(bty::Gfx &gfx)
{
    gfx.draw_text(text_, camera_);
}

void Battle::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_LEFT:
                    break;
                case GLFW_KEY_RIGHT:
                    break;
                case GLFW_KEY_UP:
                    break;
                case GLFW_KEY_DOWN:
                    break;
                case GLFW_KEY_ENTER:
                    scene_switcher_->fade_to(SceneId::Game, false);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

bool Battle::loaded()
{
    return loaded_;
}

void Battle::update(float dt)
{
}

void Battle::enter(bool reset)
{
}
