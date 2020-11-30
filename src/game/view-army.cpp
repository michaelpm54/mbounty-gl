#include "game/view-army.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"

void ViewArmy::load(bty::Assets &assets, const glm::vec4 &color) {
    static constexpr const char *const kUnitNames[25] = {
        "units/archers.png",
        "units/archmages.png",
        "units/barbarians.png",
        "units/cavalries.png",
        "units/demons.png",
        "units/dragons.png",
        "units/druids.png",
        "units/dwarves.png",
        "units/elves.png",
        "units/ghosts.png",
        "units/giants.png",
        "units/gnomes.png",
        "units/knights.png",
        "units/militias.png",
        "units/nomads.png",
        "units/ogres.png",
        "units/orcs.png",
        "units/peasants.png",
        "units/pikemen.png",
        "units/skeletons.png",
        "units/sprites.png",
        "units/trolls.png",
        "units/vampires.png",
        "units/wolves.png",
        "units/zombies.png",
    };

    for (int i = 0; i < 25; i++) {
        unit_textures_[i] = assets.get_texture(kUnitNames[i], {2, 2});
    }

    frame_.set_texture(assets.get_texture("frame/army.png"));
    frame_.set_position(0, 16);

    for (int i = 0; i < 5; i++) {
        rects_[i].set_color(color);
        rects_[i].set_size(253, 32);
        rects_[i].set_position(59, 24 + i * 40);

        units_[i].set_position(8, 24 + i * 40);
    }
}

void ViewArmy::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    gfx.draw_sprite(frame_, camera);
    for (int i = 0; i < 5; i++) {
        gfx.draw_rect(rects_[i], camera);
    }
    for (int i = 0; i < num_units_; i++) {
        gfx.draw_sprite(units_[i], camera);
    }
}

void ViewArmy::view(const SharedState &state) {
    num_units_ = state.army_size;
    for (int i = 0; i < state.army_size; i++) {
        if (state.army[i] != -1) {
            units_[i].set_texture(unit_textures_[state.army[i]]);
        }
        else {
            spdlog::warn("ViewArmy::view: army[{}] is -1, size is wrong");
        }
    }
}

void ViewArmy::update(float dt) {
    for (int i = 0; i < num_units_; i++) {
        units_[i].animate(dt);
    }
}
