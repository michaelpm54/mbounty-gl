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

    auto &state = scene_switcher_->state();

    for (int i = 0; i < 14; i++) {
        state.spells[i] = 0;
    }

    int difficulty = state.difficulty_level;

    state.days = kDays[difficulty];

    font_.load_from_texture(assets.get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});
    hud_.load(assets, font_, state);

    pause_menu_.create(
		3, 7,
		26, 16,
		bty::get_box_color(difficulty),
        assets
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

    use_magic_.create(
		6, 4,
		20, 22,
		bty::get_box_color(difficulty),
        assets
	);
    use_magic_.add_line(1, 1, "Adventuring Spells");
    magic_spells_[0] = use_magic_.add_option(4, 3, "");
    magic_spells_[1] = use_magic_.add_option(4, 4, "");
    magic_spells_[2] = use_magic_.add_option(4, 5, "");
    magic_spells_[3] = use_magic_.add_option(4, 6, "");
    magic_spells_[4] = use_magic_.add_option(4, 7, "");
    magic_spells_[5] = use_magic_.add_option(4, 8, "");
    magic_spells_[6] = use_magic_.add_option(4, 9, "");
    use_magic_.add_line(3, 12, "Combat Spells");
    magic_spells_[7] = use_magic_.add_option(4, 14, "");
    magic_spells_[8] = use_magic_.add_option(4, 15, "");
    magic_spells_[9] = use_magic_.add_option(4, 16, "");
    magic_spells_[10] = use_magic_.add_option(4, 17, "");
    magic_spells_[11] = use_magic_.add_option(4, 18, "");
    magic_spells_[12] = use_magic_.add_option(4, 19, "");
    magic_spells_[13] = use_magic_.add_option(4, 20, "");

    add_unit_to_army(12, 10);
    add_unit_to_army(14, 5);
    add_unit_to_army(0, 40);

    auto color = bty::get_box_color(difficulty);
    view_army_.load(assets, color);
    view_character_.load(assets, color, state.hero_id);
    view_continent_.load(assets, color);
    view_contract_.load(assets, color);

    map_.load(assets);
    hero_.load(assets);

    hero_.move_to_tile(map_.get_tile(11, 58));
    update_camera();

    state.visited_tiles.resize(4096);
    std::fill(state.visited_tiles.begin(), state.visited_tiles.end(), -1);
    update_visited_tiles();

    loaded_ = true;
    return success;
}

void Game::update_camera()
{
    glm::vec2 cam_centre = hero_.get_center();
    camera_pos_ = {cam_centre.x - 120, cam_centre.y - 120, 0.0f};
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
        case GameState::UseMagic:
            map_.draw(game_camera_);
            hud_.draw(gfx, camera_);
            use_magic_.draw(gfx, camera_);
            break;
        case GameState::ViewContract:
            hud_.draw(gfx, camera_);
            view_contract_.draw(gfx, camera_);
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
                                    view_continent_.view(
                                        hero_.get_tile().tx,
                                        hero_.get_tile().ty,
                                        scene_switcher_->state().continent,
                                        view_continent_fog_ ? scene_switcher_->state().visited_tiles.data() : map_.get_data()
                                    );
                                    break;
                                case 3:
                                    state_ = GameState::UseMagic;
                                    update_spells();
                                    break;
                                case 4:
                                    state_ = GameState::ViewContract;
                                    view_contract_.view(scene_switcher_->state().contract, false, hud_.get_contract());
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
        case GameState::ViewContinent: [[fallthrough]];
        case GameState::ViewContract:
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
        case GameState::UseMagic:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_BACKSPACE:
                            state_ = GameState::Unpaused;
                            hud_.update_state();
                            break;
                        case GLFW_KEY_UP:
                            use_magic_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            use_magic_.next();
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

    if (state_ == GameState::ViewContinent && action == GLFW_PRESS && key == GLFW_KEY_F) {
        view_continent_fog_ = !view_continent_fog_;
        view_continent_.view(
            hero_.get_tile().tx,
            hero_.get_tile().ty,
            scene_switcher_->state().continent,
            view_continent_fog_ ? scene_switcher_->state().visited_tiles.data() : map_.get_data()
        );
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
                    update_visited_tiles();
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
    if (state_ == GameState::Paused
    || state_ == GameState::Unpaused
    || state_ == GameState::ViewContinent
    || state_ == GameState::UseMagic
    || state_ == GameState::ViewContract) {
        hud_.update(dt);
        pause_menu_.animate(dt);
    }
    if (state_ == GameState::ViewArmy) {
        view_army_.update(dt);
    }
    else if (state_ == GameState::ViewContinent) {
        view_continent_.update(dt);
    }
    else if (state_ == GameState::UseMagic) {
        use_magic_.animate(dt);
    }
    else if (state_ == GameState::ViewContract) {
        view_contract_.update(dt);
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

void Game::update_visited_tiles() {
    auto tile = hero_.get_tile();
    auto tile_index = tile.tx + tile.ty * 64;
    auto *visited = scene_switcher_->state().visited_tiles.data();
    auto *tiles = map_.get_data();

    visited[tile_index] = tile.id;
    
    int initial_x = tile.tx;
    int initial_y = tile.ty;

    int start_x = initial_x - 2;
    int start_y = initial_y - 2;
    
    int end_x = initial_x + 2;
    int end_y = initial_y + 2;

    for (int x = start_x; x <= end_x; x++) {
        for (int y = start_y; y <= end_y; y++) {
            int i = x + y * 64;
            if (i < 0 || i > 4095) {
                continue;
            }
            int new_y = i / 64;
            if (new_y != y) {
                continue;
            }
            visited[i] = tiles[i];
        }
    }
}

void Game::update_spells()
{
    bool no_spells = true;
    int *spells = scene_switcher_->state().spells;
    for (int i = 0; i < 14; i++) {
        if (spells[i] == 0) {
            use_magic_.disable_option(i);
        }
        else {
            no_spells = false;
        }
    }

    if (no_spells) {
        hud_.set_title("You have no Adventuring spell to cast!");
    }

    int n = 0;

    magic_spells_[n]->set_string(fmt::format("{} Bridge", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Time Stop", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Find Villain", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Castle Gate", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Town Gate", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Instant Army", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Raise Control", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Clone", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Teleport", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Fireball", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Lightning", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Freeze", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Resurrect", spells[n]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Turn Undead", spells[n]));
}
