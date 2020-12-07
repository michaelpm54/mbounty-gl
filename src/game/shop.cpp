#include "game/shop.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "game/hud.hpp"
#include "game/map.hpp"
#include "game/shop-info.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "shared-state.hpp"

enum Dwellings {
    Cave,
    Forest,
    Dungeon,
    Plains,
};

static constexpr int kShopTileForUnit[] = {
    Tile_ShopWagon,      // peasants
    Tile_ShopTree,       // militia
    Tile_ShopTree,       // sprites
    Tile_ShopWagon,      // wolves
    Tile_ShopDungeon,    // skeletons
    Tile_ShopDungeon,    // zombies
    Tile_ShopTree,       // gnomes
    Tile_ShopCave,       // orcs
    Tile_ShopTree,       // archers
    Tile_ShopTree,       // elves
    Tile_ShopWagon,      // pikemen
    Tile_ShopWagon,      // nomads
    Tile_ShopCave,       // dwarves
    Tile_ShopDungeon,    // ghosts
    Tile_ShopTree,       // knights
    Tile_ShopCave,       // ogres
    Tile_ShopWagon,      // barbarians
    Tile_ShopCave,       // trolls
    Tile_ShopTree,       // cavalries
    Tile_ShopTree,       // druids
    Tile_ShopTree,       // archmages
    Tile_ShopDungeon,    // vampires
    Tile_ShopWagon,      // giants
    Tile_ShopDungeon,    // demons
    Tile_ShopCave,       // dragons
};

static constexpr const char *const kShopNames[] = {
    "Cave",
    "Forest",
    "Dungeon",
    "Plains",
};

void Shop::load(bty::Assets &assets, bty::BoxColor color, SharedState &state, Hud &hud)
{
    state_ = &state;
    hud_ = &hud;

    box_.create(1, 18, 30, 9, color, assets);
    box_.add_line(1, 1, "");     // Shop name
    box_.add_line(1, 2, "");     // underline
    box_.add_line(1, 4, "");     // available
    box_.add_line(1, 5, "");     // cost
    box_.add_line(22, 5, "");    // gp
    box_.add_line(1, 6, "");     // you may recuit up to
    box_.add_line(1, 7, "Recruit how many?");
    box_.add_line(25, 7, "");    // current

    unit_.set_position(56, 104);
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
        if (state_->army[i] == info.unit) {
            already_have = state_->army_counts[i];
            break;
        }
    }

    int recruitable = (state_->leadership / unit.hp) - already_have;
    recruitable = std::min(state_->gold / unit.recruit_cost, recruitable);

    const std::string shop_name = kShopNames[shop_type];
    int num_spaces = 14 - (shop_name.size() / 2);
    std::string spaces = std::string(num_spaces, ' ');

    box_.set_line(0, spaces + shop_name);
    box_.set_line(1, spaces + std::string(shop_name.size(), '_'));
    box_.set_line(2, fmt::format("{} {} are available.", info.count, kUnits[info.unit].name_plural));
    box_.set_line(3, fmt::format("Cost = {} each.", kUnits[info.unit].recruit_cost));
    box_.set_line(4, fmt::format("GP={}", bty::number_with_ks(state_->gold)));
    box_.set_line(5, fmt::format("You may recruit up to {}.", recruitable));
    recruit_input_.set_max(recruitable);

    bg_.set_texture(dwelling_textures_[shop_type]);
}

void Shop::update(float dt)
{
    unit_.animate(dt);
    recruit_input_.update(dt);
    box_.set_line(7, fmt::format("{:>3}", recruit_input_.get_current()));
}

int Shop::key(int key, int action)
{
    recruit_input_.key(key, action);

    switch (key) {
        case GLFW_KEY_ENTER:
            if (action == GLFW_PRESS) {
                confirm();
            }
            return 0;
        case GLFW_KEY_BACKSPACE:
            return -2;
        default:
            break;
    }
    return -1;
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

    if (recruit_cost > state_->gold) {
        hud_->set_title("     You do not have enough gold!");
    }
    else if (current > 0) {
        int cost = current * recruit_cost;
        state_->gold -= cost;
        hud_->update_state();
        info_->count -= current;

        int already_have = 0;
        for (int i = 0; i < 5; i++) {
            if (state_->army[i] == info_->unit) {
                already_have = state_->army_counts[i];
                break;
            }
        }

        bool found = false;
        for (int i = 0; i < 5; i++) {
            if (state_->army[i] == info_->unit) {
                state_->army_counts[i] += current;
                found = true;
                break;
            }
        }

        if (!found) {
            for (int i = 0; i < 5; i++) {
                if (state_->army[i] == -1) {
                    state_->army[i] = info_->unit;
                    state_->army_counts[i] = current;
                    break;
                }
            }
        }

        for (int i = 0; i < 5; i++) {
            if (state_->army[i] == info_->unit) {
                already_have = state_->army_counts[i];
                break;
            }
        }

        const auto &unit = kUnits[info_->unit];

        int recruitable = (state_->leadership / unit.hp) - already_have;
        recruitable = std::min(state_->gold / unit.recruit_cost, recruitable);

        box_.set_line(2, fmt::format("{} {} are available.", info_->count, kUnits[info_->unit].name_plural));
        box_.set_line(4, fmt::format("GP={}", bty::number_with_ks(state_->gold)));
        box_.set_line(5, fmt::format("You may recruit up to {}.", recruitable));
        recruit_input_.set_max(recruitable);
    }
}
