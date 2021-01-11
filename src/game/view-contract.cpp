#include "game/view-contract.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "data/castles.hpp"
#include "data/villains.hpp"
#include "engine/scene-stack.hpp"
#include "engine/texture-cache.hpp"
#include "game/gen-variables.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"
#include "window/window-engine-interface.hpp"

ViewContract::ViewContract(bty::SceneStack &ss, Variables &v, GenVariables &gen, bty::Sprite *contract_sprite)
    : ss(ss)
    , v(v)
    , gen(gen)
    , contract_sprite_(contract_sprite)
{
    box_.create(1, 3, 30, 24, bty::BoxColor::Intro);
    no_contract_text_.create(6, 14, "You have no contract.");
    info_.create(2, 9, "");
}

void ViewContract::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
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

void ViewContract::update_info()
{
    set_color(bty::get_box_color(v.diff));

    if (v.contract == 17) {
        no_contract_ = true;
        return;
    }

    int castle = -1;

    if (v.contract < 17 && gen.villains_found[v.contract]) {
        for (int i = 0; i < 26; i++) {
            if (gen.castle_occupants[i] == v.contract) {
                castle = i;
                break;
            }
        }
    }

    std::string castle_name = castle == -1 ? "Unknown" : kCastleInfo[castle].name;

    std::string reward = std::to_string(kVillainRewards[v.contract]);
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
        kVillains[v.contract][0],
        kVillains[v.contract][1],
        reward,
        kContinentNames[kVillainContinents[v.contract]],
        castle_name,
        kVillains[v.contract][2],
        kVillains[v.contract][3]));
}

void ViewContract::set_color(bty::BoxColor color)
{
    box_.set_color(color);
}

void ViewContract::key(int key, int action)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ENTER:
                [[fallthrough]];
            case GLFW_KEY_BACKSPACE:
                ss.pop(0);
                break;
            default:
                break;
        }
    }
}

void ViewContract::update(float)
{
}
