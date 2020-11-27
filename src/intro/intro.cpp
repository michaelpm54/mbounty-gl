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

    bg_.set_texture(assets.load_texture("bg/intro.png", success));

    loaded_ = true;
    return success;
}

void Intro::draw(bty::Gfx &gfx)
{
    gfx_draw_sprite(&gfx, bg_, camera_);
}

void Intro::key(int key, int scancode, int action, int mods)
{
    (void)key;
    (void)scancode;
    (void)action;
    (void)mods;
}

bool Intro::loaded()
{
    return loaded_;
}
