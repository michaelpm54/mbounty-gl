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
    info_.create(2, 9, "", assets.get_font());
}

void ViewContract::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    box_.draw(gfx, camera);
    if (no_contract_) {
        gfx.draw_text(no_contract_text_, camera);
    }
    else if (contract_sprite_) {
        auto pos = contract_sprite_->get_position();
        contract_sprite_->set_position(104, 40);
        gfx.draw_sprite(*contract_sprite_, camera);
        contract_sprite_->set_position(pos);
        gfx.draw_text(info_, camera);
    }
}

void ViewContract::view(int contract, bool known_location, bty::Sprite *contract_sprite) {
    no_contract_ = contract == 17;
    contract_sprite_ = contract_sprite;
    
    if (!no_contract_) {
        std::string reward = std::to_string(kVillainRewards[contract]);
        int n = reward.length() - 3;
        while (n > 0) {
            reward.insert(n, ",");
            n -= 3;
        }
        
        info_.set_string(fmt::format(
            R"raw(
Name: {}
Alias: {}
Reward: {} gold
Last seen: {}
Castle: {}
Distinguishing Features: {}
Crimes: {}
            )raw",
            kVillains[contract][0],
            kVillains[contract][1],
            reward,
            kContinents[kVillainContinents[contract]],
            "Unknown",
            kVillains[contract][2],
            kVillains[contract][3])
        );
    }
}

void ViewContract::update(float dt) {

}
