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

static constexpr int kDays[4] = {
    900,
    600,
    400,
    200,
};

Game::Game(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

static constexpr unsigned char kBoxAccents[4][2][3] =
{
	// Easy, green
	{ 
		{ 0, 32, 0 },
		{ 66, 130, 66 },
	},
	// Normal, blue
	{
		{ 0, 0, 33 },
		{ 65, 65, 132 },
	},
	// Hard, red
	{
		{ 33, 0, 0 },
		{ 132, 65, 65 },
	},
	// Impossible, grey
	{
		{ 33, 32, 33 },
		{ 132, 130, 132 },
	},
};

bool Game::load(bty::Assets &assets)
{
    bool success {true};

    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    std::array<const bty::Texture *, 8> border_textures;

    for (int i = 0; i < 8; i++) {
        std::string filename = fmt::format("border-normal/box{}.png", i);
        border_textures[i] = assets.get_texture(filename);
    }

    scene_switcher_->state().days = kDays[scene_switcher_->state().difficulty_level];

    font_.load_from_texture(assets.get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});
    hud_.load(assets, font_, scene_switcher_->state());

    const auto *accents = kBoxAccents[scene_switcher_->state().difficulty_level];

    glm::vec4 box_accents[2] = {
        { accents[0][0] / 255.0f, accents[0][1] / 255.0f, accents[0][2] / 255.0f, 1.0f },
        { accents[1][0] / 255.0f, accents[1][1] / 255.0f, accents[1][2] / 255.0f, 1.0f },
    };

    pause_menu_.create(
		3, 7,
		26, 16,
		box_accents,
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

    scene_switcher_->state().army_size = 3;
    scene_switcher_->state().army[0] = 12;
    scene_switcher_->state().army[1] = 14;
    scene_switcher_->state().army[2] = 0;
    view_army_.load(assets, box_accents[1]);

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
        case GameState::ViewArmy:
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
    if (state_ == GameState::Paused || state_ == GameState::Unpaused) {
        hud_.update(dt);
        pause_menu_.animate(dt);
    }
    if (state_ == GameState::ViewArmy) {
        view_army_.update(dt);
    }
}
