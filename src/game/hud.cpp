#include "game/hud.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "gfx/gfx.hpp"
#include "gfx/font.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"

void Hud::load(bty::Assets &assets, bty::Font &font, SharedState &state) {
    state_ = &state;
    frame_.set_texture(assets.get_texture("frame/game-hud.png"));

    top_bar_.set_size(304, 9);
    top_bar_.set_position({8, 7});

    top_bar_.set_color(bty::get_box_color(state.difficulty_level));

    font_ = &font;
    name_.create(1, 1, kHeroNames[state.hero_id][state.hero_rank], font);
    days_.create(26, 1, "", font);

    contract_textures_.resize(18);
    for (int i = 0, max = static_cast<int>(contract_textures_.size()-1); i < max; i++) {
        contract_textures_[i] = assets.get_texture(fmt::format("villains/{}.png", i), {4, 1});
    }
    contract_textures_.back() = assets.get_texture("villains/empty.png");
    contract_.set_texture(contract_textures_[0]);
    contract_.set_position({262, 24});

    siege_no = assets.get_texture("hud/siege-no.png");
    siege_yes = assets.get_texture("hud/siege-yes.png", {4, 1});
    siege_.set_position({262, 64});

    magic_no = assets.get_texture("hud/magic-no.png");
    magic_yes = assets.get_texture("hud/magic-yes.png", {4, 1});
    magic_.set_position({262, 104});

    puzzle_.set_texture(assets.get_texture("hud/puzzle-bg.png"));
    puzzle_.set_position({262, 144});

    money_.set_texture(assets.get_texture("hud/gold-bg.png"));
    money_.set_position({262, 184});

    update_state();
}

void Hud::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    gfx.draw_sprite(frame_, camera);
    gfx.draw_rect(top_bar_, camera);
    gfx.draw_text(name_, camera);
    gfx.draw_text(days_, camera);
    gfx.draw_sprite(contract_, camera);
    gfx.draw_sprite(siege_, camera);
    gfx.draw_sprite(magic_, camera);
    gfx.draw_sprite(puzzle_, camera);
    gfx.draw_sprite(money_, camera);
}

void Hud::update_state() {
    contract_.set_texture(contract_textures_[state_->contract]);
    name_.set_string(kHeroNames[state_->hero_id][state_->hero_rank]);
    days_.set_string(fmt::format("Days Left:{}", state_->days));
    siege_.set_texture(state_->siege ? siege_yes : siege_no);
    magic_.set_texture(state_->magic ? magic_yes : magic_no);
}

void Hud::update(float dt) {
    contract_.animate(dt);
    siege_.animate(dt);
    magic_.animate(dt);
}

void Hud::set_title(const std::string &msg) {
    name_.set_string(msg);
    days_.set_string("");
}
