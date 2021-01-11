#include "game/intro.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "data/bounty.hpp"
#include "data/hero.hpp"
#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "engine/texture-cache.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"

Intro::Intro(bty::SceneStack &ss, bty::DialogStack &ds, Ingame &ingame)
    : ss(ss)
    , ds(ds)
    , ingame(ingame)
{
    bg_.set_texture(Textures::instance().get("bg/intro.png"));
    name_box_.create(7, 1, 27, 3, bty::BoxColor::Intro);
    name_box_.add_line(2, 1, kHeroNames[0][0]);
    help_box_.create(1, 24, 38, 3, bty::BoxColor::Intro);
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
                    hero = (hero - 1 + 4) % 4;
                    name_box_.set_line(0, kHeroNames[hero][0]);
                    break;
                case GLFW_KEY_RIGHT:
                    hero = (hero + 1) % 4;
                    name_box_.set_line(0, kHeroNames[hero][0]);
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
    (void)dt;
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
                ss.pop(0);
                ingame.setup(hero, opt);
                ss.push(&ingame, [this](int ret) {
                    switch (ret) {
                        case 0:    // reset
                            ss.push(this, nullptr);
                            break;
                        case 1:    // battle
                            break;
                        default:
                            break;
                    }
                });
            },
        },
    });
    dialog->set_selection(1);
    dialog->set_color(bty::BoxColor::Intro);
}
