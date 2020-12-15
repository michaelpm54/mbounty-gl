#include "game/victory.hpp"

#include <fmt/format.h>

#include "data/hero.hpp"
#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "game/hud.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"

static constexpr const char *const kVictoryString = {
    "Congratulations,\n"
    "{}!  You\n"
    "have recovered the\n"
    "Sceptre of Order\n"
    "from the clutches\n"
    "of the evil Master\n"
    "Villains. As a\n"
    "reward for saving\n"
    "himself and the\n"
    "four continents\n"
    "from ruin, King\n"
    "Maximus and his\n"
    "subjects reward\n"
    "you with a large\n"
    "parcel of land,\n"
    "a rank of nobility\n"
    "and a medal\n"
    "announcing your\n"
    "\n"
    "Final Score: {:>5}",
};

Victory::Victory(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Variables &v, Hud &hud)
    : ss(ss)
    , ds(ds)
    , v(v)
    , hud(hud)
{
    bg.set_position(8, 24);
    bg.set_texture(assets.get_texture("battle/encounter.png"));

    king.set_position(8, 24);
    king.set_texture(assets.get_texture("bg/king-massive-smile.png"));

    hero.set_position(20.0f + 4 * 48.0f, 24.0f + 6 * 40.0f);
    hero.set_texture(assets.get_texture("hero/walk-moving.png", {4, 1}));
    hero.set_flip(true);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int index = j * 5 + i;
            float x = 16.0f + i * 48.0f;
            float y = 24.0f + j * 40.0f;
            if (i == 4) {
                x += 48.0f;
                unit_sprites[index].set_flip(true);
            }
            unit_sprites[index].set_position(x, y);
            unit_sprites[index].set_texture(assets.get_texture(fmt::format("units/{}.png", index), {2, 2}));
        }
    }
}

void Victory::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    if (parade) {
        gfx.draw_sprite(bg, camera);
        for (int i = 0; i < 25; i++) {
            gfx.draw_sprite(unit_sprites[i], camera);
        }
        gfx.draw_sprite(hero, camera);
    }
    else {
        gfx.draw_sprite(king, camera);
    }
    hud.draw(gfx, camera);
}

void Victory::update(float dt)
{
    if (parade) {
        for (int i = 0; i < 25; i++) {
            unit_sprites[i].update(dt);
        }
        hero.move(0.0f, -70.0f * dt);
        hero.update(dt);

        if (hero.get_position().y <= -140.0f) {
            parade = false;
            ds.show_dialog({
                .x = 19,
                .y = 3,
                .w = 20,
                .h = 24,
                .strings = {
                    {1, 2, fmt::format(kVictoryString, kShortHeroNames[v.hero], v.score)},
                },
                .callbacks = {
                    .confirm = [this](int) {
                        hud.set_error("      Press Enter to play again.", [this]() {
                            ds.pop();
                            ss.pop(0);
                        });
                    },
                },
                .pop_on_confirm = false,
            });
        }
    }
}

void Victory::key(int key, int action)
{
    (void)key;
    (void)action;
}
