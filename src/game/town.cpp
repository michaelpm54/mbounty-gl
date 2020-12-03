#include "game/town.hpp"

#include <fmt/format.h>

#include "assets.hpp"
#include "gfx/gfx.hpp"
#include "game/map.hpp"

void Town::load(bty::Assets &assets, bty::BoxColor color) {
    dialog_.create(1, 18, 30, 9, color, assets);
    dialog_.add_line(1, 1, "Town of X");
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
}

void Town::update(float dt) {
    dialog_.animate(dt);
    unit_.animate(dt);
}

void Town::prev() {
    dialog_.prev();
}

void Town::next() {
    dialog_.next();
}
