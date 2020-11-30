#include "intro/intro.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

#include "gfx/gfx.hpp"

static constexpr const char *kNames[] = 
{
    "Sir Crimsaun the Knight",
    "Lord Palmer the Paladin",
    "Mad Moham the Barbarian",
    "Tynnestra the Sorceress",
};

Intro::Intro(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Intro::load(bty::Assets &assets)
{
    bool success {true};

    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    std::array<const bty::Texture *, 8> border_textures;

    for (int i = 0; i < 8; i++) {
        std::string filename = fmt::format("border-normal/box{}.png", i);
        border_textures[i] = assets.get_texture(filename);
    }

    bg_.set_texture(assets.get_texture("bg/intro.png"));

    font_.load_from_texture(assets.get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});

    name_box_.create(7, 1, 27, 3, bty::BoxColor::Intro, border_textures, font_);
    help_box_.create(1, 24, 38, 3, bty::BoxColor::Intro, border_textures, font_);
    help_box_.add_line(2, 1, "Select a character and press Enter");

    name_box_.add_line(2, 1, kNames[hero_]);

    auto *arrow = assets.get_texture("arrow.png", {2, 2});

    diff_box_.create(7, 10, 27, 8, bty::BoxColor::Intro, border_textures, font_, arrow);
    diff_box_.add_line(2, 1, "Difficulty");
    diff_box_.add_line(15, 1, "Days");
    diff_box_.add_line(21, 1, "Score");

    static const std::string difficulties[4][3] =
	{
		{"Easy", "900", "x.5"},
		{"Normal", "600", " x1"},
		{"Hard", "400", " x2"},
		{"Impossible?", "200", " x4"},
	};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            if (j == 0) {
                diff_box_.add_option(3, 3 + i, difficulties[i][j]);
            }
            else if (j == 1) {
                diff_box_.add_line(16, 3 + i, difficulties[i][j]);
            }
            else if (j == 2) {
                diff_box_.add_line(22, 3 + i, difficulties[i][j]);
            }
        }
    }

    loaded_ = true;
    return success;
}

void Intro::draw(bty::Gfx &gfx)
{
    gfx.draw_sprite(bg_, camera_);
    name_box_.draw(gfx, camera_);
    if (state_ == IntroState::ChoosingDifficulty) {
        diff_box_.draw(gfx, camera_);
    }
    help_box_.draw(gfx, camera_);
}

void Intro::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (action)
    {
        case GLFW_PRESS:
            switch (key)
            {
                case GLFW_KEY_LEFT:
                    if (state_ == IntroState::ChoosingHero) {
                        hero_--;
                        if (hero_ == -1) {
                            hero_ = 3;
                        }
                        name_box_.set_line(0, kNames[hero_]);
                    }
                    break;
                case GLFW_KEY_RIGHT:
                    if (state_ == IntroState::ChoosingHero) {
                        hero_ = (hero_ + 1) % 4;
                        name_box_.set_line(0, kNames[hero_]);
                    }
                    break;
                case GLFW_KEY_ENTER:
                    if (state_ == IntroState::ChoosingHero) {
                        help_box_.set_line(0, "Select a difficulty and press Enter");
                        state_ = IntroState::ChoosingDifficulty;
                    }
                    else if (state_ == IntroState::ChoosingDifficulty) {
                        scene_switcher_->state().hero_id = hero_;
                        scene_switcher_->state().difficulty_level = diff_box_.get_selection();
                        scene_switcher_->fade_to(SceneId::Game);
                    }
                    break;
                case GLFW_KEY_UP:
                    diff_box_.prev();
                    break;
                case GLFW_KEY_DOWN:
                    diff_box_.next();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

bool Intro::loaded()
{
    return loaded_;
}

void Intro::update(float dt)
{
    diff_box_.animate(dt);
}
