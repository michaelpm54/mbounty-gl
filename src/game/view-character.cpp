#include "game/view-character.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "data/hero.hpp"
#include "engine/assets.hpp"
#include "engine/scene-stack.hpp"
#include "game/gen-variables.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"
#include "window/glfw.hpp"

ViewCharacter::ViewCharacter(bty::SceneStack &ss, bty::Assets &assets)
    : ss(ss)
{
    frame_.set_texture(assets.get_texture("frame/character.png"));
    frame_.set_position(0, 16);

    static const std::string kPortraitFilenames[4] = {
        "crimsaun",
        "palmer",
        "tynnestra",
        "moham",
    };

    for (int i = 0; i < 8; i++) {
        artifacts_[i] = assets.get_texture(fmt::format("artifacts/36x32/{}.png", i));
        artifact_sprites_[i].set_position(14 + (i % 4) * 48, 136 + (i / 4) * 40);
        artifact_sprites_[i].set_texture(artifacts_[i]);
    }

    for (int i = 0; i < 4; i++) {
        portraits_[i] = assets.get_texture(fmt::format("char-page/{}.png", kPortraitFilenames[i]));
    }

    portrait_.set_position(8, 24);

    rect_.set_size(208, 104);
    rect_.set_position(104, 24);

    const auto &font = assets.get_font();
    for (int i = 0; i < 11; i++) {
        info_[i].set_font(font);
        info_[i].set_position(112, 40 + i * 8);
    }

    info_[0].set_position(112, 32);

    float x = 206;
    float y = 136;
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            x -= 112;
            y += 40;
        }
        map_sprites_[i].set_texture(assets.get_texture(fmt::format("maps/{}.png", i)));
        map_sprites_[i].set_position(x + i * 56, y);
    }
}

void ViewCharacter::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(frame_, camera);
    gfx.draw_sprite(portrait_, camera);
    gfx.draw_rect(rect_, camera);
    for (int i = 0; i < 11; i++) {
        gfx.draw_text(info_[i], camera);
    }
    for (int i = 0; i < 4; i++) {
        if (maps_found_[i]) {
            gfx.draw_sprite(map_sprites_[i], camera);
        }
    }
    for (int i = 0; i < 8; i++) {
        if (artifacts_found_[i]) {
            gfx.draw_sprite(artifact_sprites_[i], camera);
        }
    }
}

void ViewCharacter::update_info(const Variables &v, const GenVariables &gen)
{
    set_color(bty::get_box_color(v.diff));

    portrait_.set_texture(portraits_[v.hero]);

    maps_found_ = gen.sail_maps_found.data();
    artifacts_found_ = gen.artifacts_found.data();

    info_[0].set_string(kHeroNames[v.hero][v.rank]);
    info_[1].set_string(fmt::format("Leadership {:>13}", v.leadership));
    info_[2].set_string(fmt::format("Commission/Week {:>8}", v.commission));
    info_[3].set_string(fmt::format("Gold {:>19}", v.gold));
    info_[4].set_string(fmt::format("Spell Power {:>12}", v.spell_power));
    info_[5].set_string(fmt::format("Max # of Spells {:>8}", v.max_spells));
    info_[6].set_string(fmt::format("Villains caught {:>8}", 0));
    info_[7].set_string(fmt::format("Artifacts found {:>8}", 0));
    info_[8].set_string(fmt::format("Castles garrisoned {:>5}", 0));
    info_[9].set_string(fmt::format("Followers killed {:>7}", v.followers_killed));
    info_[10].set_string(fmt::format("Current score {:>10}", 0));
    portrait_.set_texture(portraits_[v.hero]);
}

void ViewCharacter::set_color(bty::BoxColor color)
{
    rect_.set_color(color);
}

void ViewCharacter::update(float dt)
{
}

void ViewCharacter::key(int key, int action)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_BACKSPACE:
                [[fallthrough]];
            case GLFW_KEY_ENTER:
                ss.pop(0);
                break;
            default:
                break;
        }
    }
}
