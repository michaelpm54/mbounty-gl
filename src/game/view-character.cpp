#include "game/view-character.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"

void ViewCharacter::load(bty::Assets &assets, bty::BoxColor color, int hero_id) {
    frame_.set_texture(assets.get_texture("frame/character.png"));
    frame_.set_position(0, 16);

    static const std::string kPortraitFilenames[4] = {
        "crimsaun",
        "palmer",
        "tynnestra",
        "moham",
    };

    for (int i = 0; i < 4; i++) {
        portraits_[i] = assets.get_texture(fmt::format("char-page/{}.png", kPortraitFilenames[i]));
    }

    portrait_.set_texture(portraits_[hero_id]);
    portrait_.set_position(8, 24);

    rect_.set_color(color);
    rect_.set_size(208, 104);
    rect_.set_position(104, 24);

    const auto &font = assets.get_font();
    for (int i = 0; i < 11; i++) {
        info_[i].set_font(font);
        info_[i].set_position(112, 40 + i * 8);
    }

    info_[0].set_position(112, 32);
}

void ViewCharacter::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    gfx.draw_sprite(frame_, camera);
    gfx.draw_sprite(portrait_, camera);
    gfx.draw_rect(rect_, camera);
    for (int i = 0; i < 11; i++) {
        gfx.draw_text(info_[i], camera);
    }
}

void ViewCharacter::view(const SharedState &state) {
    info_[0].set_string(kHeroNames[state.hero_id][state.hero_rank]);
    info_[1].set_string(fmt::format("Leadership {:>13}", state.leadership));
    info_[2].set_string(fmt::format("Commission/Week {:>8}", state.commission));
    info_[3].set_string(fmt::format("Gold {:>19}", state.gold));
    info_[4].set_string(fmt::format("Spell Power {:>12}", state.spell_power));
    info_[5].set_string(fmt::format("Max # of Spells {:>8}", state.max_spells));
    info_[6].set_string(fmt::format("Villains caught {:>8}", 0));
    info_[7].set_string(fmt::format("Artifacts found {:>8}", 0));
    info_[8].set_string(fmt::format("Castles garrisoned {:>5}", 0));
    info_[9].set_string(fmt::format("Followers killed {:>7}", 0));
    info_[10].set_string(fmt::format("Current score {:>10}", 0));
    portrait_.set_texture(portraits_[state.hero_id]);
}

void ViewCharacter::set_color(bty::BoxColor color) {
    rect_.set_color(color);
}
