#include "game/kings-castle.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "game/hud.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "shared-state.hpp"

void KingsCastle::load(bty::Assets &assets, bty::BoxColor color, SharedState &state, Hud &hud)
{
    state_ = &state;
    hud_ = &hud;

    dialog_.create(1, 18, 30, 9, color, assets);
    dialog_.add_line(1, 1, "Castle of King Maximus");
    dialog_.add_line(22, 2, "");    // Gold
    dialog_.add_option(3, 4, "Recruit soldiers");
    dialog_.add_option(3, 5, "Audience with the King");

    recruit_.create(1, 18, 30, 9, color, assets);
    recruit_.add_line(1, 1, "Recruit Soldiers");
    recruit_.add_line(22, 1, "");

    recruit_.add_line(17, 4, "Select army\nto recruit.");

    recruit_.add_line(17, 3, "You may get\nup to\n\nRecruit how\nmany?");
    recruit_.set_line_visible(3, false);
    may_get_ = recruit_.add_line(23, 4, "");
    recruit_.set_line_visible(4, false);
    how_many_ = recruit_.add_line(25, 7, "");
    recruit_.set_line_visible(5, false);

    unit_.set_position(56, 104);
    bg_.set_texture(assets.get_texture("bg/castle.png"));
    bg_.set_position(8, 24);

    static constexpr int kKingsCastleUnits[5] = {
        Militias,
        Archers,
        Pikemen,
        Cavalries,
        Knights,
    };

    for (int i = 0; i < 5; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", kKingsCastleUnits[i]), {2, 2});
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
    recruit_input_.clear();

    how_many_->set_string("  0");

    update_gold();
    unit_.set_texture(unit_textures_[rand() % 5]);

    recruit_.clear_options();

    switch (state_->hero_rank) {
        case 0:
            recruit_.add_option(3, 3, "Militia   50");
            recruit_.add_option(3, 4, "Archers  250");
            recruit_.add_option(3, 5, "Pikemen  300");
            recruit_.add_option(3, 6, "Cavalry  n/a");
            recruit_.add_option(3, 7, "Knights  n/a");
            recruit_.set_option_disabled(3, true);
            recruit_.set_option_disabled(4, true);
            break;
        case 1:
            recruit_.add_option(3, 3, "Militia   50");
            recruit_.add_option(3, 4, "Archers  250");
            recruit_.add_option(3, 5, "Pikemen  300");
            recruit_.add_option(3, 6, "Cavalry  800");
            recruit_.add_option(3, 7, "Knights  n/a");
            recruit_.set_option_disabled(4, true);
            break;
        case 2:
            [[fallthrough]];
        case 3:
            recruit_.add_option(3, 3, "Militia   50");
            recruit_.add_option(3, 4, "Archers  250");
            recruit_.add_option(3, 5, "Pikemen  300");
            recruit_.add_option(3, 6, "Cavalry  800");
            recruit_.add_option(3, 7, "Knights  1000");
            break;
        default:
            break;
    }
}

void KingsCastle::update(float dt)
{
    dialog_.animate(dt);
    recruit_.animate(dt);
    unit_.animate(dt);
    recruit_input_.update(dt);
    how_many_->set_string(fmt::format("{:>3}", recruit_input_.get_current()));
}

int KingsCastle::key(int key, int action)
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
                    return -1;
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
                        return -1;
                    }
                    return -2;
                default:
                    break;
            }
        default:
            break;
    }

    return -1;
}

void KingsCastle::update_gold()
{
    recruit_.set_line(1, fmt::format("GP={}", bty::number_with_ks(state_->gold)));
}

void KingsCastle::set_color(bty::BoxColor color)
{
    dialog_.set_color(color);
}

void KingsCastle::recruit_opt()
{
    static constexpr int kKingsCastleUnits[5] = {
        Militias,
        Archers,
        Pikemen,
        Cavalries,
        Knights,
    };

    int current = recruit_input_.get_current();

    if (!show_recruit_amount_) {
        show_recruit_amount_ = true;
        recruit_.set_line_visible(2, false);
        recruit_.set_line_visible(3, true);
        recruit_.set_line_visible(4, true);
        recruit_.set_line_visible(5, true);

        int id = kKingsCastleUnits[recruit_.get_selection()];
        int potential_amount = state_->leadership / kUnits[id].hp;
        int existing_amount = 0;

        for (int i = 0; i < 5; i++) {
            if (state_->army[i] == id) {
                existing_amount = state_->army_counts[i];
                break;
            }
        }

        int max_amt = potential_amount > existing_amount ? potential_amount - existing_amount : 0;
        max_amt = std::min(max_amt, state_->gold / kUnits[id].recruit_cost);
        recruit_input_.set_max(max_amt);
        may_get_->set_string(fmt::format("{}.", max_amt));

        if (kUnits[id].recruit_cost > state_->gold) {
            hud_->set_title("     You do not have enough gold!");
        }

        how_many_->set_string(fmt::format("{:>3}", current));
    }
    else {
        if (current > 0) {
            int id = kKingsCastleUnits[recruit_.get_selection()];

            int cost = current * kUnits[id].recruit_cost;
            state_->gold -= cost;
            hud_->update_state();
            update_gold();
            show_recruit_amount_ = false;
            recruit_.set_line_visible(2, true);
            recruit_.set_line_visible(3, false);
            recruit_.set_line_visible(4, false);
            recruit_.set_line_visible(5, false);

            bool found = false;
            for (int i = 0; i < 5; i++) {
                if (state_->army[i] == id) {
                    state_->army_counts[i] += current;
                    found = true;
                    break;
                }
            }

            if (!found) {
                for (int i = 0; i < 5; i++) {
                    if (state_->army[i] == -1) {
                        state_->army[i] = id;
                        state_->army_counts[i] = current;
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
            show_audience_ = true;
            break;
        default:
            break;
    }
}
