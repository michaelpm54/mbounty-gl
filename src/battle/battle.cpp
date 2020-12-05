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
    cursor_.set_texture(assets.get_texture("battle/selection.png", {4, 1}));

    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    auto &font = assets.get_font();

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            hp_[i][j].set_font(font);
        }
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
            gfx.draw_text(hp_[i][j], camera_);
        }
    }

    gfx.draw_sprite(cursor_, camera_);
}

void Battle::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_LEFT:
                    move_cursor(0);
                    break;
                case GLFW_KEY_RIGHT:
                    move_cursor(1);
                    break;
                case GLFW_KEY_UP:
                    move_cursor(2);
                    break;
                case GLFW_KEY_DOWN:
                    move_cursor(3);
                    break;
                case GLFW_KEY_ENTER:
                    confirm();
                    break;
                case GLFW_KEY_BACKSPACE:
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
    cursor_.animate(dt);
}

void Battle::enter(bool reset)
{
    if (!reset) {
        return;
    }

    auto &state = scene_switcher_->state();

    state.army[0] = Peasants;
    state.army[1] = Ghosts;
    state.army[2] = Nomads;
    state.army[3] = -1;
    state.army[4] = -1;

    state.enemy_army[0] = Dragons;
    state.enemy_army[1] = Militias;
    state.enemy_army[2] = -1;
    state.enemy_army[3] = -1;
    state.enemy_army[4] = -1;
    state.enemy_army[5] = -1;

    army_sizes_[0] = 0;
    army_sizes_[1] = 0;

    for (int i = 0; i < 6; i++) {
        if (i < 5 && state.army[i] != -1) {
            army_sizes_[0]++;
        }
        if (state.enemy_army[i] != -1) {
            army_sizes_[1]++;
        }
    }

    static constexpr int kStartingPositionY[2][2][6] = {
        {// Team 0
         {
             // Encounter
             0,
             1,
             2,
             3,
             4,
             -1,
         },
         {
             // Siege
             4,
             4,
             3,
             3,
             3,
             -1,
         }},
        {// Team 1
         {
             // Encounter
             0,
             1,
             2,
             3,
             4,
         },
         {
             // Siege
             1,
             1,
             0,
             0,
             0,
             0,
         }},
    };

    static constexpr int kStartingPositionX[2][2][6] = {
        {// Team 0
         {
             // Encounter
             0,
             0,
             0,
             0,
             0,
             -1,
         },
         {
             // Siege
             2,
             3,
             1,
             2,
             3,
             -1,
         }},
        {// Team 1
         {
             // Encounter
             5,
             5,
             5,
             5,
             5,
         },
         {
             // Siege
             2,
             3,
             1,
             2,
             2,
             3,
         }},
    };

    int type = 0;    // encounter

    for (int i = 0; i < army_sizes_[0]; i++) {
        move_unit_to(0, i, kStartingPositionX[0][type][i], kStartingPositionY[0][type][i]);
        sprites_[0][i].set_texture(unit_textures_[state.army[i]]);
    }

    for (int i = 0; i < army_sizes_[1]; i++) {
        move_unit_to(1, i, kStartingPositionX[1][type][i], kStartingPositionY[1][type][i]);
        sprites_[1][i].set_texture(unit_textures_[state.enemy_army[i]]);
        sprites_[1][i].set_flip(true);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
            hp_[i][j].set_position(sprites_[i][j].get_position() + glm::vec2(24.0f, 26.0f));
            hp_[i][j].set_string(std::to_string(100));
        }
    }

    cx_ = kStartingPositionX[0][type][0];
    cy_ = kStartingPositionY[0][type][0];
    cursor_.set_position(16.0f + cx_ * 48.0f, 24.0f + cy_ * 40.0f);
}

void Battle::move_unit_to(int team, int unit, int x, int y)
{
    if (team > 1) {
        spdlog::warn("Battle::move_unit_to: Team {} is invalid", team);
        return;
    }
    if (unit < 0 || unit > 4) {
        spdlog::warn("Battle::move_unit_to: Unit {} is invalid", unit);
        return;
    }
    if (unit > army_sizes_[team]) {
        spdlog::warn("Battle::move_unit_to: No unit {} in team {}", unit, team);
        return;
    }

    float x_ = 16.0f + x * 48.0f;
    float y_ = 24.0f + y * 40.0f;
    sprites_[team][unit].set_position(x_, y_);
    hp_[team][unit].set_position(x_ + 24.0f, y_ + 26.0f);
}

void Battle::move_cursor(int dir)
{
    switch (dir) {
        case 0:    // left
            if (cx_ == 0) {
                return;
            }
            cx_--;
            break;
        case 1:    // right
            if (cx_ == 5) {
                return;
            }
            cx_++;
            break;
        case 2:    // up
            if (cy_ == 0) {
                return;
            }
            cy_--;
            break;
        case 3:    // down
            if (cy_ == 4) {
                return;
            }
            cy_++;
            break;
        default:
            break;
    }
    cursor_.set_position(16.0f + cx_ * 48.0f, 24.0f + cy_ * 40.0f);
}

void Battle::confirm()
{
    switch (state_) {
        case BattleState::Moving:
            move_confirm();
            break;
        default:
            break;
    }
}

void Battle::move_confirm()
{
    move_unit_to(0, 0, cx_, cy_);
}
