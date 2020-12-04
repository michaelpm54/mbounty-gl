#include "game/kings-castle.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "gfx/gfx.hpp"
#include "shared-state.hpp"

void KingsCastle::load(bty::Assets &assets, bty::BoxColor color, SharedState &state)
{
    state_ = &state;

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
    amount_ = recruit_.add_line(23, 4, "");
    recruit_.set_line_visible(4, false);
    to_buy_ = recruit_.add_line(25, 7, "");
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
    added_while_holding_ = 0;
    add_amt_ = 1;
    increasing_amt_ = false;
    decreasing_amt_ = false;
    current_amt_ = 0;

    to_buy_->set_string("  0");

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
    unit_.animate(dt);

    if (increasing_amt_) {
        update_timer_ += dt;
        if (added_while_holding_ == 10) {
            add_amt_ = 10;
        }
    }
    else if (decreasing_amt_) {
        update_timer_ += dt;
        if (added_while_holding_ == -10) {
            add_amt_ = -10;
        }
    }

    if (update_timer_ > 0.1f) {
        current_amt_ += add_amt_;
        added_while_holding_ += add_amt_;
        update_timer_ = 0;
        if (current_amt_ < 0) {
            current_amt_ = 0;
        }
        to_buy_->set_string(fmt::format("{:>3}", current_amt_));
    }
}

int KingsCastle::key(int key, int action)
{
    switch (action) {
        case GLFW_RELEASE:
            switch (key) {
                case GLFW_KEY_RIGHT:
                    [[fallthrough]];
                case GLFW_KEY_UP:
                    increasing_amt_ = false;
                    add_amt_ = 0;
                    added_while_holding_ = 0;
                    break;
                case GLFW_KEY_LEFT:
                    [[fallthrough]];
                case GLFW_KEY_DOWN:
                    decreasing_amt_ = false;
                    add_amt_ = 0;
                    added_while_holding_ = 0;
                    break;
                default:
                    break;
            }
            break;
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_LEFT:
                    decreasing_amt_ = true;
                    add_amt_ = -1;
                    update_timer_ = 0.099f;
                    break;
                case GLFW_KEY_DOWN:
                    if (show_recruit_) {
                        if (show_recruit_amount_) {
                            decreasing_amt_ = true;
                            add_amt_ = -1;
                            update_timer_ = 0.099f;
                        }
                        else {
                            recruit_.next();
                        }
                    }
                    else {
                        dialog_.next();
                    }
                    break;
                case GLFW_KEY_UP:
                    if (show_recruit_) {
                        if (show_recruit_amount_) {
                            increasing_amt_ = true;
                            add_amt_ = 1;
                            update_timer_ = 0.099f;
                        }
                        else {
                            recruit_.prev();
                        }
                    }
                    else {
                        dialog_.prev();
                    }
                    break;
                case GLFW_KEY_RIGHT:
                    increasing_amt_ = true;
                    update_timer_ = 0.099f;
                    add_amt_ = 1;
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
                            current_amt_ = 0;
                            added_while_holding_ = 0;
                            add_amt_ = 0;
                            update_timer_ = 0;
                            show_recruit_amount_ = false;
                            to_buy_->set_string("  0");
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
    if (!show_recruit_amount_) {
        show_recruit_amount_ = true;
        recruit_.set_line_visible(2, false);
        recruit_.set_line_visible(3, true);
        recruit_.set_line_visible(4, true);
        recruit_.set_line_visible(5, true);
        amount_->set_string(fmt::format("{}.", state_->leadership / kUnits[recruit_.get_selection()].hp));
        return;
    }
    else {
        static constexpr int kKingsCastleUnits[5] = {
            Militias,
            Archers,
            Pikemen,
            Cavalries,
            Knights,
        };
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
