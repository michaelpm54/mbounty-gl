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
        border_textures[i] = assets.load_texture(filename, success);
    }

    glm::vec4 accents[2] = {
        {0.0f, 32.0f/255.0f, 99.0f/255.0f, 1.0f},
        {33.0f/255.0f, 163.0f/255.0f, 232.0f/255.0f, 1.0f}
    };

    bg_.set_texture(assets.load_texture("bg/intro.png", success));

    font_.load_from_texture(assets.load_texture("fonts/genesis_custom.png", success), {8.0f, 8.0f});

    name_box_.create(7, 1, 27, 3, accents, border_textures);
    diff_box_.create(7, 10, 27, 8, accents, border_textures);
    help_box_.create(1, 24, 38, 3, accents, border_textures);

    name_text_.create(9, 2, "Sir Crimsaun the Knight", font_);

    loaded_ = true;
    return success;
}

void Intro::draw(bty::Gfx &gfx)
{
    gfx_draw_sprite(&gfx, bg_, camera_);
    name_box_.draw(gfx, camera_);
    diff_box_.draw(gfx, camera_);
    help_box_.draw(gfx, camera_);
    gfx_draw_text(&gfx, name_text_, camera_);
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
                case GLFW_KEY_B:
                    if (glIsEnabled(GL_BLEND))
                        glDisable(GL_BLEND);
                    else
                        glEnable(GL_BLEND);
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
