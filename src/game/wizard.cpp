#include "game/wizard.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "game/dialog-stack.hpp"
#include "game/hud.hpp"
#include "game/scene-stack.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"

Wizard::Wizard(SceneStack &ss, bty::Assets &assets, Variables &v, Hud &hud)
    : ss(ss)
    , v(v)
    , hud(hud)
{
    unit_.set_position(64, 104);
    unit_.set_texture(assets.get_texture("units/6.png", {2, 2}));
    bg_.set_position(8, 24);
    bg_.set_texture(assets.get_texture("bg/cave.png"));
}

void Wizard::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    gfx.draw_sprite(unit_, camera);
}

void Wizard::view(DialogStack &ds)
{
    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, R"raw(   The venerable Archmage
Aurange, will teach you the
secrets of spell casting for
   only 5000 gold pieces.)raw"},
        },
        .options = {
            {13, 6, "Accept"},
            {13, 7, "Decline"},
        },
        .callbacks = {
            .confirm = [this, &ds](int opt) {
                if (opt == 0) {
                    if (v.gold >= 5000) {
                        v.gold -= 5000;
                        v.magic = true;
                        hud.set_gold(v.gold);
                        hud.set_magic(true);
                        ss.pop(0);
                    }
                    else {
                        ds.show_dialog({
                            .x = 1,
                            .y = 18,
                            .w = 30,
                            .h = 9,
                            .strings = {
                                {1, 1, R"raw(  The sign said 5000 gold!
 Why waste my valuable time
when you know you don't have
the required amount of gold?

    Begone until you do!)raw"},
                            },
                            .callbacks = {
                                .confirm = [this](int opt) {
                                    ss.pop(1);
                                },
                            },
                        });
                    }
                }
                else {
                    ss.pop(1);
                }
            },
            .back = [this]() {
                ss.pop(1);
            },
        },
    });
}

void Wizard::update(float dt)
{
    unit_.update(dt);
}

void Wizard::key(int key, int action)
{
    switch (key) {
        case GLFW_KEY_BACKSPACE:
            ss.pop(0);
            break;
        default:
            break;
    }
}
