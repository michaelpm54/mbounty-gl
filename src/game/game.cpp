#include "game/game.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "assets.hpp"
#include "game/army-gen.hpp"
#include "game/chest.hpp"
#include "game/shop-info.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"

void Game::intro_pop(int ret)
{
    switch (ret) {
        case 0:
            ingame.setup(hero_id, difficulty);
            ss.push(&ingame, std::bind(&Game::ingame_pop, this, std::placeholders::_1));
            break;
        default:
            break;
    }
}

void Game::ingame_pop(int ret)
{
    spdlog::debug("Ingame popped!");

    switch (ret) {
        case 0:    // reset
            ss.push(&intro, std::bind(&Game::intro_pop, this, std::placeholders::_1));
            break;
        case 1:    // battle
            break;
        default:
            break;
    }
}

Game::Game(GLFWwindow *window, bty::Assets &assets)
    : window_(window)
    , ss(assets)
    , ds(assets)
    , intro(ss, ds, assets, hero_id, difficulty)
    , ingame(window, ss, ds, assets, hud_)
    , hud_(assets)
{
    ss.push(&intro, std::bind(&Game::intro_pop, this, std::placeholders::_1));
    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);
}

void Game::load(bty::Assets &assets, int difficulty)
{
    bool success {true};

    auto color = bty::get_box_color(difficulty);

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
}

void Game::draw(bty::Gfx &gfx)
{
    ss.draw(gfx, camera_);
    ds.draw(gfx, camera_);
}

void Game::key(int key, int action)
{
    if (hud_.get_error()) {
        if (action == GLFW_PRESS && key == GLFW_KEY_ENTER) {
            hud_.clear_error();
            return;
        }
    }

    if (!ds.key(key, action)) {
        ss.key(key, action);
    }
}

void Game::update(float dt)
{
    hud_.update(dt);
    ss.update(dt);
    ds.update(dt);
}

void Game::lose_game()
{
    /*
    switch (v.hero_id) {
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
	*/
}

/*
void Game::end_battle(bool victory)
{
    if (victory) {
        if (v.enemy_index != -1) {
            mobs_[v.continent][v.enemy_index].dead = true;
        }
        sort_army(v.army, v.army_counts);
    }
    else {
        if (v.enemy_index != -1) {
            for (int i = 0; i < 6; i++) {
                mobs_[v.continent][v.enemy_index].army[i] = v.enemy_army[i];
                mobs_[v.continent][v.enemy_index].counts[i] = v.enemy_counts[i];
            }
        }
        disgrace();
    }
}
*/
