#include "game/game.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

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

    FILE *map_stream = fopen("data/maps/genesis/continentia.bin", "rb");
    if (!map_stream) {
        spdlog::error("Failed to load map");
        return false;
    }
    unsigned char *map_data = new unsigned char[4096];
    fread(map_data, 1, 4096, map_stream);
    fclose(map_stream);
    map_.load(assets, map_data);
    delete[] map_data;

    game_camera_ = glm::translate(camera_, camera_pos_);

    loaded_ = true;
    return success;
}

void Game::draw(bty::Gfx &gfx)
{
    map_.draw(game_camera_);
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
        if (move_flags_ & MOVE_FLAGS_LEFT) {
            camera_pos_.x += 2.0f;
        }
        if (move_flags_ & MOVE_FLAGS_RIGHT) {
            camera_pos_.x -= 2.0f;
        }
        if (move_flags_ & MOVE_FLAGS_UP) {
            camera_pos_.y += 2.0f;
        }
        if (move_flags_ & MOVE_FLAGS_DOWN) {
            camera_pos_.y -= 2.0f;
        }

        game_camera_ = glm::translate(camera_, camera_pos_);
    }

    hud_.update(dt);
    map_.update(dt);
}
