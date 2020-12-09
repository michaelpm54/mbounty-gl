#include "game/town.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "game/map.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "shared-state.hpp"

void Town::load(bty::Assets &assets, bty::BoxColor color, SharedState &state)
{
    state_ = &state;

    dialog_.create(1, 18, 30, 9, color, assets);
    dialog_.add_line(1, 1, "");     // Town name
    dialog_.add_line(22, 2, "");    // Gold
    dialog_.add_option(3, 3, "Get contract");
    dialog_.add_option(3, 4, "");    // Rent boat
    dialog_.add_option(3, 5, "Gather information");
    dialog_.add_option(3, 6, "");    // Spell
    dialog_.add_option(3, 7, "Buy siege weapons (3000)");

    unit_.set_position(56, 104);
    bg_.set_texture(assets.get_texture("bg/town.png"));
    bg_.set_position(8, 24);
    for (int i = 0; i < 25; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }
    gather_information_.create(1, 18, 30, 9, color, assets);
    gather_information_.add_line(1, 1, "");
}

void Town::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    if (show_gather_information_) {
        gather_information_.draw(gfx, camera);
    }
    else {
        dialog_.draw(gfx, camera);
    }
    gfx.draw_sprite(unit_, camera);
}

void Town::view(int town, const Tile &tile, int continent, int unit_id, int spell)
{
    town_ = town;

    unit_.set_texture(unit_textures_[unit_id]);

    dialog_.set_line(0, fmt::format("Town of {}", kTownInfo[town].name));

    update_gold();

    int num_caught = 0;
    for (int i = 0; i < 17; i++) {
        if (state_->villains_caught[i]) {
            num_caught++;
        }
    }
    if (num_caught == 17) {
        dialog_.set_option_visibility(0, false);
    }

    if (!state_->boat_rented) {
        dialog_.set_option(1, fmt::format("Rent boat ({} week)", state_->artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500));
    }
    else {
        dialog_.set_option(1, "Cancel boat rental");
    }

    /*
    std::string army;
    for (int i = 0; i < 5; i++) {
        if (occ.army[i] != -1) {
            if (occ.army_counts[i] < 10) {
                army += fmt::format(" A few {}\n", kUnits[occ.army[i]].name_plural);
            }
            else if (occ.army_counts[i] < 20) {
                army += fmt::format(" Some {}\n", kUnits[occ.army[i]].name_plural);
            }
            else if (occ.army_counts[i] < 50) {
                army += fmt::format(" Many {}\n", kUnits[occ.army[i]].name_plural);
            }
            else if (occ.army_counts[i] < 100) {
                army += fmt::format(" A lot {}\n", kUnits[occ.army[i]].name_plural);
            }
            else if (occ.army_counts[i] < 500) {
                army += fmt::format(" A horde {}\n", kUnits[occ.army[i]].name_plural);
            }
            else {
                army += fmt::format(" A multitude {}\n", kUnits[occ.army[i]].name_plural);
            }
        }
    }

    std::string occupier = occ.occupier == -1 ? "no one" : ("\n" + std::string(kVillains[occ.occupier][0]));

    gather_information_.set_line(0, fmt::format(
                                        R"raw(Castle {}{}is under {}'s rule.
{})raw",
                                        kCastleInfo[occ.index].name,
                                        occ.occupier == -1 ? "\n" : " ",
                                        occupier,
                                        army));
    current_info_contract_ = occ.occupier;
	*/

    dialog_.set_option(3, fmt::format("{} spell ({})", kSpellNames[spell], kSpellCosts[spell]));
    dialog_.set_option_visibility(4, !state_->siege);
}

void Town::update(float dt)
{
    dialog_.animate(dt);
    unit_.animate(dt);
}

int Town::key(int key)
{
    switch (key) {
        case GLFW_KEY_DOWN:
            dialog_.next();
            break;
        case GLFW_KEY_UP:
            dialog_.prev();
            break;
        case GLFW_KEY_ENTER:
            if (show_gather_information_) {
                show_gather_information_ = false;
                return 2;
            }
            if (dialog_.get_selection() == 1) {
                if (state_->boat_rented) {
                    dialog_.set_option(1, fmt::format("Rent boat ({} week)", state_->artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500));
                }
                else {
                    dialog_.set_option(1, "Cancel boat rental");
                }
            }
            else if (dialog_.get_selection() == 2) {
                show_gather_information_ = true;
                if (current_info_contract_ != -1) {
                    state_->known_villains[current_info_contract_] = true;
                }
            }
            else if (dialog_.get_selection() == 4) {
                if (state_->gold >= 3000) {
                    dialog_.set_option_visibility(4, false);
                    return 4;
                }
            }
            return dialog_.get_selection();
        case GLFW_KEY_BACKSPACE:
            if (show_gather_information_) {
                show_gather_information_ = false;
                return -1;
            }
            return -2;
        default:
            break;
    }

    return -1;
}

int Town::get_town() const
{
    return town_;
}

void Town::update_gold()
{
    dialog_.set_line(1, fmt::format("GP={}", bty::number_with_ks(state_->gold)));
}

void Town::set_color(bty::BoxColor color)
{
    dialog_.set_color(color);
    gather_information_.set_color(color);
}
