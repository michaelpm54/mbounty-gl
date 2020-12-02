#include "game/view-army.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"

void ViewArmy::load(bty::Assets &assets, bty::BoxColor color) {
    for (int i = 0; i < 25; i++) {
        auto name = kUnits[i].name_plural;
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c){ return std::tolower(c);
        });
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", name), {2, 2});
    }

    frame_.set_texture(assets.get_texture("frame/army.png"));
    frame_.set_position(0, 16);

    const auto &font = assets.get_font();

    for (int i = 0; i < 5; i++) {
        rects_[i].set_color(color);
        rects_[i].set_size(253, 32);
        rects_[i].set_position(59, 24 + i * 40);
        units_[i].set_position(8, 24 + i * 40);

        for (int j = 0; j < 7; j++) {
            info_[i][j].set_font(font);
        }

        float x = 64;
        float y = 32 + i * 40;
        info_[i][0].set_position(x,       y);
        info_[i][1].set_position(x + 120, y);
        info_[i][2].set_position(x,       y + 8);
        info_[i][3].set_position(x + 56,  y + 8);
        info_[i][4].set_position(x + 120, y + 8);
        info_[i][5].set_position(x,       y + 16);
        info_[i][6].set_position(x + 120, y + 16);
    }
}

void ViewArmy::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    gfx.draw_sprite(frame_, camera);
    for (int i = 0; i < 5; i++) {
        gfx.draw_rect(rects_[i], camera);
    }
    for (int i = 0; i < num_units_; i++) {
        gfx.draw_sprite(units_[i], camera);
        for (int j = 0; j < 7; j++) {
            gfx.draw_text(info_[i][j], camera);
        }
    }
}

void ViewArmy::view(const SharedState &state) {
    num_units_ = 0;

    for (int i = 0; i < 5; i++) {
        if (state.army[i] != -1) {
            num_units_++;
        }
    }
    
    for (int i = 0; i < num_units_; i++) {
        int unit_id = state.army[i];
        const auto &unit = kUnits[unit_id];
        if (unit_id != -1) {
            int hp = state.army_counts[i] * unit.hp;
            int min_dmg = state.army_counts[i] * unit.melee_damage_min;
            int max_dmg = state.army_counts[i] * unit.melee_damage_max;
            int g_cost = state.army_counts[i] * unit.weekly_cost;

            std::string morale = [](int morale) {
                switch (morale) {
                    case 0:
                        return "Morale: Low";
                    case 1:
                        return "Morale: Medium";
                    case 2:
                        return "Morale: High";
                    default:
                        break;
                }
                return "Out of control";
            }(state.army_morales[i]);

            units_[i].set_texture(unit_textures_[unit_id]);
            info_[i][0].set_string(fmt::format("{} {}", state.army_counts[i], unit.name_plural));
            info_[i][1].set_string(fmt::format("HitPts: {}", hp));
            info_[i][2].set_string(fmt::format("SL: {}", unit.skill_level));
            info_[i][3].set_string(fmt::format("MV: {}", unit.initial_moves));
            info_[i][4].set_string(fmt::format("Damage: {}-{}", min_dmg, max_dmg));
            info_[i][5].set_string(fmt::format(morale));
            info_[i][6].set_string(fmt::format("G-Cost: {}", g_cost));
        }
    }
}

void ViewArmy::update(float dt) {
    for (int i = 0; i < num_units_; i++) {
        units_[i].animate(dt);
    }
}

void ViewArmy::set_color(bty::BoxColor color) {
    for (int i = 0; i < 5; i++) {
        rects_[i].set_color(color);
    }
}
