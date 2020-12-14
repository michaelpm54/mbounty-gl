#include "game/garrison.hpp"

#include <spdlog/spdlog.h>

#include "assets.hpp"
#include "game/dialog-stack.hpp"
#include "game/gen-variables.hpp"
#include "game/hud.hpp"
#include "game/scene-stack.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"

static constexpr int kKingsCastleUnits[5] = {
    Militias,
    Archers,
    Pikemen,
    Cavalries,
    Knights,
};

Garrison::Garrison(SceneStack &ss, DialogStack &ds, bty::Assets &assets, Hud &hud, Variables &v, GenVariables &gen)
    : ss(ss)
    , ds(ds)
    , hud(hud)
    , v(v)
    , gen(gen)
{
    dialog_.create(1, 18, 30, 9, bty::BoxColor::Intro, assets);
    t_castle_name = dialog_.add_line(1, 1, "");
    t_gp = dialog_.add_line(22, 1, "");    // Gold
    dialog_.add_option(21, 3, "Garrison");
    dialog_.add_option(21, 4, "Remove");
    for (int i = 0; i < 5; i++) {
        t_units[i] = dialog_.add_line(3, 3 + i, "");
    }

    unit_.set_position(64, 104);
    bg_.set_texture(assets.get_texture("bg/castle.png"));
    bg_.set_position(8, 24);

    for (int i = 0; i < 5; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", kKingsCastleUnits[i]), {2, 2});
    }
}

void Garrison::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    gfx.draw_sprite(bg_, camera);
    dialog_.draw(gfx, camera);
    gfx.draw_sprite(unit_, camera);
}

void Garrison::view(int castle_id)
{
    this->castle_id = castle_id;

    dialog_.set_color(bty::get_box_color(v.diff));
    unit_.set_texture(unit_textures_[rand() % 5]);

    t_castle_name->set_string(fmt::format("Castle {}", kCastleInfo[castle_id].name));
    t_gp->set_string(fmt::format("GP={}", bty::number_with_ks(v.gold)));
    update_units(0);
}

void Garrison::update(float dt)
{
    dialog_.update(dt);
    unit_.update(dt);
}

void Garrison::key(int key, int action)
{
    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_DOWN:
                    next();
                    break;
                case GLFW_KEY_UP:
                    prev();
                    break;
                case GLFW_KEY_ENTER:
                    select(dialog_.get_selection());
                    break;
                case GLFW_KEY_BACKSPACE:
                    ss.pop(0);
                    break;
                default:
                    break;
            }
        default:
            break;
    }
}

void Garrison::select(int opt)
{
    std::vector<DialogDef::StringDef> strings;
    std::vector<DialogDef::StringDef> options;

    strings.push_back({1, 1, fmt::format("Castle {}", kCastleInfo[castle_id].name)});
    strings.push_back({22, 1, fmt::format("GP={}", bty::number_with_ks(v.gold))});

    if (opt == 0) {
        int army_size = 0;
        for (int i = 0; i < 5; i++) {
            if (v.army[i] != -1) {
                army_size++;
            }
        }

        if (army_size == 1) {
            hud.set_error(" You may not garrison your last army!");
            return;
        }

        strings.push_back({21, 4, " Select\narmy  to\ngarrison"});

        for (int i = 0; i < 5; i++) {
            if (v.army[i] == -1) {
                strings.push_back({3, 3 + i, "empty"});
            }
            else {
                auto cost = bty::number_with_ks(kUnits[v.army[i]].weekly_cost * v.counts[i]);
                auto name = kUnits[v.army[i]].name_plural;
                std::string spaces(15 - cost.size() - name.size(), ' ');
                options.push_back({3, 3 + i, fmt::format("{}{}{}", name, spaces, cost)});
            }
        }

        ds.show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = strings,
            .options = options,
            .callbacks = {
                .confirm = [this](int opt) {
                    bool did_garrison {false};
                    for (int i = 0; i < 5; i++) {
                        if (gen.garrison_armies[castle_id][i] == -1) {
                            did_garrison = true;
                            gen.garrison_armies[castle_id][i] = v.army[opt];
                            gen.garrison_counts[castle_id][i] = v.counts[opt];
                            // TODO: Gold error?
                            v.gold -= kUnits[v.army[opt]].weekly_cost * v.counts[opt];
                            v.army[opt] = -1;
                            v.counts[opt] = 0;
                            gen.castle_occupants[castle_id] = -1;
                            break;
                        }
                    }
                    if (!did_garrison) {
                        // TODO: Slots error?
                    }
                    bty::sort_army(v.army, v.counts);
                    view(castle_id);
                },
            },
        });
    }
    else {
        int army_size = 0;
        for (int i = 0; i < 5; i++) {
            if (gen.garrison_armies[castle_id][i] != -1) {
                army_size++;
            }
        }

        if (army_size == 0) {
            hud.set_error("    There are no armies to remove!");
            return;
        }

        strings.push_back({21, 4, " Select\narmy  to\n remove"});

        for (int i = 0; i < 5; i++) {
            if (gen.garrison_armies[castle_id][i] == -1) {
                strings.push_back({3, 3 + i, "empty"});
            }
            else {
                auto count = std::to_string(gen.garrison_counts[castle_id][i]);
                auto name = kUnits[gen.garrison_armies[castle_id][i]].name_plural;
                std::string spaces(15 - count.size() - name.size(), ' ');
                options.push_back({3, 3 + i, fmt::format("{}{}{}", name, spaces, count)});
            }
        }

        ds.show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = strings,
            .options = options,
            .callbacks = {
                .confirm = [this](int opt) {
                    bool did_remove {false};
                    for (int i = 0; i < 5; i++) {
                        if (v.army[i] == -1) {
                            did_remove = true;
                            v.army[i] = gen.garrison_armies[castle_id][opt];
                            v.counts[i] = gen.garrison_counts[castle_id][opt];
                            gen.garrison_armies[castle_id][opt] = -1;
                            gen.garrison_counts[castle_id][opt] = 0;
                            break;
                        }
                        else if (v.army[i] == gen.garrison_armies[castle_id][i]) {
                            did_remove = true;
                            v.counts[i] += gen.garrison_counts[castle_id][opt];
                            gen.garrison_armies[castle_id][opt] = -1;
                            gen.garrison_counts[castle_id][opt] = 0;
                            break;
                        }
                    }
                    if (!did_remove) {
                        // TODO: Slot error?
                        hud.set_error("Failed to remove!");
                    }
                    else {
                        bty::sort_army(v.army, v.counts);
                    }
                    view(castle_id);
                },
            },
        });
    }
}

void Garrison::next()
{
    dialog_.next();
    update_units(dialog_.get_selection());
}

void Garrison::prev()
{
    dialog_.prev();
    update_units(dialog_.get_selection());
}

void Garrison::update_units(int opt)
{
    if (opt == 0) {
        for (int i = 0; i < 5; i++) {
            if (v.army[i] == -1) {
                t_units[i]->set_string("empty");
            }
            else {
                auto cost = bty::number_with_ks(kUnits[v.army[i]].weekly_cost * v.counts[i]);
                auto name = kUnits[v.army[i]].name_plural;
                std::string spaces(15 - cost.size() - name.size(), ' ');
                t_units[i]->set_string(fmt::format("{}{}{}", name, spaces, cost));
            }
        }
    }
    else {
        for (int i = 0; i < 5; i++) {
            if (gen.garrison_armies[castle_id][i] == -1) {
                t_units[i]->set_string("empty");
            }
            else {
                auto count = std::to_string(gen.garrison_counts[castle_id][i]);
                auto name = kUnits[gen.garrison_armies[castle_id][i]].name_plural;
                std::string spaces(15 - count.size() - name.size(), ' ');
                t_units[i]->set_string(fmt::format("{}{}{}", name, spaces, count));
            }
        }
    }
}
