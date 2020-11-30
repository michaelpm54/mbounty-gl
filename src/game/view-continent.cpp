#include "game/view-continent.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"

void ViewContinent::load(bty::Assets &assets, bty::BoxColor color, const bty::Font &font, const std::array<const bty::Texture *, 8> &border_textures) {
    box_.create(10, 7, 12, 16, color, border_textures, font);
}

void ViewContinent::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    box_.draw(gfx, camera);
}

void ViewContinent::view(const SharedState &state) {
    
}

void ViewContinent::update(float dt) {

}
