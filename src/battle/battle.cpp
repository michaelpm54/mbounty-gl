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

    auto &state = scene_switcher_->state();

    auto color = bty::get_box_color(state.difficulty_level);

    bg_.set_texture(assets.get_texture("battle/encounter.png"));
    bg_.set_position(8, 24);
    frame_.set_texture(assets.get_texture("frame/game-empty.png"));
    bar_.set_color(color);
    bar_.set_size(304, 9);
    bar_.set_position(8, 7);

    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    loaded_ = true;
    return true;
}

void Battle::draw(bty::Gfx &gfx)
{
    gfx.draw_sprite(bg_, camera_);
    gfx.draw_sprite(frame_, camera_);
    gfx.draw_rect(bar_, camera_);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
            gfx.draw_sprite(sprites_[i][j], camera_);
        }
    }
}

void Battle::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_LEFT:
                    sprites_[0][0].move(-48.0f, 0.0f);
                    break;
                case GLFW_KEY_RIGHT:
                    sprites_[0][0].move(48.0f, 0.0f);
                    break;
                case GLFW_KEY_UP:
                    sprites_[0][0].move(0.0f, -40.0f);
                    break;
                case GLFW_KEY_DOWN:
                    sprites_[0][0].move(0.0f, 40.0f);
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
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
            sprites_[i][j].animate(dt);
        }
    }
}

void Battle::enter(bool reset)
{
    if (!reset) {
        return;
    }

    auto &state = scene_switcher_->state();

    // state.army[0] = Peasants;
    // state.army[1] = Ghosts;
    // state.army[2] = -1;
    // state.army[3] = -1;
    // state.army[4] = -1;

    // state.enemy_army[0] = Dragons;
    // state.enemy_army[1] = Militias;
    // state.enemy_army[2] = -1;
    // state.enemy_army[3] = -1;
    // state.enemy_army[4] = -1;

    army_sizes_[0] = 0;
    army_sizes_[1] = 0;

    for (int i = 0; i < 5; i++) {
        if (state.army[i] != -1) {
            army_sizes_[0]++;
        }
        if (state.enemy_army[i] != -1) {
            army_sizes_[1]++;
        }
    }

    for (int i = 0; i < army_sizes_[0]; i++) {
        sprites_[0][i].set_position({16, 24 + i * 40});
        sprites_[0][i].set_texture(unit_textures_[state.army[i]]);
    }

    for (int i = 0; i < army_sizes_[1]; i++) {
        sprites_[1][i].set_position({248, 24 + i * 40});
        sprites_[1][i].set_texture(unit_textures_[state.enemy_army[i]]);
        sprites_[1][i].set_flip(true);
    }
}
