#include "game/town.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "game/dialog-stack.hpp"
#include "game/gen-variables.hpp"
#include "game/hud.hpp"
#include "game/map.hpp"
#include "game/scene-stack.hpp"
#include "game/variables.hpp"
#include "game/view-contract.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"

Town::Town(SceneStack &ss, DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, Hud &hud, ViewContract &view_contract)
    : ss(ss)
    , ds(ds)
    , v(v)
    , gen(gen)
    , hud(hud)
    , view_contract(view_contract)
{
    unit_.set_position(64, 104);
    bg_.set_texture(assets.get_texture("bg/town.png"));
    bg_.set_position(8, 24);
    for (int i = 0; i < 25; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    dialog.create(1, 18, 30, 9, bty::BoxColor::Intro, assets);
    t_town_name = dialog.add_line(1, 1, "");
    t_gp = dialog.add_line(22, 2, "");
    t_get_contract = dialog.add_option(3, 3, "Get contract");
    t_rent_boat = dialog.add_option(3, 4, "");
    t_gather_information = dialog.add_option(3, 5, "Gather information");
    t_buy_spell = dialog.add_option(3, 6, "");
    t_buy_siege = dialog.add_option(3, 7, "");
}

void Town::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    gfx.draw_sprite(unit_, camera);
    dialog.draw(gfx, camera);
}

void Town::view(int town_id, const Tile &tile, int continent, int unit_id, int spell)
{
    dialog.set_color(bty::get_box_color(v.diff));

    bool have_anchor = gen.artifacts_found[ArtiAnchorOfAdmirality];
    int boat_cost = have_anchor ? 100 : 500;

    t_town_name->set_string(fmt::format("Town of {}", kTownInfo[town_id].name));
    t_gp->set_string(fmt::format("GP={}", bty::number_with_ks(v.gold)));
    t_rent_boat->set_string(v.boat_rented ? "Cancel boat rental" : fmt::format("Rent boat ({} week)", boat_cost));
    t_buy_spell->set_string(fmt::format("{} spell ({})", kSpellNames[spell], kSpellCosts[spell]));
    t_buy_siege->set_string(v.siege ? "" : "Buy siege weapons (1000)");

    int num_captured = 0;
    for (int i = 0; i < 17; i++) {
        if (gen.villains_captured[i]) {
            num_captured++;
        }
    }

    t_get_contract->set_visible(num_captured < 17);
    t_buy_siege->set_visible(!v.siege);

    this->town_id = town_id;
    unit_.set_texture(unit_textures_[unit_id]);

    hud.set_gold(v.gold);
}

void Town::update(float dt)
{
    unit_.update(dt);
    dialog.update(dt);
}

void Town::key(int key, int action)
{
    if (action == GLFW_RELEASE) {
        return;
    }

    switch (key) {
        case GLFW_KEY_ENTER:
            switch (dialog.get_selection()) {
                case 0:
                    get_contract();
                    break;
                case 1:
                    rent_boat();
                    break;
                case 2:
                    gather_information();
                    break;
                case 3:
                    buy_spell();
                    break;
                case 4:
                    buy_siege();
                    break;
                default:
                    break;
            }
            break;
        case GLFW_KEY_BACKSPACE:
            ss.pop(0);
            break;
        case GLFW_KEY_UP:
            dialog.prev();
            break;
        case GLFW_KEY_DOWN:
            dialog.next();
            break;
        default:
            break;
    }
}

void Town::buy_spell()
{
    if (v.known_spells < v.max_spells) {
        if (v.gold >= kSpellCosts[gen.town_spells[town_id]]) {
            v.gold -= kSpellCosts[gen.town_spells[town_id]];
            v.spells[gen.town_spells[town_id]]++;
            v.known_spells++;
            hud.set_gold(v.gold);

            int remaining = v.max_spells - v.known_spells;
            hud.set_title(fmt::format("     You can learn {} more spell{}.", remaining, remaining != 1 ? "s" : ""));
        }
        else {
            hud.set_error("    You do not have enough gold!");
        }
    }
    else {
        hud.set_error("   You can not learn anymore spells!", [this]() {
            hud.set_hero(v.hero, v.rank);
            hud.set_days(v.days);
        });
    }
}

void Town::buy_siege()
{
    if (v.gold < 3000) {
        hud.set_error("    You do not have enough gold!");
    }
    else {
        v.gold -= 3000;
        v.siege = true;
        hud.set_gold(v.gold);
        t_gp->set_string(fmt::format("GP={}", bty::number_with_ks(v.gold)));
        hud.set_siege(true);
    }
    dialog.prev();
    t_buy_siege->set_visible(!v.siege);
}

void Town::gather_information()
{
    std::string army;
    for (int i = 0; i < 5; i++) {
        army += fmt::format(" {} {}\n", bty::get_descriptor(gen.castle_counts[town_id][i]), kUnits[gen.castle_armies[town_id][i]].name_plural);
    }

    int villain_id = gen.castle_occupants[town_id];
    std::string occupier = villain_id == -1 ? "no one" : ("\n" + std::string(kVillains[villain_id][0]));

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {
                1,
                1,
                fmt::format("Castle {}{}is under {}'s rule.\n{}", kCastleInfo[town_id].name, villain_id == -1 ? "\n" : " ", occupier, army),
            },
        },
    });

    if (villain_id != -1) {
        gen.villains_found[villain_id] = true;
    }
}

void Town::get_contract()
{
    int num_caught = 0;
    for (int i = 0; i < 17; i++) {
        if (gen.villains_captured[i]) {
            num_caught++;
        }
    }
    if (num_caught == 17) {
        return;
    }

    int contracts[5];
    int have = 0;

    /* Get the first five uncaught villains */
    for (int i = 0; i < 17; i++) {
        if (!gen.villains_captured[i]) {
            contracts[have++] = i;
            if (have == 5) {
                break;
            }
        }
    }

    int current = 0;
    for (int i = 0; i < 5; i++) {
        if (v.contract == contracts[i]) {
            current = i;
        }
    }

    if (v.contract == 17 && contracts[0] == 0) {
        v.contract = 0;
    }
    else {
        v.contract = contracts[(current + 1) % 5];
    }

    hud.set_contract(v.contract);
    view_contract.update_info();
    ss.push(&view_contract, nullptr);
}

void Town::rent_boat()
{
    v.boat_rented = !v.boat_rented;

    bool have_anchor = gen.artifacts_found[ArtiAnchorOfAdmirality];
    int boat_cost = have_anchor ? 100 : 500;
    if (v.boat_rented) {
        v.gold -= boat_cost;
        hud.set_gold(v.gold);
    }
    t_gp->set_string(fmt::format("GP={}", bty::number_with_ks(v.gold)));
    t_rent_boat->set_string(v.boat_rented ? "Cancel boat rental" : fmt::format("Rent boat ({} week)", boat_cost));
}
