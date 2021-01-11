#include "game/kings-castle.hpp"

#include <spdlog/spdlog.h>

#include "data/hero.hpp"
#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "engine/texture-cache.hpp"
#include "game/gen-variables.hpp"
#include "game/hud.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"

static constexpr int kKingsCastleUnits[5] = {
    Militias,
    Archers,
    Pikemen,
    Cavalries,
    Knights,
};

KingsCastle::KingsCastle(bty::SceneStack &ss, bty::DialogStack &ds, Hud &hud, Variables &v, GenVariables &gen)
    : ss(ss)
    , ds(ds)
    , hud(hud)
    , v(v)
    , gen(gen)
{
    dialog_.create(1, 18, 30, 9, bty::BoxColor::Intro);
    dialog_.add_line(1, 1, "Castle of King Maximus");
    dialog_.add_line(22, 2, "");    // Gold
    dialog_.add_option(3, 4, "Recruit soldiers");
    dialog_.add_option(3, 5, "Audience with the King");

    recruit_.create(1, 18, 30, 9, bty::BoxColor::Intro);
    recruit_.add_line(1, 1, "Recruit Soldiers");
    recruit_.add_line(22, 1, "");

    recruit_.add_line(17, 4, "Select army\nto recruit.");

    recruit_.add_line(17, 3, "You may get\nup to\n\nRecruit how\nmany?");
    recruit_.set_line_visible(3, false);
    may_get_ = recruit_.add_line(23, 4, "");
    recruit_.set_line_visible(4, false);
    how_many_ = recruit_.add_line(25, 7, "");
    recruit_.set_line_visible(5, false);

    unit_.set_position(64, 104);
    bg_.set_texture(Textures::instance().get("bg/castle.png"));
    bg_.set_position(8, 24);

    for (int i = 0; i < 5; i++) {
        unit_textures_[i] = Textures::instance().get(fmt::format("units/{}.png", kKingsCastleUnits[i]), {2, 2});
    }
}

void KingsCastle::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    if (show_recruit_) {
        recruit_.draw(gfx, camera);
    }
    else {
        dialog_.draw(gfx, camera);
    }
    gfx.draw_sprite(unit_, camera);
}

void KingsCastle::view()
{
    set_color(bty::get_box_color(v.diff));

    recruit_input_.clear();

    how_many_->set_string("  0");

    set_gold(v.gold);
    unit_.set_texture(unit_textures_[rand() % 5]);

    recruit_.clear_options();

    if (v.rank == 0) {
        recruit_.add_option(3, 3, "Militia   50");
        recruit_.add_option(3, 4, "Archers  250");
        recruit_.add_option(3, 5, "Pikemen  300");
        auto *a = recruit_.add_option(3, 6, "Cavalry  n/a");
        auto *b = recruit_.add_option(3, 7, "Knights  n/a");
        a->disable();
        b->disable();
    }
    else if (v.rank == 1) {
        recruit_.add_option(3, 3, "Militia   50");
        recruit_.add_option(3, 4, "Archers  250");
        recruit_.add_option(3, 5, "Pikemen  300");
        recruit_.add_option(3, 6, "Cavalry  800");
        auto *a = recruit_.add_option(3, 7, "Knights  n/a");
        a->disable();
    }
    else if (v.rank == 2 || v.rank == 3) {
        recruit_.add_option(3, 3, "Militia   50");
        recruit_.add_option(3, 4, "Archers  250");
        recruit_.add_option(3, 5, "Pikemen  300");
        recruit_.add_option(3, 6, "Cavalry  800");
        recruit_.add_option(3, 7, "Knights  1000");
    }
}

void KingsCastle::update(float dt)
{
    dialog_.update(dt);
    recruit_.update(dt);
    unit_.update(dt);
    recruit_input_.update(dt);
    how_many_->set_string(fmt::format("{:>3}", recruit_input_.get_current()));
}

void KingsCastle::key(int key, int action)
{
    if (show_recruit_amount_) {
        recruit_input_.key(key, action);
    }

    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_DOWN:
                    if (show_recruit_ && !show_recruit_amount_) {
                        recruit_.next();
                    }
                    else {
                        dialog_.next();
                    }
                    break;
                case GLFW_KEY_UP:
                    if (show_recruit_ && !show_recruit_amount_) {
                        recruit_.prev();
                    }
                    else {
                        dialog_.prev();
                    }
                    break;
                    break;
                case GLFW_KEY_ENTER:
                    if (show_recruit_) {
                        recruit_opt();
                    }
                    else {
                        main_opt();
                    }
                    break;
                case GLFW_KEY_BACKSPACE:
                    if (show_recruit_) {
                        if (show_recruit_amount_) {
                            show_recruit_amount_ = false;
                            how_many_->set_string("  0");
                            recruit_.set_line_visible(2, true);
                            recruit_.set_line_visible(3, false);
                            recruit_.set_line_visible(4, false);
                            recruit_.set_line_visible(5, false);
                        }
                        else {
                            show_recruit_ = false;
                        }
                        break;
                    }
                    else {
                        ss.pop(0);
                    }
                    break;
                default:
                    break;
            }
        default:
            break;
    }
}

void KingsCastle::set_gold(int gold)
{
    recruit_.set_line(1, fmt::format("GP={}", bty::number_with_ks(gold)));
}

void KingsCastle::set_color(bty::BoxColor color)
{
    dialog_.set_color(color);
    recruit_.set_color(color);
}

void KingsCastle::recruit_opt()
{
    int current = recruit_input_.get_current();

    if (!show_recruit_amount_) {
        show_recruit_amount_ = true;
        recruit_.set_line_visible(2, false);
        recruit_.set_line_visible(3, true);
        recruit_.set_line_visible(4, true);
        recruit_.set_line_visible(5, true);

        int id = kKingsCastleUnits[recruit_.get_selection()];
        int potential_amount = v.leadership / kUnits[id].hp;
        int existing_amount = 0;

        for (int i = 0; i < 5; i++) {
            if (v.army[i] == id) {
                existing_amount = v.counts[i];
                break;
            }
        }

        int max_amt = potential_amount > existing_amount ? potential_amount - existing_amount : 0;
        max_amt = std::min(max_amt, v.gold / kUnits[id].recruit_cost);
        recruit_input_.set_max(max_amt);
        may_get_->set_string(fmt::format("{}.", max_amt));

        if (kUnits[id].recruit_cost > v.gold) {
            hud.set_error("     You do not have enough gold!");
        }

        how_many_->set_string(fmt::format("{:>3}", current));
    }
    else {
        if (current > 0) {
            int id = kKingsCastleUnits[recruit_.get_selection()];

            int cost = current * kUnits[id].recruit_cost;
            v.gold -= cost;
            hud.set_gold(v.gold);
            set_gold(v.gold);
            show_recruit_amount_ = false;
            recruit_.set_line_visible(2, true);
            recruit_.set_line_visible(3, false);
            recruit_.set_line_visible(4, false);
            recruit_.set_line_visible(5, false);

            bool found = false;
            for (int i = 0; i < 5; i++) {
                if (v.army[i] == id) {
                    v.counts[i] += current;
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (int i = 0; i < 5; i++) {
                    if (v.army[i] == -1) {
                        v.army[i] = id;
                        v.counts[i] = current;
                        break;
                    }
                }
            }
        }
    }
}

void KingsCastle::main_opt()
{
    switch (dialog_.get_selection()) {
        case 0:
            show_recruit_ = true;
            break;
        case 1:
            show_audience();
            break;
        default:
            break;
    }
}

void KingsCastle::show_audience()
{
    int num_captured = 0;
    for (int i = 0; i < 17; i++) {
        if (gen.villains_captured[i]) {
            num_captured++;
        }
    }

    int required_villains = 0;
    if (v.rank < 3) {
        if (num_captured < kRankVillainsCaptured[v.hero][v.rank + 1]) {
            required_villains = kRankVillainsCaptured[v.hero][v.rank + 1] - num_captured;
        }
    }

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, R"raw(   Trumpets announce your
 arrival with regal fanfare.

   King Maximus rises from
   his throne to greet you
       and proclaims:)raw"},
        },
        .callbacks = {
            .confirm = [this, required_villains](int) {
                if (required_villains <= 0) {
                    ds.show_dialog({
                        .x = 1,
                        .y = 18,
                        .w = 30,
                        .h = 9,
                        .strings = {
                            {1, 1, fmt::format("{},", kHeroNames[v.hero][v.rank])},
                            {1, 3, fmt::format(R"raw(      Congratulations!

    I now promote you to
          {}.)raw",
                                               kHeroRankNames[v.hero][v.rank + 1])},
                        },
                    });
                    v.rank++;
                    hud.set_hero(v.hero, v.rank);
                }
                else {
                    ds.show_dialog({
                        .x = 1,
                        .y = 18,
                        .w = 30,
                        .h = 9,
                        .strings = {
                            {1, 1, fmt::format("{},", kHeroNames[v.hero][0])},
                            {1, 3, fmt::format(R"raw(    I can aid you better
   after you've captured {}
        more villains.)raw",
                                               required_villains)},
                        },
                    });
                }
            },
        },
    });
}
