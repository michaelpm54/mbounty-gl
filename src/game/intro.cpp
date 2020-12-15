#include "game/intro.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "data/bounty.hpp"
#include "data/hero.hpp"
#include "engine/assets.hpp"
#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"

Intro::Intro(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, int &hero_id, int &difficulty)
    : ss(ss)
    , ds(ds)
    , hero_id_(hero_id)
    , difficulty_(difficulty)
{
    bg_.set_texture(assets.get_texture("bg/intro.png"));
    name_box_.create(7, 1, 27, 3, bty::BoxColor::Intro, assets);
    name_box_.add_line(2, 1, kHeroNames[0][0]);
    help_box_.create(1, 24, 38, 3, bty::BoxColor::Intro, assets);
    help_box_.add_line(2, 1, "Select a character and press Enter");
}

void Intro::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    name_box_.draw(gfx, camera);
    ds.draw(gfx, camera);
    help_box_.draw(gfx, camera);
}

void Intro::key(int key, int action)
{
    if (ds.empty()) {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_LEFT:
                    hero_id_ = (hero_id_ - 1 + 4) % 4;
                    name_box_.set_line(0, kHeroNames[hero_id_][0]);
                    break;
                case GLFW_KEY_RIGHT:
                    hero_id_ = (hero_id_ + 1) % 4;
                    name_box_.set_line(0, kHeroNames[hero_id_][0]);
                    break;
                case GLFW_KEY_ENTER:
                    show_difficulty();
                    break;
                default:
                    break;
            }
        }
    }
    else {
        ds.key(key, action);
    }
}

void Intro::update(float dt)
{
}

void Intro::show_difficulty()
{
    auto *dialog = ds.show_dialog({
        .x = 7,
        .y = 10,
        .w = 27,
        .h = 8,
        .strings = {
            {2, 1, "Difficulty   Days  Score"},
        },
        .options = {
            {3, 3, "Easy         900   x.5"},
            {3, 4, "Normal       600    x1"},
            {3, 5, "Hard         400    x2"},
            {3, 6, "Impossible?  200    x4"},
        },
        .callbacks = {
            .confirm = [this](int opt) {
                difficulty_ = opt;
                ss.pop(0);
            },
        },
    });
    dialog->set_selection(1);
    dialog->set_color(bty::BoxColor::Intro);
}
