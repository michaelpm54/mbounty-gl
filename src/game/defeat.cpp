#include "game/defeat.hpp"

#include <spdlog/spdlog.h>

#include "data/hero.hpp"
#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "game/hud.hpp"
#include "gfx/gfx.hpp"

Defeat::Defeat(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Hud &hud)
    : ss(ss)
    , ds(ds)
    , hud(hud)
{
    lose_pic.set_texture(assets.get_texture("bg/king-dead.png"));
    lose_pic.set_position(168, 24);
}

void Defeat::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(lose_pic, camera);
}

void Defeat::key(int key, int action)
{
}

void Defeat::update(float dt)
{
}

void Defeat::show(int hero)
{
    hud.set_blank_frame();

    ds.show_dialog({
        .x = 1,
        .y = 3,
        .w = 20,
        .h = 24,
        .strings = {
            {1, 2, fmt::format("Oh, {},", kShortHeroNames[hero])},
            {1, 4, R"raw(you have failed to
recover the
Sceptre of Order
in time to save
the land! Beloved
King Maximus has
died and the Demon
King Urthrax
Killspite rules in
his place.  The
Four Continents
lay in ruin about
you, its people
doomed to a life
of misery and
oppression because
you could not find
the Sceptre.)raw"},
        },
        .callbacks = {
            .confirm = [this](int opt) {
                hud.set_error("      Press Enter to play again.", [this]() {
                    spdlog::debug("Popping defeat info text");
                    ds.pop();
                    spdlog::debug("Popping defeat scene");
                    ss.pop(0);
                });
            },
        },
        .pop_on_confirm = false,
    });
}
