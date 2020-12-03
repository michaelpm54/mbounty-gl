#include "game/game.hpp"

#include <algorithm>
#include <chrono>

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
    auto color = bty::get_box_color(state.difficulty_level);

    hud_.load(assets, state);

    /* Create pause menu */
    pause_menu_.create(
		3, 7,
		26, 16,
		color,
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
    pause_menu_.add_option(3, 13, "Save game");

    /* Create "Use magic" menu */
    use_magic_.create(
		6, 4,
		20, 22,
		color,
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

    for (int i = 7; i < 14; i++) {
        use_magic_.set_option_disabled(i, true);
    }

    /* Create various pause-menu menu's */
    view_army_.load(assets, color);
    view_character_.load(assets, color, state.hero_id);
    view_continent_.load(assets, color);
    view_contract_.load(assets, color);
    view_puzzle_.load(assets);

    /* Load map */
    map_.load(assets);

    /* Load hero sprites */
    hero_.load(assets);
    
    /* Create end-of-week messages */
    astrology_.create(1, 18, 30, 9, color, assets);
    budget_.create(1, 18, 30, 9, color, assets);

    astrology_.add_line(1, 1, "");
    astrology_.add_line(1, 3, "");

    budget_.add_line(1, 1, "");
    budget_.add_line(23, 1, "Budget");
    budget_.add_line(1, 3, "");
    budget_.add_line(1, 4, "");
    budget_.add_line(1, 5, "");
    budget_.add_line(1, 6, "");
    budget_.add_line(1, 7, "");
    budget_.add_line(15, 3, "");
    budget_.add_line(15, 4, "");
    budget_.add_line(15, 5, "");
    budget_.add_line(15, 6, "");
    budget_.add_line(15, 7, "");

    /* Days run out/lose game */
    lose_msg_.create(1, 3, 20, 24, color, assets);
    lose_msg_name_ = lose_msg_.add_line(1, 2, "");
    lose_msg_.add_line(1, 4,
R"raw(you have failed to
recover the
Sceptre of Order
in time to save
the land! Beloved
King Maximus has
died and the Demon
King Urthrax
Killspite rules in
his place.  The
Four Continents
lay in ruin about
you, its people
doomed to a life
of misery and
oppression because
you could not find
the Sceptre.)raw");
    lose_pic_.set_texture(assets.get_texture("bg/king-dead.png"));
    lose_pic_.set_position(168, 24);

    dismiss_.create(1, 18, 30, 9, color, assets);
    dismiss_.add_line(5, 1, "Dismiss which army?");

    /* Found a map message */
    found_map_.create(1, 18, 30, 9, color, assets);
    found_map_.add_line(1, 2,
R"raw(  Hidden within an ancient
  chest, you find maps and
charts describing passage to)raw");
    found_map_continent_ = found_map_.add_line(10, 6, "");

    sail_dialog_.create(1, 18, 30, 9, color, assets);
    sail_dialog_.add_line(3, 1, "Sail to which continent?");

    town_.load(assets, color, state);

    loaded_ = true;
    return success;
}

void Game::enter()
{
    setup_game();
}

void Game::update_camera()
{
    glm::vec2 cam_centre = hero_.get_center();
    camera_pos_ = {cam_centre.x - 120, cam_centre.y - 120, 0.0f};
    game_camera_ = camera_ * glm::translate(-camera_pos_);
}

void Game::draw(bty::Gfx &gfx)
{
    bool tmp_hud{false};
    if (state_ == GameState::HudMessage) {
        state_ = last_state_;
        tmp_hud = true;
    }

    int continent = scene_switcher_->state().continent;
    switch (state_) {
        case GameState::Unpaused:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            break;
        case GameState::Paused:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            pause_menu_.draw(gfx, camera_);
            break;
        case GameState::ViewArmy:
            map_.draw(game_camera_, continent);
            hud_.draw(gfx, camera_);
            view_army_.draw(gfx, camera_);
            break;
        case GameState::ViewCharacter:
            hud_.draw(gfx, camera_);
            view_character_.draw(gfx, camera_);
            break;
        case GameState::ViewContinent:
            map_.draw(game_camera_, continent);
            hud_.draw(gfx, camera_);
            view_continent_.draw(gfx, camera_);
            break;
        case GameState::UseMagic:
            map_.draw(game_camera_, continent);
            hud_.draw(gfx, camera_);
            use_magic_.draw(gfx, camera_);
            break;
        case GameState::ViewContract:
            hud_.draw(gfx, camera_);
            view_contract_.draw(gfx, camera_);
            break;
        case GameState::WeekPassed:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            if (week_passed_card_ == WeekPassedCard::Astrology) {
                astrology_.draw(gfx, camera_);
            }
            else {
                budget_.draw(gfx, camera_);
            }
            break;
        case GameState::LoseGame:
            hud_.draw(gfx, camera_);
            lose_msg_.draw(gfx, camera_);
            gfx.draw_sprite(lose_pic_, camera_);
            break;
        case GameState::ViewPuzzle:
            hud_.draw(gfx, camera_);
            view_puzzle_.draw(gfx, camera_);
            break;
        case GameState::Dismiss:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            dismiss_.draw(gfx, camera_);
            break;
        case GameState::ChestMap:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            found_map_.draw(gfx, camera_);
            break;
        case GameState::SailNext:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            sail_dialog_.draw(gfx, camera_);
            break;
        case GameState::Town:
            hud_.draw(gfx, camera_);
            town_.draw(gfx, camera_);
            break;
        default:
            break;
    }
    if (tmp_hud) {
        state_ = GameState::HudMessage;
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
                        case GLFW_KEY_G:
                            hero_.set_mount(hero_.get_mount() == Mount::Fly ? Mount::Walk : Mount::Fly);
                            break;
                        case GLFW_KEY_P:
                            hero_.set_collision_rect_visible(!hero_.get_collision_rect_visible());
                            break;
                        case GLFW_KEY_R:
                            gen_tiles();
                            break;
                        case GLFW_KEY_L:
                            set_state(GameState::LoseGame);
                            break;
                        case GLFW_KEY_SPACE:
                            set_state(GameState::Paused);
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
                            set_state(GameState::Unpaused);
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
                                    set_state(GameState::ViewArmy);
                                    break;
                                case 1:
                                    set_state(GameState::ViewCharacter);
                                    break;
                                case 2:
                                    set_state(GameState::ViewContinent);
                                    break;
                                case 3:
                                    set_state(GameState::UseMagic);
                                    break;
                                case 4:
                                    set_state(GameState::ViewContract);
                                    break;
                                case 5:
                                    /*
                                        9 becomes 5
                                        8 becomes 5
                                        7 becomes 5
                                        etc
                                     */
                                    if ((scene_switcher_->state().days % 5) != 0) {
                                        scene_switcher_->state().days = (scene_switcher_->state().days / 5) * 5;
                                    }
                                    /*
                                        100 becomes 95
                                        65 becomes 60
                                        10 becomes 5
                                        etc
                                    */
                                    else {
                                        scene_switcher_->state().days = ((scene_switcher_->state().days / 5) - 1) * 5;
                                    }
                                    if (scene_switcher_->state().days == 0) {
                                        set_state(GameState::LoseGame);
                                    }
                                    else {
                                        set_state(GameState::WeekPassed);
                                    }
                                    clock_ = 0;
                                    hud_.update_state();
                                    break;
                                case 6:
                                    set_state(GameState::ViewPuzzle);
                                    break;
                                case 7:
                                    break;
                                case 8:
                                    set_state(GameState::Dismiss);
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
        case GameState::ViewPuzzle:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_SPACE: [[fallthrough]];
                        case GLFW_KEY_BACKSPACE: [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::ViewContract:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_SPACE: [[fallthrough]];
                        case GLFW_KEY_BACKSPACE: [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            if (last_state_ == GameState::Town) {
                                set_state(GameState::Town);
                            }
                            else {
                                set_state(GameState::Unpaused);
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
        case GameState::UseMagic:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
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
        case GameState::LoseGame:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_ENTER:
                            if (lose_state_ == 0) {
                                hud_.set_title("      Press Enter to play again.");
                                lose_state_++;
                            }
                            else {
                                scene_switcher_->fade_to(SceneId::Intro);
                            }
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::WeekPassed:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_ENTER:
                            if (week_passed_card_ == WeekPassedCard::Budget) {
                                week_passed_card_ = WeekPassedCard::Astrology;
                                set_state(GameState::Unpaused);
                            }
                            else {
                                week_passed_card_ = WeekPassedCard::Budget;
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
        case GameState::Dismiss:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_UP:
                            dismiss_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            dismiss_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            dismiss_slot(dismiss_.get_selection());
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::HudMessage:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER: [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            set_state(last_state_);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::ChestMap:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE: [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::SailNext:
            switch (action)
            {
                case GLFW_PRESS:
                    switch (key)
                    {
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            sail_to(scene_switcher_->state().continent);
                            break;
                        case GLFW_KEY_ENTER:
                            set_state(GameState::Unpaused);
                            sail_to(sail_dialog_.get_selection());
                            break;
                        case GLFW_KEY_UP:
                            sail_dialog_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            sail_dialog_.next();
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::Town:
            switch (action)
            {
                case GLFW_PRESS:
                    town_option(town_.key(key));
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
            view_continent_fog_ ? scene_switcher_->state().visited_tiles[scene_switcher_->state().continent].data() : map_.get_data(scene_switcher_->state().continent)
        );
    }
}

bool Game::loaded()
{
    return loaded_;
}

void Game::find_map(const Tile &tile) {
    auto &state = scene_switcher_->state();

    if (state.continent < 3 && map_tiles_[state.continent] == glm::ivec2{tile.tx, tile.ty}) {
        hud_.set_title("You found a map!");
        map_.erase_tile(tile, state.continent);
        state.maps_found[state.continent+1] = true;
        found_map_continent_->set_string(kContinents[state.continent+1]);
        set_state(GameState::ChestMap);
    }
}

int coord(int x, int y) {
    return ((x & 0xFF) << 8) | y;
}

void Game::town(const Tile &tile) {
    int continent = scene_switcher_->state().continent;

    int town = -1;

    for (int i = 0; i < 26; i++) {
        if (kTownInfo[i].x == tile.tx && 63 - kTownInfo[i].y == tile.ty) {
            town = i;
            break;
        }
    }

    town_.view(town, tile, continent, town_units_[town], town_spells_[town], castle_occupations_[kTownInfo[town].castle]);
    set_state(GameState::Town);
}

void Game::collide(Tile &tile) {
    switch (tile.id) {
        case Tile_Chest:
            find_map(tile);
            break;
        case Tile_Town:
            town(tile);
            break;
        default:
            break;
    }
}

void Game::update(float dt)
{
    bool tmp_hud{false};
    if (state_ == GameState::HudMessage) {
        state_ = last_state_;
    }

    if (state_ == GameState::Unpaused) {
        if (controls_locked_) {
            control_lock_timer_ -= dt;
            if (control_lock_timer_ <= 0) {
                controls_locked_ = false;
            }

            float speed = 100;
            float vel = speed * dt;
            float dx = auto_move_dir_.x * vel;
            float dy = auto_move_dir_.y * vel;

            auto manifold = hero_.move(dx, dy, map_, scene_switcher_->state().continent);
            hero_.set_position(manifold.new_position);
            update_visited_tiles();
            update_camera();
        }
        else {
            move_flags_ = MOVE_FLAGS_NONE;
            if (scene_switcher_->get_key(GLFW_KEY_LEFT)) {
                move_flags_ |= MOVE_FLAGS_LEFT;
            }
            if (scene_switcher_->get_key(GLFW_KEY_RIGHT)) {
                move_flags_ |= MOVE_FLAGS_RIGHT;
            }
            if (scene_switcher_->get_key(GLFW_KEY_UP)) {
                move_flags_ |= MOVE_FLAGS_UP;
            }
            if (scene_switcher_->get_key(GLFW_KEY_DOWN)) {
                move_flags_ |= MOVE_FLAGS_DOWN;
            }
        }

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

            float speed = hero_.get_mount() == Mount::Fly ? 200 : 100;
            float vel = speed * dt;
            float dx = dir.x * vel;
            float dy = dir.y * vel;

            auto manifold = hero_.move(dx, dy, map_, scene_switcher_->state().continent);

            hero_.set_position(manifold.new_position);

            if (manifold.collided) {
                for (auto &tile : manifold.collided_tiles) {
                    collide(tile);
                }
            }
            else if (manifold.out_of_bounds) {
                set_state(GameState::SailNext);
            }
            else {
                if (manifold.changed_tile) {
                    hero_.set_tile_info(manifold.new_tile);
                    if (hero_.get_mount() == Mount::Boat && manifold.new_tile.id == Tile_Grass) {
                        hero_.set_mount(Mount::Walk);
                    }
                    update_visited_tiles();
                }
            }

            update_camera();
        }
        else if (!controls_locked_) {
            hero_.set_moving(false);
        }
        map_.update(dt);
        hero_.animate(dt);

        if (clock_ >= 14) {
            clock_ = 0;
            int &days = scene_switcher_->state().days;
            days--;
            if (days == 0) {
                set_state(GameState::LoseGame);
            }
            else {
                days_passed_this_week++;
                if (days_passed_this_week == 5) {
                    days_passed_this_week = 0;
                    set_state(GameState::WeekPassed);
                    weeks_passed_++;
                    end_of_week(false);
                }
                hud_.update_state();
            }
        }

        clock_ += dt;
    }
    if (state_ == GameState::Paused
    || state_ == GameState::Unpaused
    || state_ == GameState::ViewContinent
    || state_ == GameState::UseMagic
    || state_ == GameState::ViewContract
    || state_ == GameState::ViewPuzzle
    || state_ == GameState::Dismiss) {
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
    else if (state_ == GameState::ViewPuzzle) {
        view_puzzle_.update(dt);
    }
    else if (state_ == GameState::Dismiss) {
        hero_.animate(dt);
        dismiss_.animate(dt);
    }
    else if (state_ == GameState::SailNext) {
        sail_dialog_.animate(dt);
    }
    else if (state_== GameState::Town) {
        hud_.update(dt);
        town_.update(dt);
    }

    if (tmp_hud) {
        state_ = GameState::HudMessage;
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

    int army_size = 0;

    for (int i = 0; i < 5; i++) {
        if (scene_switcher_->state().army[i] != -1) {
            army_size++;
        }
    }

    if (army_size == 5) {
        spdlog::warn("Game::add_unit_to_army: army already full");
        return;
    }

    int index = army_size++;

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
    auto index = tile.tx + tile.ty * 64;
    auto *visited = scene_switcher_->state().visited_tiles[scene_switcher_->state().continent].data();
    auto *tiles = map_.get_data(scene_switcher_->state().continent);

    visited[index] = tile.id;
    
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

    for (int i = 0; i < 7; i++) {
        use_magic_.set_option_disabled(i, spells[(i + 7) % 14] == 0);
        if (spells[(i + 7) % 14] != 0) {
            no_spells = false;
        }
    }

    if (no_spells) {
        hud_.set_title("You have no Adventuring spell to cast!");
    }

    int n = 0;

    magic_spells_[n]->set_string(fmt::format("{} Bridge", spells[7]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Time Stop", spells[8]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Find Villain", spells[9]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Castle Gate", spells[10]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Town Gate", spells[11]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Instant Army", spells[12]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Raise Control", spells[13]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Clone", spells[0]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Teleport", spells[1]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Fireball", spells[2]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Lightning", spells[3]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Freeze", spells[4]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Resurrect", spells[5]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Turn Undead", spells[6]));
}

void Game::end_of_week(bool search)
{
    std::string week = fmt::format("Week #{}", weeks_passed_);

    int unit = rand() % 25;

    const auto &name = kUnits[unit].name_singular;

    astrology_.set_line(0, week);
    astrology_.set_line(1, fmt::format("Astrologers proclaim:\nWeek of the {}\n\nAll {} dwellings are\nrepopulated.", name, name));

    auto &state = scene_switcher_->state();

    std::string on_hand = bty::number_with_ks(state.gold);

    int army_total = 0;
    int commission = state.commission;
    int gold = state.gold;
    int boat = state.boat_rented ? (state.artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500) : 0;
    int balance = (commission + gold) - boat;

    for (int i = 0; i < 5; i++) {
        if (state.army[i] == -1) {
            budget_.set_line(7+i, "");
            continue;
        }

        const auto &unit = kUnits[state.army[i]];
        int weekly_cost = unit.weekly_cost * state.army_counts[i];
        if (balance > weekly_cost || balance - weekly_cost == 0) {
            balance -= weekly_cost;
            army_total += weekly_cost;
            std::string cost = bty::number_with_ks(weekly_cost);
            std::string spaces(14-(cost.size()+unit.name_plural.size()), ' ');
            budget_.set_line(7+i, fmt::format("{}{}{}", unit.name_plural, spaces, cost));
        }
        else {
            std::string leave = "Leave";
            std::string spaces(14-(leave.size()+unit.name_plural.size()), ' ');
            budget_.set_line(7+i, fmt::format("{}{}{}", unit.name_plural, spaces, leave));
            state.army[i] = -1;
            state.army_counts[i] = -1;
        }
    }

    sort_army();

    bool out_of_money = (boat + army_total) > (gold + commission);
    if (!out_of_money) {
        balance = (commission + gold) - (boat - army_total);
    }

    state.gold = balance;

    budget_.set_line(0, week);
    budget_.set_line(2, fmt::format("On Hand: {:>4}", on_hand));
    budget_.set_line(3, fmt::format("Payment: {:>4}", commission));
    budget_.set_line(4, fmt::format("Boat: {:>7}", boat));
    budget_.set_line(5, fmt::format("Army: {:>7}", army_total));
    budget_.set_line(6, fmt::format("Balance: {:>4}", bty::number_with_ks(balance)));

    if ((state.army[0] == -1 || out_of_money) && search) {
        set_state(GameState::Disgrace);
    }
}

void Game::sort_army()
{
    int *army = scene_switcher_->state().army;
    int *army_counts = scene_switcher_->state().army_counts;

    int last_free = -1;
    for (int i = 0; i < 5; i++) {
        if (last_free == -1 && army[i] == -1) {
            last_free = i;
        }
        else if (last_free != -1 && army[i] != -1) {
            army[last_free] = army[i];
            army[i] = -1;
            army_counts[last_free] = army_counts[i];
            army_counts[i] = -1;
            last_free = i;
        }
    }
}

void Game::setup_game()
{
    set_state(GameState::Unpaused);

    auto &state = scene_switcher_->state();

    /* Misc */
    controls_locked_ = false;
    control_lock_timer_ = 0;

    /* Clear spells */
    for (int i = 0; i < 14; i++) {
        state.spells[i] = 0;
    }
    state.known_spells = 0;

    /* Known villains */
    for (int i = 0; i < 17; i++) {
        state.known_villains[i] = false;
    }

    /* Set days */
    state.days = kDays[state.difficulty_level];

    /* Set hero to starting position */
    hero_.move_to_tile(map_.get_tile(11, 58, scene_switcher_->state().continent));
    update_camera();

    /* Clear visited tiles */
    for (int i = 0; i < 4; i++) {
        state.visited_tiles[i].resize(4096);
        std::fill(state.visited_tiles[i].begin(), state.visited_tiles[i].end(), -1);
    }
    update_visited_tiles();

    /* Update box colours */
    auto color = bty::get_box_color(state.difficulty_level);
    hud_.set_color(color);
    pause_menu_.set_color(color);
    use_magic_.set_color(color);
    view_army_.set_color(color);
    view_character_.set_color(color);
    view_continent_.set_color(color);
    view_contract_.set_color(color);
    astrology_.set_color(color);
    budget_.set_color(color);

    /* Add starting army */
    for (int i = 0; i < 5; i++) {
        state.army[i] = -1;
        state.army_counts[i] = 0;
    }

    switch (state.hero_id) {
        case 0:
            state.army[0] = Militias;
            state.army_counts[0] = 20;
            state.army[1] = Archers;
            state.army_counts[1] = 2;
            break;
        case 1:
            state.army[0] = Peasants;
            state.army_counts[0] = 20;
            state.army[1] = Militias;
            state.army_counts[1] = 20;
            break;
        case 2:
            state.army[0] = Peasants;
            state.army_counts[0] = 30;
            state.army[1] = Sprites;
            state.army_counts[1] = 10;
            break;
        case 3:
            state.army[0] = Wolves;
            state.army_counts[0] = 20;
            break;
        default:
            break;
    }
    
    /* Set starting stats */
    state.gold = kStartingGold[state.hero_id];
    state.commission = kRankCommission[state.hero_id][0];
    state.leadership = kRankLeadership[state.hero_id][0];
    state.max_spells = kRankSpells[state.hero_id][0];
    state.spell_power = kRankSpellPower[state.hero_id][0];

    /* Reset dialogs */
    pause_menu_.set_selection(0);
    use_magic_.set_selection(0);
    dismiss_.set_selection(0);

    if (state.hero_id == 2) {
        state.magic = true;
    }

    /* Add continentia to maps */
    for (int i = 0; i < 4; i++) {
        state.maps_found[i] = false;
    }
    state.maps_found[0] = true;
    
    /* Villains and artifacts */
    for (int i = 0; i < 17; i++) {
        state.villains_caught[i] = false;
    }
    for (int i = 0; i < 8; i++) {
        state.artifacts_found[i] = false;
    }

    hud_.update_state();

    gen_tiles();
}

void Game::gen_tiles() {
    map_.reset();

    static constexpr int kNumShopsPerContinent[4] = {
		6, 6, 4, 5
	};

    static constexpr int kAvailableUnitsPerContinent[4][6] = {
		{
            Peasants,
            Sprites,
            Orcs,
            Skeletons,
            Wolves,
            Gnomes,
        },
		{
            Dwarves,
            Zombies,
            Nomads,
            Elves,
            Ogres,
            Elves
        },
		{
            Ghosts,
            Barbarians,
            Trolls,
            Druids,
            Peasants,
            Peasants
        },
		{
            Giants,
            Vampires,
            Archmages,
            Dragons,
            Demons,
            Peasants
        },
	};

    static constexpr int kShopTileForUnit[] = {
		Tile_ShopWagon,
		Tile_ShopTree,
		Tile_ShopCave,
		Tile_ShopDungeon,
		Tile_ShopCave,
		Tile_ShopTree,
		Tile_ShopDungeon,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopCave,
		Tile_ShopTree,
		Tile_ShopCave,
		Tile_ShopTree,
		Tile_ShopDungeon,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopCave,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopTree,
		Tile_ShopTree,
	};

    static constexpr int kArtifactTiles[] = {
        Tile_AfctScroll, Tile_AfctShield, Tile_AfctCrown, Tile_AfctAmulet,
        Tile_AfctRing, Tile_AfctAnchor, Tile_AfctBook, Tile_AfctSword,
    };

    /* Place artifacts in random order, then when adding them to
        continents just increment an index into this array. */
    std::vector<int> artifacts {
        0, 1, 2, 3, 4, 5, 6, 7
    };
    
    rng_.seed(std::chrono::system_clock::now().time_since_epoch().count());
    std::shuffle(std::begin(artifacts), std::end(artifacts), rng_);

    int artifact_index = 0;

    for (int i = 0; i < 26; i++) {
        town_units_[i] = Peasants;
    }

    std::uniform_int_distribution<int> unit_gen(0, 24);
    std::uniform_int_distribution<int> spell_gen(0, 14);

    for (int continent = 0; continent < 4; continent++) {
        auto *tiles = map_.get_data(continent);

        std::vector<glm::ivec2> random_tiles;
        std::vector<int> castle_indices;

        int n = 0;
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 64; x++) {
                int id = tiles[n];
                if (id == Tile_GenRandom) {
                    random_tiles.push_back({x, y});
                }
                else if (id == Tile_GenTown) {
                    int town = -1;
                    for (int i = 0; i < kTownsPerContinent[continent]; i++) {
                        int index = kTownIndices[continent] + i;
                        if (kTownInfo[index].x == x && 63 - kTownInfo[index].y == y) {
                            town = index;
                        }
                    }
                    if (town != -1) {
                        town_units_[town] = unit_gen(rng_);

                        /* Hunterville always has Bridge */
                        if (town == 2) {
                            town_spells_[town] = 7;
                        }
                        else {
                            town_spells_[town] = spell_gen(rng_);
                        }

                        tiles[x + y * 64] = Tile_Town;
                    }
                    else {
                        spdlog::warn("({}) Unknown town at {}, {}", continent, x, 63-y);
                    }
                }
                else if (id == Tile_GenCastleGate) {
                    int castle = -1;
                    for (int i = 0; i < kCastlesPerContinent[continent]; i++) {
                        int index = kCastleIndices[continent] + i;
                        if (kCastleInfo[index].x == x && 63 - kCastleInfo[index].y == y) {
                            castle = index;
                            break;
                        }
                    }
                    if (castle != -1) {
                        castle_indices.push_back(castle);
                        tiles[x + y * 64] = Tile_CastleB;
                    }
                    else {
                        /* Not king's castle */
                        if (continent != 0 && x != 11 && y != 56) {
                            spdlog::warn("({}) Unknown castle at {}, {}", continent, x, 63-y);
                        }
                    }
                }
                n++;
            }
        }

        if (castle_indices.size() != kCastlesPerContinent[continent]) {
            spdlog::error("({}) Failed to find {} castles!", continent, kCastlesPerContinent[continent] - castle_indices.size());

            for (int i = 0; i < kCastlesPerContinent[continent]; i++) {
                bool found = false;
                int index = kCastleIndices[continent] + i;
                for (int j = 0; j < castle_indices.size(); j++) {
                    if (castle_indices[j] == index) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    spdlog::error("Missing {}", kCastleInfo[index].name);
                }
            }
        }

        rng_.seed(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(std::begin(random_tiles), std::end(random_tiles), rng_);

        rng_.seed(std::chrono::system_clock::now().time_since_epoch().count());
        std::shuffle(std::begin(castle_indices), std::end(castle_indices), rng_);

        int used_tiles = 0;

        for (int i = 0; i < kNumShopsPerContinent[continent]; i++) {
            int unit = kAvailableUnitsPerContinent[continent][rand() % 6];
            tiles[random_tiles[used_tiles].x + random_tiles[used_tiles].y * 64] = kShopTileForUnit[unit];
            used_tiles++;
        }

        /* Saharia doesn't have a map to anywhere */
        if (continent != 3) {
            tiles[random_tiles[used_tiles].x + random_tiles[used_tiles].y * 64] = Tile_Chest;
            map_tiles_[continent] = random_tiles[used_tiles];
            used_tiles++;
        }

        /* 2 artifacts per continent */
        auto tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifact_index++]];
        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifact_index++]];

        for (int i = 0; i < kCastlesPerContinent[continent]; i++) {
            int index = kCastleIndices[continent] + i;
            castle_occupations_[index] = {
                index,
                -1,
                {{Peasants, Peasants, Peasants, Peasants, Peasants}},
                {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}
            };
        }

        int used_castles = 0;
        for (int i = 0; i < kVillainsPerContinent[continent]; i++) {
            int castle = castle_indices[used_castles++];

            CastleOccupation occ;
            occ.index = castle;
            occ.occupier = i + kVillainIndices[continent];

            for (int j = 0; j < 5; j++) {
                occ.army[j] = unit_gen(rng_);
                occ.army_counts[j] = rand() % 100;
            }

            castle_occupations_[castle] = occ;
        }
    }

    map_.create_geometry();
}

void Game::disgrace() {
    set_state(GameState::LoseGame);
}

void Game::lose_game() {
    switch (scene_switcher_->state().hero_id) {
        case 0:
            lose_msg_name_->set_string("Oh, Sir Crimsaun");
            break;
        case 1:
            lose_msg_name_->set_string("Oh, Lord Palmer");
            break;
        case 2:
            lose_msg_name_->set_string("Oh, Moham");
            break;
        case 3:
            lose_msg_name_->set_string("Oh, Tynnestra");
            break;
        default:
            break;
    }
    hud_.set_blank_frame();
    lose_state_ = 0;
}

void Game::view_puzzle()
{
    view_puzzle_.view(scene_switcher_->state().villains_caught, scene_switcher_->state().artifacts_found);
}

void Game::dismiss()
{
    int opt = dismiss_.get_selection();
    dismiss_.clear_options();

    auto &state = scene_switcher_->state();

    for (int i = 0; i < 5; i++) {
        if (state.army[i] != -1) {
            dismiss_.add_option(11, 3 + i, kUnits[state.army[i]].name_plural);
        }
    }

    if (opt > 0) {
        dismiss_.set_selection(opt-1);
    }
}

void Game::dismiss_slot(int slot) {
    int num_units = 0;
    for (int i = 0; i < 5; i++) {
        if (scene_switcher_->state().army[i] != -1) {
            num_units++;
        }
    }

    if (num_units == 1) {
        hud_.set_title("  You may not dismiss your last army!");
        set_state(GameState::HudMessage);
        return;
    }

    scene_switcher_->state().army[slot] = -1;
    scene_switcher_->state().army_counts[slot] = 0;
    sort_army();
    dismiss();
}

void Game::set_state(GameState state) {
    switch (state) {
        case GameState::Unpaused:
            hud_.set_hud_frame();
            hud_.update_state();
            break;
        case GameState::LoseGame:
            lose_game();
            break;
        case GameState::ViewArmy:
            view_army_.view(scene_switcher_->state());
            break;
        case GameState::ViewCharacter:
            view_character_.view(scene_switcher_->state());
            break;
        case GameState::ViewContinent:
            view_continent_.view(
                hero_.get_tile().tx,
                hero_.get_tile().ty,
                scene_switcher_->state().continent,
                view_continent_fog_ ? scene_switcher_->state().visited_tiles[scene_switcher_->state().continent].data() : map_.get_data(scene_switcher_->state().continent)
            );
            break;
        case GameState::UseMagic:
            update_spells();
            break;
        case GameState::ViewContract:
            view_contract();
            break;
        case GameState::WeekPassed:
            weeks_passed_++;
            end_of_week(false);
            break;
        case GameState::ViewPuzzle:
            view_puzzle();
            break;
        case GameState::Dismiss:
            dismiss();
            hud_.update_state();
            break;
        case GameState::Disgrace:
            disgrace();
            break;
        case GameState::HudMessage:
            break;
        case GameState::SailNext:
            sail_next();
            break;
        default:
            break;
    }

    last_state_ = state_;
    state_ = state;
    clear_movement();
}

void Game::clear_movement() {
    move_flags_ = MOVE_FLAGS_NONE;
    hero_.set_moving(false);
}

void Game::sail_next() {
    sail_dialog_.clear_options();

    auto &state = scene_switcher_->state();

    for (int i = 0; i < 4; i++) {
        if (state.maps_found[i]) {
            sail_dialog_.add_option(10, 3+i, kContinents[i]);
        }
    }
}

void Game::sail_to(int continent) {
    if (continent == scene_switcher_->state().continent) {
        auto pos = hero_.get_position();

        auto_move_dir_ = {0,0};

        if (pos.x <= 0) {
            auto_move_dir_.x = 1;
            hero_.set_flip(false);
        }
        else if (pos.x >= 48 * 63) {
            auto_move_dir_.x = -1;
            hero_.set_flip(true);
        }

        if (pos.y <= 0) {
            auto_move_dir_.y = 1;
        }
        else if (pos.y >= 40 * 63) {
            auto_move_dir_.y = -1;
        }

        control_lock_timer_ = 1;
        controls_locked_ = true;

        hero_.set_moving(true);
    }
    else {
        control_lock_timer_ = 1;
        controls_locked_ = true;
        hero_.set_moving(true);

        switch (continent) {
            case 0:
                auto_move_dir_.x = 0;
                auto_move_dir_.y = 1;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(11, 58, continent));
                break;
            case 1:
                auto_move_dir_.x = -1;
                auto_move_dir_.y = 1;
                hero_.set_flip(true);
                hero_.move_to_tile(map_.get_tile(63, 1, continent));
                break;
            case 2:
                auto_move_dir_.x = 1;
                auto_move_dir_.y = 0;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(1, 20, continent));
                break;
            case 3:
                auto_move_dir_.x = 0;
                auto_move_dir_.y = 1;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(32, 63, continent));
                break;
            default:
                break;
        }

        hero_.set_mount(Mount::Boat);
        update_camera();
        scene_switcher_->state().continent = continent;
    }
}

void Game::rent_boat() {
    scene_switcher_->state().boat_rented = !scene_switcher_->state().boat_rented;
}

void Game::town_option(int opt) {
    switch (opt) {
        case -1: // internal option
            break;
        case -2: // close town
            set_state(GameState::Unpaused);
            break;
        case 0:
            next_contract();
            break;
        case 1:
            rent_boat();
            break;
        case 3:
            buy_spell();
            break;
        case 4:
            buy_siege();
            break;
        default:
            break;
    }
}

void Game::next_contract() {
    auto &state = scene_switcher_->state();

    int num_caught = 0;
    for (int i = 0; i < 17; i++) {
        if (state.villains_caught[i]) {
            num_caught++;
        }
    }
    if (num_caught == 17) {
        return;
    }

    int contracts[5];
    int have = 0;

    /* Get the first five uncaught villains */
    for (int i = 0; i < 17; i++) {
        if (!state.villains_caught[i]) {
            contracts[have++] = i;
            if (have == 5) {
                break;
            }
        }
    }

    int current = 0;
    for (int i = 0; i < 5; i++) {
        if (state.contract == contracts[i]) {
            current = i;
        }
    }

    if (state.contract == 17 && contracts[0] == 0) {
        state.contract = 0;
    }
    else {
        state.contract = contracts[(current + 1) % 5];
    }

    hud_.update_state();
    set_state(GameState::ViewContract);
}

void Game::view_contract() {
    auto &state = scene_switcher_->state();

    std::string castle;

    if (state.known_villains[state.contract]) {
        int castle_id = -1;
        for (int i = 0; i < 26; i++) {
            if (castle_occupations_[i].occupier == state.contract) {
                castle_id = i;
                break;
            }
        }
        if (castle_id != -1) {
            castle = kCastleInfo[castle_id].name;
        }
        else {
            spdlog::warn("Can't find castle for villain {}", state.contract);
            castle = "Unknown";
        }
    }
    else {
        castle = "Unknown";
    }

    view_contract_.view(scene_switcher_->state().contract, castle, hud_.get_contract());
}

void Game::buy_spell()
{
    auto &state = scene_switcher_->state();

    int town = town_.get_town();

    if (state.known_spells < state.max_spells) {
        if (state.gold >= kSpellCosts[town_spells_[town]]) {
            state.gold -= kSpellCosts[town_spells_[town]];
            state.spells[town_spells_[town_.get_town()]]++;
            state.known_spells++;
            town_.update_gold();

            int remaining = state.max_spells - state.known_spells;
            hud_.set_title(fmt::format("     You can learn {} more spell{}.", remaining, remaining != 1 ? "s" : ""));
        }
        else {
            hud_.set_title("    You do not have enough gold!");
            set_state(GameState::HudMessage);
        }
    }
    else {
        hud_.set_title("  You can not learn anymore spells!");
        set_state(GameState::HudMessage);
    }
}

void Game::buy_siege()
{
    auto &state = scene_switcher_->state();

    if (state.gold < 3000) {
        hud_.set_title("    You do not have enough gold!");
        set_state(GameState::HudMessage);
    }
    else {
        state.siege = true;
        hud_.update_state();
        town_.update_gold();
    }
}
