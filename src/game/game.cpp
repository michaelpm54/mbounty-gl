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
