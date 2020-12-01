#include "game/view-contract.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"

void ViewContract::load(bty::Assets &assets, bty::BoxColor color) {
    box_.create(1, 3, 30, 24, color, assets);
    no_contract_text_.create(6, 14, "You have no contract.", assets.get_font());
}

void ViewContract::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    box_.draw(gfx, camera);
    if (no_contract_) {
        gfx.draw_text(no_contract_text_, camera);
    }
}

void ViewContract::view(int contract, bool known_location) {
    no_contract_ = contract == 17;
}

void ViewContract::update(float dt) {

}
