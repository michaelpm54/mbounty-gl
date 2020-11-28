#include "intro/intro.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"

#include "gfx/gfx.hpp"

bool Intro::load(bty::Assets &assets)
{
    bool success {true};

    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    std::array<const bty::Texture *, 8> border_textures;

    for (int i = 0; i < 8; i++) {
        std::string filename = fmt::format("border-normal/box{}.png", i);
        border_textures[i] = assets.get_texture(filename);
    }

    glm::vec4 accents[2] = {
        {0.0f, 32.0f/255.0f, 99.0f/255.0f, 1.0f},
        {33.0f/255.0f, 163.0f/255.0f, 232.0f/255.0f, 1.0f}
    };

    bg_.set_texture(assets.get_texture("bg/intro.png"));

    font_.load_from_texture(assets.get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});

    name_box_.create(7, 1, 27, 3, accents, border_textures, font_);
    help_box_.create(1, 24, 38, 3, accents, border_textures, font_);

    name_box_.add_line(2, 1, "Sir Crimsaun the Knight");

    auto *arrow = assets.get_texture("arrow.png", {2, 2});

    diff_box_.create(7, 10, 27, 8, accents, border_textures, font_, arrow);
    diff_box_.add_option(3, 3, "Test A");
    diff_box_.add_option(3, 4, "Test B");
    diff_box_.add_option(3, 5, "Test C");
    diff_box_.add_option(3, 6, "Test D");

    loaded_ = true;
    return success;
}

void Intro::draw(bty::Gfx &gfx)
{
    gfx.draw_sprite(bg_, camera_);
    name_box_.draw(gfx, camera_);
    diff_box_.draw(gfx, camera_);
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
                    break;
                case GLFW_KEY_RIGHT:
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
