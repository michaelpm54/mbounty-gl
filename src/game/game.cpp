#include "game/game.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "assets.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

#include "gfx/gfx.hpp"

static constexpr const char *kNames[4][4] = 
{
    "Sir Crimsaun the Knight",
    "Lord Palmer the Paladin",
    "Mad Moham the Barbarian",
    "Tynnestra the Sorceress",
};

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

    font_.load_from_texture(assets.get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});
    hud_.load(assets, font_, scene_switcher_->state());

    map_.load(assets);

    hero_walk_moving_texture_ = assets.get_texture("hero/walk-moving.png", {4, 1});
    hero_walk_stationary_texture_ = assets.get_texture("hero/walk-stationary.png", {4, 1});
    hero_boat_moving_texture_ = assets.get_texture("hero/boat-moving.png", {4, 1});
    hero_boat_stationary_texture_ = assets.get_texture("hero/boat-stationary.png", {2, 1});

    hero_.set_texture(hero_walk_stationary_texture_);

    hero_.set_tile(11, 57);
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
    map_.draw(game_camera_);
    gfx.draw_sprite(hero_, game_camera_);
    hud_.draw(gfx, camera_);
}

void Game::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (action)
    {
        case GLFW_PRESS:
            switch (key)
            {
                case GLFW_KEY_SPACE:
                    if (state_ == GameState::Unpaused) {
                        state_ = GameState::Paused;
                    }
                    else if (state_ == GameState::Paused) {
                        state_ = GameState::Unpaused;
                    }
                    break;
                case GLFW_KEY_B:
                    in_boat_ = !in_boat_;
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
}

bool Game::loaded()
{
    return loaded_;
}

void Game::update(float dt)
{
    if (move_flags_) {
        if ((move_flags_ & MOVE_FLAGS_LEFT) && !(move_flags_ & MOVE_FLAGS_RIGHT)) {
            hero_.set_flip(true);
        }
        else if ((move_flags_ & MOVE_FLAGS_RIGHT) && !(move_flags_ & MOVE_FLAGS_LEFT)) {
            hero_.set_flip(false);
        }

        hero_.move(dt, move_flags_, map_);
        update_camera();

        if (in_boat_) {
            hero_.set_texture(hero_boat_moving_texture_);
        }
        else {
            hero_.set_texture(hero_walk_moving_texture_);
        }
    }
    else {
        if (in_boat_) {
            hero_.set_texture(hero_boat_stationary_texture_);
        }
        else {
            hero_.set_texture(hero_walk_stationary_texture_);
        }
    }

    hud_.update(dt);
    map_.update(dt);
    hero_.animate(dt);
}
