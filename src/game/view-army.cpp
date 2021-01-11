#include "game/view-army.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>

#include "data/bounty.hpp"
#include "engine/scene-stack.hpp"
#include "engine/texture-cache.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"
#include "window/glfw.hpp"

ViewArmy::ViewArmy(bty::SceneStack &ss)
    : ss(ss)
{
    auto &textures {Textures::instance()};

    for (int i = 0; i < 25; i++) {
        unit_textures_[i] = textures.get(fmt::format("units/{}.png", i), {2, 2});
    }

    frame_.set_texture(textures.get("frame/army.png"));
    frame_.set_position(0, 16);

    const auto &font = textures.get_font();

    for (int i = 0; i < 5; i++) {
        rects_[i].set_color(bty::BoxColor::Intro);
        rects_[i].set_size(253, 32);
        rects_[i].set_position(59, 24.0f + i * 40);
        units_[i].set_position(8, 24.0f + i * 40);

        for (int j = 0; j < 7; j++) {
            info_[i][j].set_font(font);
        }

        float x = 64;
        float y = 32.0f + i * 40;
        info_[i][0].set_position(x, y);
        info_[i][1].set_position(x + 120, y);
        info_[i][2].set_position(x, y + 8);
        info_[i][3].set_position(x + 56, y + 8);
        info_[i][4].set_position(x + 120, y + 8);
        info_[i][5].set_position(x, y + 16);
        info_[i][6].set_position(x + 120, y + 16);
    }
}

void ViewArmy::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
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

void ViewArmy::update_info(int *army, int *counts, int *morales, int diff)
{
    set_color(bty::get_box_color(diff));

    num_units_ = 0;

    for (int i = 0; i < 5; i++) {
        if (army[i] != -1) {
            num_units_++;
        }
    }

    for (int i = 0; i < num_units_; i++) {
        int unit_id = army[i];
        const auto &unit = kUnits[unit_id];
        if (unit_id != -1) {
            int hp = counts[i] * unit.hp;
            int min_dmg = counts[i] * unit.melee_damage_min;
            int max_dmg = counts[i] * unit.melee_damage_max;
            int g_cost = counts[i] * unit.weekly_cost;

            static const std::array<std::string, 4> kMoraleStrings = {{
                "Morale: Norm",
                "Morale: Low",
                "Morale: High",
                "Out of Control",
            }};

            assert(unit_id < 25);
            units_[i].set_texture(unit_textures_[unit_id]);
            info_[i][0].set_string(fmt::format("{} {}", counts[i], unit.name_plural));
            info_[i][1].set_string(fmt::format("HitPts: {}", hp));
            info_[i][2].set_string(fmt::format("SL: {}", unit.skill_level));
            info_[i][3].set_string(fmt::format("MV: {}", unit.initial_moves));
            info_[i][4].set_string(fmt::format("Damage: {}-{}", min_dmg, max_dmg));
            info_[i][5].set_string(fmt::format(kMoraleStrings[morales[i]]));
            info_[i][6].set_string(fmt::format("G-Cost: {}", g_cost));
        }
    }
}

void ViewArmy::update(float dt)
{
    for (int i = 0; i < num_units_; i++) {
        units_[i].update(dt);
    }
}

void ViewArmy::set_color(bty::BoxColor color)
{
    for (int i = 0; i < 5; i++) {
        rects_[i].set_color(color);
    }
}

void ViewArmy::key(int key, int action)
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
