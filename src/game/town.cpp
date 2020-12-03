#include "game/town.hpp"

#include <spdlog/spdlog.h>
#include <GLFW/glfw3.h>

#include "shared-state.hpp"
#include "assets.hpp"
#include "gfx/gfx.hpp"
#include "game/map.hpp"
#include "game/hud.hpp"

void Town::load(bty::Assets &assets, bty::BoxColor color, SharedState &state, Hud &hud) {
    state_ = &state;
    hud_ = &hud;
    dialog_.create(1, 18, 30, 9, color, assets);
    dialog_.add_line(1, 1, "");
    dialog_.add_line(22, 2, "");
    dialog_.add_option(3, 3, "Get contract");
    dialog_.add_option(3, 4, "");
    unit_.set_position(56, 104);
    bg_.set_texture(assets.get_texture("bg/town.png"));
    bg_.set_position(8, 24);
    for (int i = 0; i < 25; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }
}

void Town::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    gfx.draw_sprite(bg_, camera);
    dialog_.draw(gfx, camera);
    gfx.draw_sprite(unit_, camera);
}

void Town::view(const Tile &tile, int continent, int unit_id) {

    unit_.set_texture(unit_textures_[unit_id]);

    for (int i = 0; i < kTownsPerContinent[continent]; i++) {
        if (tile.tx == kTownLocations[i].x && tile.ty == 63 - kTownLocations[i].y) {
            dialog_.set_line(0, fmt::format("Town of {}", kTownLocations[i].name));
        }
    }

    dialog_.set_line(1, fmt::format("GP={}", bty::number_with_ks(state_->gold)));

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
}

void Town::update(float dt) {
    dialog_.animate(dt);
    unit_.animate(dt);
}

int Town::key(int key) {
    switch (key) {
        case GLFW_KEY_DOWN:
            dialog_.next();
            break;
        case GLFW_KEY_UP:
            dialog_.prev();
            break;
        case GLFW_KEY_ENTER:
            if (dialog_.get_selection() == 1) {
                if (state_->boat_rented) {
                    dialog_.set_option(1, fmt::format("Rent boat ({} week)", state_->artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500));
                }
                else {
                    dialog_.set_option(1, "Cancel boat rental");
                }
            }
            return dialog_.get_selection();
        default:
            break;
    }

    return -1;
}


