#include "game/shop.hpp"

#include <spdlog/spdlog.h>

#include "data/shop.hpp"
#include "data/tiles.hpp"
#include "engine/assets.hpp"
#include "engine/scene-stack.hpp"
#include "game/hud.hpp"
#include "game/map.hpp"
#include "game/shop-info.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"

enum Dwellings {
    Cave,
    Forest,
    Dungeon,
    Plains,
};

static constexpr const char *const kShopNames[] = {
    "Cave",
    "Forest",
    "Dungeon",
    "Plains",
};

Shop::Shop(bty::SceneStack &ss, bty::Assets &assets, Variables &v, Hud &hud)
    : ss(ss)
    , v(v)
    , hud(hud)
{
    box_.create(1, 18, 30, 9, bty::BoxColor::Intro, assets);
    box_.add_line(1, 1, "");     // Shop name
    box_.add_line(1, 2, "");     // underline
    box_.add_line(1, 4, "");     // available
    box_.add_line(1, 5, "");     // cost
    box_.add_line(22, 5, "");    // gp
    box_.add_line(1, 6, "");     // you may recuit up to
    box_.add_line(1, 7, "Recruit how many?");
    box_.add_line(25, 7, "");    // current

    unit_.set_position(64, 104);
    bg_.set_position(8, 24);
    for (int i = 0; i < 25; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    static constexpr const char *const kShopImages[] = {
        "bg/cave.png",
        "bg/forest.png",
        "bg/dungeon.png",
        "bg/plains.png",
    };

    for (int i = 0; i < 4; i++) {
        dwelling_textures_[i] = assets.get_texture(kShopImages[i]);
    }
}

void Shop::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    gfx.draw_sprite(unit_, camera);
    box_.draw(gfx, camera);
}

void Shop::view(ShopInfo &info)
{
    if (info.unit == -1) {
        spdlog::debug("Shop: invalid unit ID");
        return;
    }

    set_color(bty::get_box_color(v.diff));

    info_ = &info;

    recruit_input_.clear();

    unit_.set_texture(unit_textures_[info.unit]);

    static constexpr int kTileToTypeMapping[] = {
        Tile_ShopCave,
        Tile_ShopTree,
        Tile_ShopDungeon,
        Tile_ShopWagon,
    };

    int shop_type = 0;
    for (int i = 0; i < 4; i++) {
        if (kShopTileForUnit[info.unit] == kTileToTypeMapping[i]) {
            shop_type = i;
            break;
        }
    }

    const auto &unit = kUnits[info.unit];

    int already_have = 0;
    for (int i = 0; i < 5; i++) {
        if (v.army[i] == info.unit) {
            already_have = v.counts[i];
            break;
        }
    }

    int recruitable = (v.leadership / unit.hp) - already_have;
    recruitable = std::min(v.gold / unit.recruit_cost, recruitable);

    const std::string shop_name = kShopNames[shop_type];
    int num_spaces = 14 - (shop_name.size() / 2);
    std::string spaces = std::string(num_spaces, ' ');

    box_.set_line(0, spaces + shop_name);
    box_.set_line(1, spaces + std::string(shop_name.size(), '_'));
    box_.set_line(2, fmt::format("{} {} are available.", info.count, kUnits[info.unit].name_plural));
    box_.set_line(3, fmt::format("Cost = {} each.", kUnits[info.unit].recruit_cost));
    box_.set_line(4, fmt::format("GP={}", bty::number_with_ks(v.gold)));
    box_.set_line(5, fmt::format("You may recruit up to {}.", recruitable));
    recruit_input_.set_max(recruitable);

    bg_.set_texture(dwelling_textures_[shop_type]);
}

void Shop::update(float dt)
{
    unit_.update(dt);
    recruit_input_.update(dt);
    box_.set_line(7, fmt::format("{:>3}", recruit_input_.get_current()));
}

void Shop::key(int key, int action)
{
    recruit_input_.key(key, action);

    switch (key) {
        case GLFW_KEY_ENTER:
            if (action == GLFW_PRESS) {
                confirm();
            }
            break;
        case GLFW_KEY_BACKSPACE:
            ss.pop(0);
            break;
        default:
            break;
    }
}

void Shop::set_color(bty::BoxColor color)
{
    box_.set_color(color);
}

void Shop::confirm()
{
    if (info_->unit == -1 || !info_) {
        return;
    }

    int recruit_cost = kUnits[info_->unit].recruit_cost;
    int current = recruit_input_.get_current();

    if (recruit_cost > v.gold) {
        hud.set_error("     You do not have enough gold!");
    }
    else if (current > 0) {
        int cost = current * recruit_cost;
        v.gold -= cost;
        hud.set_gold(v.gold);
        info_->count -= current;

        /* Add the amount to the existing one. */
        bool already_have = false;
        for (int i = 0; i < 5; i++) {
            if (v.army[i] == info_->unit) {
                v.counts[i] += current;
                already_have = true;
                break;
            }
        }

        /* Find the first -1 unit and set it to the new one. */
        if (!already_have) {
            for (int i = 0; i < 5; i++) {
                if (v.army[i] == -1) {
                    v.army[i] = info_->unit;
                    v.counts[i] = current;
                    break;
                }
            }
        }

        /* Update the shop info. */
        const auto &unit = kUnits[info_->unit];

        int recruitable = (v.leadership / unit.hp) - already_have;
        recruitable = std::min(v.gold / unit.recruit_cost, recruitable);

        box_.set_line(2, fmt::format("{} {} are available.", info_->count, kUnits[info_->unit].name_plural));
        box_.set_line(4, fmt::format("GP={}", bty::number_with_ks(v.gold)));
        box_.set_line(5, fmt::format("You may recruit up to {}.", recruitable));
        recruit_input_.set_max(recruitable);
    }
}
