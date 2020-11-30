#include "game/game.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "assets.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

#include "gfx/gfx.hpp"

Game::Game(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Game::load(bty::Assets &assets)
{
    bool success {true};

    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    std::array<const bty::Texture *, 8> border_textures;

    for (int i = 0; i < 8; i++) {
        std::string filename = fmt::format("border-normal/box{}.png", i);
        border_textures[i] = assets.get_texture(filename);
    }

    int difficulty = scene_switcher_->state().difficulty_level;

    scene_switcher_->state().days = kDays[difficulty];

    font_.load_from_texture(assets.get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});
    hud_.load(assets, font_, scene_switcher_->state());

    pause_menu_.create(
		3, 7,
		26, 16,
		bty::get_box_color(difficulty),
		border_textures,
		font_,
		assets.get_texture("arrow.png", {2, 2})
	);
    pause_menu_.add_option(3, 2, "View your army");
    pause_menu_.add_option(3, 3, "View your character");
    pause_menu_.add_option(3, 4, "Look at continent map");
    pause_menu_.add_option(3, 5, "Use magic");
    pause_menu_.add_option(3, 6, "Contract information");
    pause_menu_.add_option(3, 7, "Wait to end of week");
    pause_menu_.add_option(3, 8, "Look at puzzle pieces");
    pause_menu_.add_option(3, 9, "Search the area");
    pause_menu_.add_option(3, 10, "Dismiss army");
    pause_menu_.add_option(3, 11, "Game controls");
    pause_menu_.add_option(3, 13, "Get password");

    add_unit_to_army(12, 10);
    add_unit_to_army(14, 5);
    add_unit_to_army(0, 40);

    auto color = bty::get_box_color(difficulty);
    view_army_.load(assets, color, font_);
    view_character_.load(assets, color, font_, scene_switcher_->state().hero_id);
    view_continent_.load(assets, color, font_, border_textures);

    map_.load(assets);
    hero_.load(assets);

    hero_.move_to_tile(map_.get_tile(11, 57));
    update_camera();

    loaded_ = true;
    return success;
}

void Game::update_camera()
{
    glm::vec2 cam_centre = hero_.get_center();
    camera_pos_ = {cam_centre.x - 140, cam_centre.y - 120, 0.0f};
    game_camera_ = camera_ * glm::translate(-camera_pos_);
    // game_camera_ = zoom_;
    // game_camera_ = camera_;
}

void Game::draw(bty::Gfx &gfx)
{
    switch (state_) {
        case GameState::Unpaused:
            map_.draw(game_camera_);
            gfx.draw_sprite(hero_, game_camera_);
            hud_.draw(gfx, camera_);
            break;
        case GameState::Paused:
            map_.draw(game_camera_);
            gfx.draw_sprite(hero_, game_camera_);
            hud_.draw(gfx, camera_);
            pause_menu_.draw(gfx, camera_);
            break;
        case GameState::ViewArmy:
            hud_.draw(gfx, camera_);
            view_army_.draw(gfx, camera_);
            break;
        case GameState::ViewCharacter:
            hud_.draw(gfx, camera_);
            view_character_.draw(gfx, camera_);
            break;
        case GameState::ViewContinent:
            map_.draw(game_camera_);
            hud_.draw(gfx, camera_);
            view_continent_.draw(gfx, camera_);
            break;
        default:
            break;
    }
}

void Game::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;
    
    switch (state_) {
        case GameState::Unpaused:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_SPACE:
                            state_ = GameState::Paused;
                            move_flags_ = MOVE_FLAGS_NONE;
                            break;
                        case GLFW_KEY_B:
                            hero_.set_mount(hero_.get_mount() == Mount::Walk ? Mount::Boat : Mount::Walk);
                            break;
                        case GLFW_KEY_M:
                            scene_switcher_->state().siege = !scene_switcher_->state().siege;
                            scene_switcher_->state().magic = !scene_switcher_->state().magic;
                            hud_.update_state();
                            break;
                        case GLFW_KEY_C:
                            scene_switcher_->state().contract = (scene_switcher_->state().contract + 1) % 18;
                            hud_.update_state();
                            break;
                        case GLFW_KEY_LEFT:
                            move_flags_ |= MOVE_FLAGS_LEFT;
                            break;
                        case GLFW_KEY_RIGHT:
                            move_flags_ |= MOVE_FLAGS_RIGHT;
                            break;
                        case GLFW_KEY_UP:
                            move_flags_ |= MOVE_FLAGS_UP;
                            break;
                        case GLFW_KEY_DOWN:
                            move_flags_ |= MOVE_FLAGS_DOWN;
                            break;
                        default:
                            break;
                    }
                    break;
                case GLFW_RELEASE:
                    switch (key) {
                        case GLFW_KEY_LEFT:
                            move_flags_ &= ~MOVE_FLAGS_LEFT;
                            break;
                        case GLFW_KEY_RIGHT:
                            move_flags_ &= ~MOVE_FLAGS_RIGHT;
                            break;
                        case GLFW_KEY_UP:
                            move_flags_ &= ~MOVE_FLAGS_UP;
                            break;
                        case GLFW_KEY_DOWN:
                            move_flags_ &= ~MOVE_FLAGS_DOWN;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::Paused:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_BACKSPACE: [[fallthrough]];
                        case GLFW_KEY_SPACE:
                            state_ = GameState::Unpaused;
                            break;
                        case GLFW_KEY_UP:
                            pause_menu_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            pause_menu_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            switch (pause_menu_.get_selection()) {
                                case 0:
                                    state_ = GameState::ViewArmy;
                                    view_army_.view(scene_switcher_->state());
                                    break;
                                case 1:
                                    state_ = GameState::ViewCharacter;
                                    view_character_.view(scene_switcher_->state());
                                    break;
                                case 2:
                                    state_ = GameState::ViewContinent;
                                    view_continent_.view(scene_switcher_->state());
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::ViewArmy: [[fallthrough]];
        case GameState::ViewCharacter: [[fallthrough]];
        case GameState::ViewContinent:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_SPACE: [[fallthrough]];
                        case GLFW_KEY_BACKSPACE: [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            state_ = GameState::Unpaused;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

bool Game::loaded()
{
    return loaded_;
}

void Game::collide(Tile &tile) {
    switch (tile.id) {
        default:
            break;
    }
}

void Game::update(float dt)
{
    if (state_ == GameState::Unpaused) {
        if (move_flags_) {
            hero_.set_moving(true);

            if ((move_flags_ & MOVE_FLAGS_LEFT) && !(move_flags_ & MOVE_FLAGS_RIGHT)) {
                hero_.set_flip(true);
            }
            else if ((move_flags_ & MOVE_FLAGS_RIGHT) && !(move_flags_ & MOVE_FLAGS_LEFT)) {
                hero_.set_flip(false);
            }

            glm::vec2 dir{0.0f};

            if (move_flags_ & MOVE_FLAGS_UP)
                dir.y -= 1.0f;
            if (move_flags_ & MOVE_FLAGS_DOWN)
                dir.y += 1.0f;
            if (move_flags_ & MOVE_FLAGS_LEFT)
                dir.x -= 1.0f;
            if (move_flags_ & MOVE_FLAGS_RIGHT)
                dir.x += 1.0f;

            static constexpr float speed = 100.0f;
            float vel = speed * dt;
            float dx = dir.x * vel;
            float dy = dir.y * vel;

            auto manifold = hero_.move(dx, dy, map_);

            hero_.set_position(manifold.new_position);

            if (manifold.collided) {
                for (auto &tile : manifold.collided_tiles) {
                    collide(tile);
                }
            }
            else {
                if (manifold.changed_tile) {
                    hero_.set_tile_info(manifold.new_tile);
                    if (hero_.get_mount() == Mount::Boat && manifold.new_tile.id == Grass) {
                        hero_.set_mount(Mount::Walk);
                    }
                }
            }

            update_camera();
        }
        else {
            hero_.set_moving(false);
        }
        map_.update(dt);
        hero_.animate(dt);
    }
    if (state_ == GameState::Paused || state_ == GameState::Unpaused || state_ == GameState::ViewContinent) {
        hud_.update(dt);
        pause_menu_.animate(dt);
    }
    else if (state_ == GameState::ViewArmy) {
        view_army_.update(dt);
    }
    else if (state_ == GameState::ViewContinent) {
        view_continent_.update(dt);
    }
}

/*
 *  bounty.c -- tables and static data needed in the game
 *  Copyright (C) 2011 Vitaly Driedfruit
 *
 *  This file is part of openkb.
 *
 *  openkb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openkb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openkb.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MORALE_NORMAL 0
#define MORALE_LOW 1
#define MORALE_HIGH 2

#define _N MORALE_NORMAL
#define _L MORALE_LOW
#define _H MORALE_HIGH

static char kMoraleChart[5][5] = {
    /*	 A	 B	 C	 D	 E	 */
    /* A */ {_N, _N, _N, _N, _N},
    /* B */ {_N, _N, _N, _N, _N},
    /* C */ {_N, _N, _H, _N, _N},
    /* D */ {_L, _N, _L, _H, _N},
    /* E */ {_L, _L, _L, _N, _N},
};

#undef _N
#undef _L
#undef _H

void Game::add_unit_to_army(int id, int count) {
    if (id < 0 || id >= 25) {
        spdlog::warn("Game::add_unit_to_army: id out of range: {}", id);
        return;
    }

    if (scene_switcher_->state().army_size == 5) {
        spdlog::warn("Game::add_unit_to_army: army already full");
        return;
    }

    int index = scene_switcher_->state().army_size++;

    int *army = scene_switcher_->state().army;
    int *army_counts = scene_switcher_->state().army_counts;
    int *army_morales = scene_switcher_->state().army_morales;

    army[index] = id;
    army_counts[index] = count;

    for (int i = 0; i < 5; i++) {
        const auto &unit = kUnits[id];

        if (army_counts[count] * unit.hp > scene_switcher_->state().leadership) {
            army_morales[i] = 3;
            continue;
        }

        /* Morale regresses to how the other guy feels about me */
        char morale_cnv[3] = {MORALE_LOW, MORALE_NORMAL, MORALE_HIGH};
        char my_morale = MORALE_HIGH;
        for (int j = 0; j < 5; j++)
        {
            if (army_counts[j] == 0)
                break;
            char other_group = kUnits[army[j]].morale_group;
            char other_morale = kMoraleChart[other_group][unit.morale_group];
            if (morale_cnv[other_morale] < morale_cnv[my_morale])
                my_morale = other_morale;
        }
        
        scene_switcher_->state().army_morales[i] = my_morale;
    }
}
