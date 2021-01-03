#include "game/ingame.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <random>

#include "data/bounty.hpp"
#include "data/castles.hpp"
#include "data/hero.hpp"
#include "data/shop.hpp"
#include "data/signs.hpp"
#include "data/spells.hpp"
#include "data/tiles.hpp"
#include "data/towns.hpp"
#include "data/villains.hpp"
#include "engine/assets.hpp"
#include "engine/scene-stack.hpp"
#include "game/army-gen.hpp"
#include "game/chest.hpp"
#include "game/game-options.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "game/cute_c2.hpp"
#include "game/hud.hpp"
#include "game/shop-gen.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"

Ingame::Ingame(GLFWwindow *window, bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Hud &hud, GameOptions &game_options)
    : window(window)
    , game_options(game_options)
    , ss(ss)
    , ds(ds)
    , hud(hud)
    , hero(v.boat_x, v.boat_y, v.boat_c)
    , view_army(ss, assets)
    , view_char(ss, assets)
    , view_continent(ss, assets)
    , view_contract(ss, assets, v, gen, hud.get_contract())
    , view_puzzle(ss, assets)
    , day_timer(16.0f, std::bind(&Ingame::day_tick, this))
    , timestop_timer(0.25f, std::bind(&Ingame::timestop_tick, this))
    , automove_timer(0.0f, std::bind(&Ingame::automove_tick, this))
    , kings_castle(ss, ds, assets, hud, v, gen)
    , shop(ss, assets, v, hud)
    , town(ss, ds, assets, v, gen, hud, view_contract, boat)
    , s_wizard(ss, assets, v, hud)
    , s_defeat(ss, ds, assets, hud)
    , s_garrison(ss, ds, assets, hud, v, gen)
    , s_victory(ss, ds, assets, v, hud)
    , s_controls(ss, assets, game_options)
    , s_battle(ss, ds, assets, v, gen, view_army, view_char, s_controls, game_options, hud)
    , cr({0.2f, 0.4f, 0.7f, 0.9f}, {8, 8}, {0, 0})
    , save_manager(ds, std::bind(&Ingame::save_state, this, std::placeholders::_1), std::bind(&Ingame::load_state, this, std::placeholders::_1))
{
    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    map.load(assets);
    hero.load(assets);

    ui_cam = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);
    map_cam = ui_cam;

    boat.set_position(11 * 48.0f + 8.0f, 58 * 40.0f + 8.0f);
    boat.set_texture(assets.get_texture("hero/boat-stationary.png", {2, 1}));

    tile_text.create(4, 7, "X -1\nY -1\nT -1", assets.get_font());
}

void Ingame::setup(int hero, int diff)
{
    auto color = bty::get_box_color(diff);

    this->hero.set_moving(false);
    this->hero.set_flip(false);
    this->hero.set_mount(Mount::Walk);
    ds.set_default_color(color);
    hud.set_color(color);
    hud.set_hud_frame();
    s_controls.set_color(color);

    day_timer.reset();
    timestop_timer.reset();

    v.weeks_passed = 0;
    v.days_passed_this_week = 0;
    v.x = 0;
    v.y = 0;
    v.boat_x = -1;
    v.boat_y = -1;
    v.boat_c = -1;

    for (int i = 0; i < 26; i++) {
        v.visited_towns[i] = false;
        v.visited_castles[i] = false;
    }

    v.auto_move = false;
    v.auto_move_dir = {0, 0};
    automove_timer.reset();

    for (int i = 0; i < 14; i++) {
        v.spells[i] = 5;
    }

    for (int i = 0; i < 4; i++) {
        v.visited_tiles[i].clear();
        v.visited_tiles[i].resize(4096);
        std::fill(v.visited_tiles[i].begin(), v.visited_tiles[i].end(), 0xFF);
        gen.sail_maps_found[i] = false;
        gen.continent_maps_found[i] = false;
        v.tiles[i] = map.get_data(i);
        for (auto j = 0u; j < gen.mobs[i].size(); j++) {
            gen.mobs[i][j].id = i * 40 + j;
            for (int k = 0; k < 5; k++) {
                gen.mobs[i][j].army[k] = -1;
                gen.mobs[i][j].counts[k] = 0;
            }
        }
    }

    gen.sail_maps_found[0] = true;

    for (int i = 0; i < 5; i++) {
        v.army[i] = -1;
        v.counts[i] = 0;
        v.morales[i] = 0;
    }

    for (int i = 0; i < 8; i++) {
        gen.artifacts_found[i] = false;
    }

    static constexpr int const kDays[4] = {
        900,
        600,
        400,
        200,
    };

    v.boat_rented = false;
    v.hero = hero;
    v.rank = 0;
    v.days = kDays[diff];
    v.diff = diff;
    v.known_spells = 0;
    v.contract = 17;
    v.score = 0;
    v.gold = kStartingGold[hero];
    v.commission = kRankCommission[hero][0];
    v.permanent_leadership = kRankLeadership[hero][0];
    v.leadership = v.permanent_leadership;
    v.max_spells = kRankSpells[hero][0];
    v.spell_power = kRankSpellPower[hero][0];
    v.followers_killed = 0;
    v.magic = hero == 2;
    v.siege = false;
    v.continent = 0;

    switch (hero) {
        case 0:
            v.army[0] = Militias;
            v.counts[0] = 20;
            v.army[1] = Archers;
            v.counts[1] = 2;
            break;
        case 1:
            v.army[0] = Peasants;
            v.counts[0] = 20;
            v.army[1] = Militias;
            v.counts[1] = 20;
            break;
        case 2:
            v.army[0] = Peasants;
            v.counts[0] = 30;
            v.army[1] = Sprites;
            v.counts[1] = 10;
            break;
        case 3:
            v.army[0] = Wolves;
            v.counts[0] = 20;
            break;
        default:
            break;
    }

    v.timestop = 0;

    gen_tiles();

    hud.set_color(bty::get_box_color(v.diff));
    hud.set_contract(v.contract);
    hud.set_days(v.days);
    hud.set_magic(v.magic);
    hud.set_siege(v.siege);
    hud.set_hero(v.hero, v.rank);
    hud.set_gold(v.gold);

    move_hero_to(11, 58, 0);
}

void Ingame::draw(bty::Gfx &gfx, glm::mat4 &)
{
    map.draw(map_cam, v.continent);
    draw_mobs(gfx);
    if (v.boat_rented && hero.get_mount() != Mount::Boat) {
        gfx.draw_sprite(boat, map_cam);
    }
    hero.draw(gfx, map_cam);
    hud.draw(gfx, ui_cam);

    if (game_options.debug) {
        gfx.draw_rect(cr, map_cam);
        gfx.draw_text(tile_text, ui_cam);
    }
}

void Ingame::key(int key, int action)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            pause();
        }
        else if (key == GLFW_KEY_K) {
            victory();
        }
        else if (key == GLFW_KEY_R) {
            gen_tiles();
        }
        else if (key == GLFW_KEY_G) {
            hero.set_mount(hero.get_mount() == Mount::Fly ? Mount::Walk : Mount::Fly);
        }
        else if (key == GLFW_KEY_B) {
            hero.set_mount(hero.get_mount() == Mount::Walk ? Mount::Boat : Mount::Walk);
        }
        else if (key == GLFW_KEY_E) {
            v.days = 1;
            day_timer.trigger();
        }
        else if (key == GLFW_KEY_F) {
            fly_land();
        }
        else if (key == GLFW_KEY_J) {
            sail_to(0);
        }
        else if (key == GLFW_KEY_L) {
            sail_to(3);
        }
    }
}

void Ingame::update(float dt)
{
    if (save_manager.waiting()) {
        save_manager.update();
        return;
    }

    if (!ds.empty() || ss.get() != this || hud.get_error()) {
        return;
    }

    if (v.auto_move) {
        automove(dt);
        automove_timer.tick(dt);
    }

    if (!v.auto_move) {
        move_hero(get_move_input(), dt);
    }

    map.update(dt);
    hero.update(dt);

    if (v.timestop) {
        timestop_timer.tick(dt);
        /* Update their animations. */
        for (auto &mob : gen.mobs[v.continent]) {
            mob.entity.update(dt);
        }
    }
    else {
        day_timer.tick(dt);
        update_mobs(dt);
    }

    if (v.boat_rented) {
        boat.update(dt);
    }
}

void Ingame::gen_tiles()
{
    for (int i = 0; i < 26; i++) {
        gen.castle_occupants[i] = 0x7F;
        for (int j = 0; j < 5; j++) {
            gen.castle_armies[i][j] = -1;
            gen.castle_counts[i][j] = 0;
            gen.garrison_armies[i][j] = -1;
            gen.garrison_counts[i][j] = 0;
        }
        gen.town_units[i] = -1;
        gen.town_spells[i] = -1;
    }

    for (int i = 0; i < 17; i++) {
        gen.villains_found[i] = false;
        gen.villains_captured[i] = false;
    }

    for (int i = 0; i < 4; i++) {
        for (auto &mob : gen.mobs[i]) {
            mob.dead = true;
        }
        for (auto &mob : gen.friendly_mobs[i]) {
            mob = -1;
        }
    }

    map.reset();

    static constexpr int kNumShopsPerContinent[4] = {
        6,
        6,
        4,
        5,
    };

    static constexpr int kNumHighValueShopsPerContinent[4] = {
        4,
        4,
        4,
        2,
    };

    static constexpr int kArtifactTiles[] = {
        Tile_AfctScroll,
        Tile_AfctShield,
        Tile_AfctCrown,
        Tile_AfctAmulet,
        Tile_AfctRing,
        Tile_AfctAnchor,
        Tile_AfctBook,
        Tile_AfctSword,
    };

    /* Place artifacts in random order, then when adding them to
        continents just increment an index into this array. */
    std::vector<int> artifacts {
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7};

    std::default_random_engine rng_ {};

    rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::shuffle(std::begin(artifacts), std::end(artifacts), rng_);

    int artifact_index = 0;

    std::uniform_int_distribution<int> unit_gen(0, UnitId::UnitCount - 1);
    std::uniform_int_distribution<int> spell_gen(0, 13);

    /* Gen sceptre location */
    do {
        sceptre_continent = rand() % 4;
        sceptre_x = rand() % 64;
        sceptre_y = rand() % 64;
    } while (map.get_data(sceptre_continent)[sceptre_y * 64 + sceptre_x] != Tile_Grass);

    for (int continent = 0; continent < 4; continent++) {
        auto *tiles = map.get_data(continent);

        std::vector<glm::ivec2> random_tiles;

        int num_mobs = 0;

        int n = 0;
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 64; x++) {
                int id = tiles[n];

                if (id == Tile_GenRandom) {
                    random_tiles.push_back({x, y});
                }
                else if (id == Tile_GenTown) {
                    int town = -1;
                    for (int i = 0; i < 26; i++) {
                        if (kTownInfo[i].continent == continent && kTownInfo[i].x == x && 63 - kTownInfo[i].y == y) {
                            town = i;
                        }
                    }
                    if (town != -1) {
                        gen.town_units[town] = unit_gen(rng_);

                        /* Hunterville always has Bridge */
                        if (town == 21) {
                            gen.town_spells[town] = 7;
                        }
                        else {
                            gen.town_spells[town] = spell_gen(rng_);
                        }

                        tiles[x + y * 64] = Tile_Town;
                    }
                    else {
                        spdlog::warn("({}) Unknown town at {}, {}", continent, x, 63 - y);
                    }
                }
                else if (id == Tile_GenCastleGate) {
                    tiles[x + y * 64] &= 0x7F;
                }
                else if (id == Tile_GenMonster) {
                    auto &mob = gen.mobs[continent][num_mobs++];
                    mob.tile = {x, y};
                    mob.dead = false;

                    gen_mob_army(continent, mob.army, mob.counts);
                    bty::sort_army(mob.army, mob.counts);

                    int highest = 0;
                    for (int i = 0; i < 5; i++) {
                        if (mob.army[i] == -1) {
                            continue;
                        }
                        if (kUnits[mob.army[i]].hp >= kUnits[mob.army[highest]].hp) {
                            highest = i;
                        }
                    }

                    mob.sprite_id = mob.army[highest];

                    mob.entity.set_texture(unit_textures[mob.sprite_id]);
                    mob.entity.move_to_tile({x, y, Tile_Grass});

                    tiles[x + y * 64] = Tile_Grass;
                }
                n++;
            }
        }

        rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::shuffle(std::begin(random_tiles), std::end(random_tiles), rng_);

        unsigned int used_tiles = 0;
        glm::ivec2 tile;

        /* Process:
			for each continent:
				Gather list of RNG tiles
				[-2] Gen 2 teleport caves
				[-1] Gen 1 sail map
				[-1] Gen 1 local map
				[-2] Gen 2 artifacts
				Init 11 shops
				(continent 0) -> Gen 1 peasant shop
				[-K] Gen K shops
				[-L] Gen L high value shops
				Init 35 mobs
				Gen M mobs
				[-5] Gen 5 friendlies
				Set all RNG tiles to Grass
			Total RNG tiles used:
				11 + K + L
		*/

        /* 2 cave teleport tiles per continent */
        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = Tile_ShopCave;
        gen.teleport_cave_tiles[continent][0] = tile;

        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = Tile_ShopCave;
        gen.teleport_cave_tiles[continent][1] = tile;

        /* 1 sail map */
        if (continent != 3) {
            /* Saharia doesn't have a map to anywhere */
            tile = random_tiles[used_tiles++];
            tiles[tile.x + tile.y * 64] = Tile_Chest;
            gen.sail_map_tiles[continent] = tile;
        }

        /* 1 local map */
        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = Tile_Chest;
        gen.continent_map_tiles[continent] = tile;

        /* 2 artifacts */
        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifact_index++]];

        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifact_index++]];

        /* Init 11 shops. */
        gen.shops[continent].resize(11);
        for (int i = 0; i < 11; i++) {
            auto &shop = gen.shops[continent][i];
            shop.count = 0;
            shop.unit = Peasants;
            shop.x = 0;
            shop.y = 0;
        }

        int num_shops = 0;

        /* Gen guaranteed peasant dwelling. */
        if (continent == 0) {
            tiles[27 + 52 * 64] = Tile_ShopWagon;

            auto &shop = gen.shops[0][num_shops++];
            shop.x = 27;
            shop.y = 63 - 11;
            shop.unit = Peasants;

            int a = rand() % 25;
            int b = a + 200;
            int c = rand() % 25;
            int d = c + b;
            int e = rand() % 25;
            int f = e + d;
            int g = rand() % 25;
            int h = g + f;

            shop.count = h;
        }

        /* Gen K shops (skipping the first peasant shop) */
        for (int i = 0; i < kNumShopsPerContinent[continent]; i++) {
            tile = random_tiles[used_tiles++];
            auto &shop = gen.shops[continent][num_shops++];
            shop.x = tile.x;
            shop.y = tile.y;
            shop.unit = gen_shop_unit(i, continent);
            shop.count = gen_shop_count(shop.unit);
            shop.tile_id = get_shop_tile(shop.unit);
            tiles[tile.x + tile.y * 64] = shop.tile_id;
        }

        /* Gen L high value shops */
        for (int i = 0; i < kNumHighValueShopsPerContinent[continent]; i++) {
            tile = random_tiles[used_tiles++];
            auto &shop = gen.shops[continent][num_shops++];
            shop.x = tile.x;
            shop.y = tile.y;
            if (continent == 3) {
                int roll = rand() % 3;
                shop.unit = roll + 0x15;
                int max = kMaxShopCounts[shop.unit];
                int a = (rand() % kMaxShopCounts[shop.unit]) / 8;
                int b = (rand() % kMaxShopCounts[shop.unit]) / 16;
                int c = rand() % 4;
                shop.count = c + a + b + max;
                tiles[tile.x + tile.y * 64] = kShopTileForUnit[shop.unit];
            }
            else {
                int id = 0;
                do {
                    int roll = rand() % 14;
                    id = roll + continent;
                } while (kMaxShopCounts[id] == 0);
                int max = kMaxShopCounts[id];
                int a = (rand() % kMaxShopCounts[shop.unit]) / 8;
                int b = (rand() % kMaxShopCounts[shop.unit]) / 16;
                int c = rand() % 4;
                shop.unit = id;
                shop.count = c + a + b + max;
                tiles[tile.x + tile.y * 64] = kShopTileForUnit[shop.unit];
            }
        }

        /* Gen 5 friendlies */
        for (int i = 0; i < 5; i++) {
            tile = random_tiles[used_tiles++];

            int id = gen_mob_unit(continent);
            int count = gen_mob_count(continent, id);

            auto &mob = gen.mobs[continent][num_mobs++];
            mob.tile = {tile.x, tile.y};
            mob.dead = false;
            mob.army[0] = id;
            mob.counts[0] = count;
            mob.entity.set_texture(unit_textures[id]);
            mob.entity.move_to_tile({tile.x, tile.y, Tile_Grass});
            mob.sprite_id = id;

            tiles[tile.x + tile.y * 64] = Tile_Grass;

            gen.friendly_mobs[continent].push_back(mob.id);
        }

        /* Gen castle occupants */
        for (int i = 0; i < kVillainsPerContinent[continent]; i++) {
            int castle;
            do {
                castle = bty::random(26);
            } while (gen.castle_occupants[castle] != 0x7F || kCastleInfo[castle].continent != continent);

            int villain = i + kVillainIndices[continent];

            gen.castle_occupants[castle] = villain;
            gen_villain_army(villain, gen.castle_armies[castle], gen.castle_counts[castle]);
        }

        /* Turn the rest of the RNG tiles into chests */
        for (auto i = used_tiles; i < random_tiles.size(); i++) {
            tiles[random_tiles[i].x + random_tiles[i].y * 64] = Tile_Chest;
        }
    }

    for (unsigned int i = 0; i < 26; i++) {
        if (gen.castle_occupants[i] != 0x7F) {
            continue;
        }
        gen_castle_army(kCastleInfo[i].continent, gen.castle_armies[i], gen.castle_counts[i]);
    }

    hud.set_puzzle(gen.villains_captured.data(), gen.artifacts_found.data());

    map.create_geometry();
}

void Ingame::update_camera()
{
    glm::vec2 cam_centre = hero.get_center();
    glm::vec3 cam_pos = {cam_centre.x - 120, cam_centre.y - 120, 0.0f};
    map_cam = ui_cam * glm::translate(-cam_pos);
}

void Ingame::update_visited_tiles()
{
    const glm::ivec2 tile {v.x, v.y};
    auto *visited = v.visited_tiles[v.continent].data();
    auto *tiles = map.get_data(v.continent);

    int range = 4;
    int offset = range / 2;

    for (int i = 0; i <= range; i++) {
        for (int j = 0; j <= range; j++) {
            int x = tile.x - offset + i;
            int y = tile.y - offset + j;

            if (x < 0 || x > 63 || y < 0 || y > 63) {
                continue;
            }

            int index = x + y * 64;
            visited[index] = tiles[index];
        }
    }
}

void Ingame::move_hero_to(int x, int y, int c)
{
    this->hero.move_to_tile(map.get_tile(x, y, c));
    update_camera();

    v.x = x;
    v.y = y;
    v.continent = c;

    update_visited_tiles();
}

void Ingame::use_magic()
{
    std::vector<bty::DialogDef::StringDef> options;

    for (int i = 7; i < 14; i++) {
        options.push_back({4, 3 + i - 7, fmt::format("{} {}", v.spells[i], kSpellNames[i])});
    }

    for (int i = 0; i < 7; i++) {
        options.push_back({4, 14 + i, fmt::format("{} {}", v.spells[i], kSpellNames[i])});
    }

    bool no_spells = true;
    for (int i = 7; i < 14; i++) {
        if (v.spells[i] != 0) {
            no_spells = false;
            break;
        }
    }

    auto confirm = no_spells ? std::function<void(int)>(nullptr) : std::bind(&Ingame::use_spell, this, std::placeholders::_1);

    auto *dialog = ds.show_dialog({
        .x = 6,
        .y = 4,
        .w = 20,
        .h = 22,
        .strings = {
            {1, 1, "Adventuring Spells"},
            {3, 12, "Combat Spells"},
        },
        .options = options,
        .callbacks = {
            .confirm = confirm,
        },
    });

    auto &opt = dialog->get_options();
    for (int i = 7; i < 14; i++) {
        opt[i].disable();
    }

    if (no_spells) {
        ds.show_dialog({
            .x = 0,
            .y = 0,
            .w = 40,
            .h = 3,
            .strings = {{1, 1, "You have no Adventuring spell to cast!"}},
        });
        for (int i = 0; i < 7; i++) {
            opt[i].disable();
        }
    }
}

void Ingame::use_spell(int spell)
{
    if (false && !v.magic) {
        ds.show_dialog({
            .x = 6,
            .y = 10,
            .w = 20,
            .h = 10,
            .strings = {
                {2, 1, "You haven't been"},
                {1, 2, "trained in the art"},
                {3, 3, "of spellcasting"},
                {3, 4, "yet. Visit the"},
                {2, 5, "Archmage Aurange"},
                {2, 6, "in Continentia at"},
                {3, 7, "11,19 for this"},
                {6, 8, "ability."},
            },
        });
        return;
    }

    v.spells[spell + 7]--;

    switch (spell) {
        case 0:
            spell_bridge();
            break;
        case 1:
            spell_timestop();
            break;
        case 2:
            spell_find_villain();
            break;
        case 3:
            spell_tc_gate(false);
            break;
        case 4:
            spell_tc_gate(true);
            break;
        case 5:
            spell_instant_army();
            break;
        case 6:
            spell_raise_control();
            break;
        default:
            break;
    }
}

void Ingame::place_bridge_at(int x, int y, int continent, bool horizontal)
{
    auto destination = map.get_tile(x, y, continent);

    if (destination.id == -1 || destination.id < Tile_WaterIRT || destination.id > Tile_Water) {
        hud.set_error("Not a suitable location for a bridge!", [this]() {
            hud.set_error("    What a waste of a good spell!");
        });
    }
    else {
        map.set_tile({x, y, -1}, continent, horizontal ? Tile_BridgeHorizontal : Tile_BridgeVertical);
    }
}

void Ingame::spell_bridge()
{
    int c = v.continent;
    int x = v.x;
    int y = v.y;

    ds.show_dialog({
        .x = 1,
        .y = 20,
        .w = 30,
        .h = 7,
        .strings = {
            {2, 1, "Bridge in which direction?"},
            {14, 3, " \x81\n\x84 \x82\n \x83"},
        },
        .callbacks = {
            .up = {
                [this, x, y, c](bty::Dialog &) {
                    place_bridge_at(x, y - 1, c, false);
                    ds.pop();
                },
            },
            .down = {
                [this, x, y, c](bty::Dialog &) {
                    place_bridge_at(x, y + 1, c, false);
                    ds.pop();
                },
            },
            .left = {
                [this, x, y, c](bty::Dialog &) {
                    place_bridge_at(x - 1, y, c, true);
                    ds.pop();
                },
            },
            .right = {
                [this, x, y, c](bty::Dialog &) {
                    place_bridge_at(x + 1, y, c, true);
                    ds.pop();
                },
            },
        },
    });
}

void Ingame::spell_timestop()
{
    timestop_timer.reset();
    v.timestop += v.spell_power * 10;
    if (v.timestop > 9999) {
        v.timestop = 9999;
    }
    hud.set_timestop(v.timestop);
}

void Ingame::spell_find_villain()
{
    if (v.contract < 17) {
        gen.villains_found[v.contract] = true;
    }
    view_contract.update_info();
    ss.push(&view_contract, nullptr);
}

static void tc_gate_left(bty::Dialog &dialog);
static void tc_gate_right(bty::Dialog &dialog);

void Ingame::town_gate_confirm(int opt)
{
    move_hero_to(kTownGateX[kTownsAlphabetical[opt]], 63 - kTownGateY[kTownsAlphabetical[opt]], kTownInfo[opt].continent);
}

void Ingame::castle_gate_confirm(int opt)
{
    move_hero_to(kCastleInfo[opt].x, (63 - kCastleInfo[opt].y) + 1, kCastleInfo[opt].continent);
}

void Ingame::spell_tc_gate(bool town)
{
    std::vector<bty::DialogDef::StringDef> options(26);
    std::vector<bool> visible_options(26);

    int n = 0;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 13; y++) {
            options[n] = {
                .x = 3 + x * 15,
                .y = 6 + y,
                .str = town ? kTownInfo[kTownsAlphabetical[n]].name : kCastleInfo[n].name,
            };
            visible_options[n] = town ? v.visited_towns[n] : v.visited_castles[n];
            n++;
        }
    }

    ds.show_dialog({
        .x = 1,
        .y = 3,
        .w = 30,
        .h = 24,
        .strings = {
            {4, 3, fmt::format("{} you have been to", town ? "Towns" : "Castles")},
            {6, 20, fmt::format("Revisit which {}?", town ? "town" : "castle")},
        },
        .options = options,
        .visible_options = visible_options,
        .callbacks = {
            .confirm = std::bind(town ? &Ingame::town_gate_confirm : &Ingame::castle_gate_confirm, this, std::placeholders::_1),
            .left = &tc_gate_left,
            .right = &tc_gate_right,
        },
    });
}

void tc_gate_left(bty::Dialog &dialog)
{
    int selection = dialog.get_selection();
    selection = (selection - 13 + 26) % 26;
    int nearest_index = 0;
    int nearest_distance = 26;
    auto &opt = dialog.get_options();
    for (int i = 0; i < 13; i++) {
        int test = selection > 12 ? (13 + i) : (i);
        int distance = std::abs(selection - test);
        if (distance < nearest_distance) {
            if (opt[i].visible()) {
                nearest_index = test;
                nearest_distance = distance;
            }
        }
    }
    dialog.set_selection(nearest_index);
}

void tc_gate_right(bty::Dialog &dialog)
{
    int selection = dialog.get_selection();
    selection = (selection + 13) % 26;
    int nearest_index = 0;
    int nearest_distance = 26;
    auto &opt = dialog.get_options();
    for (int i = 0; i < 13; i++) {
        int test = selection > 12 ? (13 + i) : (i);
        int distance = std::abs(selection - test);
        if (distance < nearest_distance) {
            if (opt[i].visible()) {
                nearest_index = test;
                nearest_distance = distance;
            }
        }
    }
    dialog.set_selection(nearest_index);
}

bool Ingame::add_unit_to_army(int id, int count)
{
    if (id < 0 || id >= UnitId::UnitCount) {
        spdlog::warn("add_unit_to_army: id out of range: {}", id);
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (v.army[i] == id) {
            v.counts[i] += count;
            return true;
        }
    }

    int army_size = 0;

    for (int i = 0; i < 5; i++) {
        if (v.army[i] != -1) {
            army_size++;
        }
    }

    if (army_size == 5) {
        spdlog::warn("add_unit_to_army: army already full");
        return false;
    }

    int index = army_size++;

    v.army[index] = id;
    v.counts[index] = count;

    return true;
}

void Ingame::spell_instant_army()
{
    static constexpr int kInstantArmyUnits[4][4] = {
        {
            Peasants,
            Militias,
            Archers,
            Knights,
        },
        {
            Peasants,
            Militias,
            Archers,
            Cavalries,
        },
        {
            Sprites,
            Gnomes,
            Elves,
            Druids,
        },
        {
            Peasants,
            Wolves,
            Orcs,
            Ogres,
        },
    };

    int unit = kInstantArmyUnits[v.rank][v.hero];
    int amt = v.spell_power * 3 + (rand() % v.spell_power + 2);

    if (!add_unit_to_army(unit, amt)) {
        hud.set_error(" You do not have any more army slots!");
    }
    else {
        ds.show_dialog({
            .x = 1,
            .y = 21,
            .w = 30,
            .h = 6,
            .strings = {
                {1, 1, fmt::format("{} {}", bty::get_descriptor(amt), kUnits[unit].name_plural)},
                {3, 3, "have joined your army."},
            },
        });
    }
}

void Ingame::spell_raise_control()
{
    v.leadership += v.spell_power * 100;
    v.leadership &= 0xFFFF;
}

void Ingame::end_week_astrology(bool search)
{
    int unit = rand() % UnitId::UnitCount;
    const auto &name = kUnits[unit].name_singular;

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, fmt::format("Week #{}", v.weeks_passed)},
            {1, 3, fmt::format("Astrologers proclaim:\nWeek of the {}\n\nAll {} dwellings are\nrepopulated.", name, name)},
        },
        .callbacks = {
            .confirm = [this, search](int) {
                end_week_budget(search);
            },
        },
    });

    v.leadership = v.permanent_leadership;

    for (int i = 0; i < 26; i++) {
        if (gen.castle_occupants[i] == -1) {
            if (gen.garrison_armies[i][0] == -1) {
                gen_castle_army(kCastleInfo[i].continent, gen.castle_armies[i], gen.castle_counts[i]);
                gen.castle_occupants[i] = 0x7F;
            }
        }
    }
}

void Ingame::end_week_budget(bool search)
{
    int army_total = 0;
    int commission = v.commission;

    for (int i = 0; i < 26; i++) {
        if (gen.castle_occupants[i] == -1) {
            commission += 250;
        }
    }

    for (int i = 0; i < 26; i++) {
        if (gen.castle_occupants[i] == -1) {
            v.gold += 250;
        }
    }

    int gold = v.gold;
    int boat = v.boat_rented ? (gen.artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500) : 0;
    int balance = (commission + gold) - boat;

    std::vector<bty::DialogDef::StringDef> strings;

    for (int i = 0; i < 5; i++) {
        if (v.army[i] == -1) {
            continue;
        }

        const auto &unit = kUnits[v.army[i]];

        int weekly_cost = unit.weekly_cost * v.counts[i];
        if (balance > weekly_cost || balance - weekly_cost == 0) {
            balance -= weekly_cost;
            army_total += weekly_cost;

            std::string cost = bty::number_with_ks(weekly_cost);
            std::string spaces(14 - (cost.size() + unit.name_plural.size()), ' ');
            strings.push_back({15, 3 + i, fmt::format("{}{}{}", unit.name_plural, spaces, cost)});
        }
        else {
            v.army[i] = -1;
            v.counts[i] = -1;

            std::string spaces(14 - (5 + unit.name_plural.size()), ' ');
            strings.push_back({15, 3 + i, fmt::format("{}{}Leave", unit.name_plural, spaces)});
        }
    }

    bty::sort_army(v.army, v.counts);

    bool out_of_money = (boat + army_total) > (gold + commission);
    if (!out_of_money) {
        balance = (commission + gold) - (boat + army_total);
    }

    strings.push_back({1, 1, fmt::format("Week #{}", v.weeks_passed)});
    strings.push_back({23, 1, "Budget"});
    strings.push_back({1, 3, fmt::format("On Hand: {}", bty::number_with_ks(v.gold))});
    strings.push_back({1, 4, fmt::format("Payment: {:>4}", commission)});
    strings.push_back({1, 5, fmt::format("Boat: {:>7}", boat)});
    strings.push_back({1, 6, fmt::format("Army: {:>7}", army_total)});
    strings.push_back({1, 7, fmt::format("Balance: {:>4}", bty::number_with_ks(balance))});

    v.gold = balance;

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = strings,
        .callbacks = {
            .confirm = [this, search](int) {
                if (search) {
                    end_week(false);
                }
            },
        },
    });

    if (v.army[0] == -1 || out_of_money) {
        disgrace();
    }
}

void Ingame::disgrace()
{
    move_hero_to(11, 58, 0);
    hero.set_mount(Mount::Walk);
    hero.set_flip(false);
    hero.set_moving(false);

    v.army[0] = Peasants;
    v.army[1] = -1;
    v.army[2] = -1;
    v.army[3] = -1;
    v.army[4] = -1;
    v.counts[0] = 20;
    v.counts[1] = 0;
    v.counts[2] = 0;
    v.counts[3] = 0;
    v.counts[4] = 0;

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, "After being disgraced on the"},
            {5, 2, "field of battle, King"},
            {2, 3, "Maximus summons you to his"},
            {3, 4, "castle. After a lesson in"},
            {4, 5, "tactics, he reluctantly"},
            {1, 6, "reissues your commission and"},
            {4, 7, "sends you on your way."},
        },
    });
}

void Ingame::defeat()
{
}

void Ingame::victory()
{
    hud.set_blank_frame();
    ss.push(&s_victory, [this](int) {
        ss.pop(0);
    });
}

void Ingame::dismiss()
{
    std::vector<bty::DialogDef::StringDef> options;

    for (int i = 0; i < 5; i++) {
        if (v.army[i] != -1) {
            options.push_back({11, 3 + i, kUnits[v.army[i]].name_plural});
        }
    }

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {{5, 1, "Dismiss which army?"}},
        .options = options,
        .callbacks = {
            .confirm = [this](int opt) {
                dismiss_slot(opt);
                dismiss();
            },
        },
    });
}

void Ingame::dismiss_slot(int slot)
{
    int num_units = 0;
    for (int i = 0; i < 5; i++) {
        if (v.army[i] != -1) {
            num_units++;
        }
    }

    if (num_units == 1) {
        hud.set_error("  You may not dismiss your last army!");
        return;
    }

    v.army[slot] = -1;
    v.counts[slot] = 0;
    bty::sort_army(v.army, v.counts);
    dismiss();
}

void Ingame::pause()
{
    ds.show_dialog({
        .x = 3,
        .y = 7,
        .w = 26,
        .h = 16,
        .options = {
            {3, 2, "View your army"},
            {3, 3, "View your character"},
            {3, 4, "Look at continent map"},
            {3, 5, "Use magic"},
            {3, 6, "Contract information"},
            {3, 7, "Wait to end of week"},
            {3, 8, "Look at puzzle pieces"},
            {3, 9, "Search the area"},
            {3, 10, "Dismiss army"},
            {3, 11, "Game controls"},
            {3, 12, "Load game"},
            {3, 13, "Save game"},
        },
        .callbacks = {
            .confirm = [this](int opt) {
                switch (opt) {
                    case 0:
                        for (int i = 0; i < 5; i++) {
                            if (v.counts[i] * kUnits[v.army[i]].hp > v.leadership) {
                                v.morales[i] = 3;
                                continue;
                            }
                            v.morales[i] = bty::check_morale(i, v.army.data());
                        }
                        view_army.update_info(v.army.data(), v.counts.data(), v.morales.data(), v.diff);
                        ss.push(&view_army, nullptr);
                        break;
                    case 1:
                        view_char.update_info(v, gen);
                        ss.push(&view_char, nullptr);
                        break;
                    case 2:
                        view_continent.update_info(v, gen.continent_maps_found[v.continent]);
                        ss.push(&view_continent, nullptr);
                        break;
                    case 3:
                        use_magic();
                        break;
                    case 4:
                        view_contract.update_info();
                        ss.push(&view_contract, nullptr);
                        break;
                    case 5:
                        end_week(false);
                        break;
                    case 6:
                        map_cam = ui_cam * glm::translate(-glm::vec3 {sceptre_x * 48.0f + 24.0f - 120, sceptre_y * 40.0f + 20.0f - 120, 0.0f});
                        temp_continent = v.continent;
                        v.continent = sceptre_continent;
                        view_puzzle.update_info(gen);
                        ss.push(&view_puzzle, [this](int) {
                            update_camera();
                            v.continent = temp_continent;
                        });
                        break;
                    case 7:
                        search_area();
                        break;
                    case 8:
                        dismiss();
                        break;
                    case 9:
                        s_controls.set_battle(false);
                        ss.push(&s_controls, nullptr);
                        break;
                    case 10:
                        save_manager.show_saves_dialog(true);
                        break;
                    case 11:
                        save_manager.show_saves_dialog(false);
                        break;
                    default:
                        break;
                }
            },
        },
        .pop_on_confirm = true,
    });
}

int Ingame::get_move_input()
{
    int move_flags = DIR_FLAG_NONE;

    if (glfwGetKey(window, GLFW_KEY_LEFT)) {
        move_flags |= DIR_FLAG_LEFT;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT)) {
        move_flags |= DIR_FLAG_RIGHT;
    }
    if (glfwGetKey(window, GLFW_KEY_UP)) {
        move_flags |= DIR_FLAG_UP;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN)) {
        move_flags |= DIR_FLAG_DOWN;
    }

    return move_flags;
}

bool Ingame::hero_can_move(int id)
{
    return hero.can_move(id, 0, 0, 0);
}

bool Ingame::mob_can_move(int id)
{
    return id == Tile_Grass;
}

bool Ingame::move_increment(c2AABB &box, float dx, float dy, Tile &center_tile, Tile &collided_tile, bool (Ingame::*can_move)(int), bool mob)
{
    box.min.x += dx;
    box.max.x += dx;
    box.min.y += dy;
    box.max.y += dy;

    /* Test. */
    center_tile = map.get_tile(box.min.x + 4, box.min.y + 4, v.continent);

    if (!mob) {
        /* Tile center is the same as the last event tile we collided with.
		If we're not overlapping any other tiles, don't collide. */
        if (center_tile.tx == last_event_tile.tx && center_tile.ty == last_event_tile.ty) {
            return false;
        }
        /* Don't endlessly loop between the two teleport caves just because
		"technically they are different tiles." (who am I quoting?) */
        else if (last_event_tile.id == Tile_ShopCave) {
            for (int i = 0; i < 2; i++) {
                if (last_event_tile.tx == gen.teleport_cave_tiles[v.continent][i].x && last_event_tile.ty == gen.teleport_cave_tiles[v.continent][i].y) {
                    return false;
                }
            }
        }
    }

    /* Collided; undo move and register it. */
    if (!(this->*can_move)(center_tile.id)) {
        box.min.x -= dx;
        box.max.x -= dx;
        box.min.y -= dy;
        box.max.y -= dy;
        collided_tile = center_tile;
        return true;
    }
    /* Didn't collide; confirm move. */
    else {
        return false;
    }
}

void Ingame::move_mob(Mob &mob, float dt, const glm::vec2 &dir)
{
    float speed = 70.0f;
    float vel = speed * dt;
    float dx = dir.x * vel;
    float dy = dir.y * vel;
    auto last_pos = mob.entity.get_position();

    /* Create shape. */
    auto aabb = mob.entity.get_aabb();

    last_tile = map.get_tile(aabb.min.x + 4, aabb.min.y + 4, v.continent);

    Tile center_tile {-1, -1, -1};
    Tile collided_tile {-1, -1, -1};

    bool collide_x = move_increment(aabb, dx, 0, center_tile, collided_tile, &Ingame::mob_can_move, true);
    bool collide_y = move_increment(aabb, 0, dy, center_tile, collided_tile, &Ingame::mob_can_move, true);

    if (center_tile.tx != mob.tile.x || center_tile.ty != mob.tile.y) {
        mob.tile = {center_tile.tx, center_tile.ty};
    }

    mob.entity.set_position(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);
}

void Ingame::move_hero(int move_flags, float dt)
{
    if (game_options.debug) {
        cr.set_color({0.0f, 0.0f, 0.7f, 0.9f});
    }

    if (move_flags == DIR_FLAG_NONE) {
        hero.set_moving(false);
        return;
    }

    hero.set_moving(true);

    if ((move_flags & DIR_FLAG_LEFT) && !(move_flags & DIR_FLAG_RIGHT)) {
        hero.set_flip(true);
    }
    else if ((move_flags & DIR_FLAG_RIGHT) && !(move_flags & DIR_FLAG_LEFT)) {
        hero.set_flip(false);
    }

    glm::vec2 dir {0.0f};

    if (move_flags & DIR_FLAG_UP) {
        dir.y -= 1.0f;
    }
    if (move_flags & DIR_FLAG_DOWN) {
        dir.y += 1.0f;
    }
    if (move_flags & DIR_FLAG_LEFT) {
        dir.x -= 1.0f;
    }
    if (move_flags & DIR_FLAG_RIGHT) {
        dir.x += 1.0f;
    }

    auto mount = hero.get_mount();
    float speed = mount == Mount::Fly ? 300.0f : 120.0f;
    float vel = speed * dt * hero.get_speed_multiplier();
    float dx = dir.x * vel;
    float dy = dir.y * vel;

    /* Create shape. */
    auto aabb = hero.get_aabb();
    auto last_pos = hero.get_position();
    last_tile = map.get_tile(aabb.min.x + 4, aabb.min.y + 4, v.continent);

    Tile center_tile {-1, -1, -1};
    Tile collided_tile {-1, -1, -1};

    bool collide_x = move_increment(aabb, dx, 0, center_tile, collided_tile, &Ingame::hero_can_move, false);
    bool collide_y = move_increment(aabb, 0, dy, center_tile, collided_tile, &Ingame::hero_can_move, false);

    bool teleport {false};

    /* Deal with the consequences of the collision. */
    if (collide_x || collide_y) {
        /* Try to collide with it, and see if it's an event tile. */
        if (hero.get_mount() == Mount::Walk && events(collided_tile, teleport)) {
            last_event_tile = collided_tile;

            if (!teleport && collided_tile.id != Tile_CastleB) {
                /* Move into it. */
                aabb.min.x += dx;
                aabb.max.x += dx;
                aabb.min.y += dy;
                aabb.max.y += dy;
            }
            else if (collided_tile.id == Tile_CastleB) {
                /* Move /away/ from it. */
                aabb.min.y -= dy * 3;
                aabb.max.y -= dy * 3;
            }
            center_tile = map.get_tile(aabb.min.x + 4, aabb.min.y + 4, v.continent);
        }
        /* Walked into the boat tile. */
        else if (collided_tile.tx == v.boat_x && collided_tile.ty == v.boat_y && v.continent == v.boat_c) {
            hero.set_mount(Mount::Boat);
            /* Move into it. */
            aabb.min.x += dx;
            aabb.max.x += dx;
            aabb.min.y += dy;
            aabb.max.y += dy;
            center_tile = map.get_tile(aabb.min.x + 4, aabb.min.y + 4, v.continent);
        }
        /* Dismount. */
        else if (collided_tile.id == Tile_Grass || bty::is_event_tile(collided_tile.id) && hero.get_mount() == Mount::Boat) {
            hero.set_mount(Mount::Walk);
            /* Move into it. */
            aabb.min.x += dx;
            aabb.max.x += dx;
            aabb.min.y += dy;
            aabb.max.y += dy;
            center_tile = map.get_tile(aabb.min.x + 4, aabb.min.y + 4, v.continent);

            v.boat_x = last_tile.tx;
            v.boat_y = last_tile.ty;
            v.boat_c = v.continent;

            boat.set_flip(hero.get_flip());

            auto hp = glm::vec2(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);
            auto bp = last_pos;

            glm::vec4 a {hp.x, hp.y, 0.0f, 1.0f};
            glm::vec4 b {bp.x, bp.y, 0.0f, 1.0f};

            auto push_away_from_land_dir = glm::normalize(a - b);
            push_away_from_land_dir.x *= -8.0f;
            push_away_from_land_dir.y *= -8.0f;
            boat.set_position(last_pos + glm::vec2 {push_away_from_land_dir.x, push_away_from_land_dir.y});
        }
    }
    /* Not colliding; if the tile is different to the previous one, update it and
		forget about the last event tile meaning we can once again collide with it. */
    else if (center_tile.tx != last_tile.tx || center_tile.ty != last_tile.ty) {
        if (hero.get_mount() == Mount::Walk && center_tile.id >= Tile_SandELT && center_tile.id <= Tile_Sand) {
            hero.set_speed_multiplier(0.6f);
        }
        else {
            hero.set_speed_multiplier(1.0f);
        }
        last_tile = center_tile;
        last_event_tile = {-1, -1, -1};
    }

    v.x = center_tile.tx;
    v.y = center_tile.ty;

    update_visited_tiles();

    if (!teleport) {
        hero.set_position(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);
    }

    if (game_options.debug) {
        cr.set_position(aabb.min.x, aabb.min.y);
        if (collide_x && collide_y) {
            cr.set_color({0.75f, 0.95f, 0.73f, 0.9f});
        }
        else if (collide_x) {
            cr.set_color({0.7f, 0.0f, 0.0f, 0.9f});
        }
        else if (collide_y) {
            cr.set_color({0.75f, 0.35f, 0.73f, 0.9f});
        }
    }

    if (game_options.debug) {
        tile_text.set_string(fmt::format("X {}\nY {}\nT {}{}{}\nLast tile {} {} {}", v.x, v.y, map.get_tile(v.x, v.y, v.continent).id, collide_x || collide_y ? "\nC " : "", collide_x || collide_y ? std::to_string(collided_tile.id) : "", last_tile.tx, last_tile.ty, last_tile.id));
    }

    update_camera();
}

void Ingame::sail_to(int continent)
{
    v.auto_move = true;

    if (continent == v.continent) {
        auto pos = hero.get_center();

        v.auto_move_dir = {0, 0};
        hero.set_moving(true);

        if (pos.x < 0) {
            v.auto_move_dir.x = 1;
            hero.set_flip(false);
        }
        else if (pos.x > 48 * 64) {
            v.auto_move_dir.x = -1;
            hero.set_flip(true);
        }

        if (pos.y < 0) {
            v.auto_move_dir.y = 1;
        }
        else if (pos.y > 40 * 64) {
            v.auto_move_dir.y = -1;
        }
    }
    else {
        hero.set_moving(true);
        hero.set_moving(true);
        v.auto_move_dir = {0, 0};

        switch (continent) {
            case 0:
                // Up
                automove_timer.set_timer(1.0f);
                v.auto_move_dir.y = -1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(11, 64 - 1, v.continent));
                break;
            case 1:
                // Right
                automove_timer.set_timer(0.4f);
                v.auto_move_dir.x = 1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(0, 64 - 25, v.continent));
                break;
            case 2:
                // Down
                automove_timer.set_timer(0.6f);
                v.auto_move_dir.y = 1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(14, 0, v.continent));
                break;
            case 3:
                // Up
                automove_timer.set_timer(1);
                v.auto_move_dir.y = -1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(9, 64 - 1, v.continent));
                break;
            default:
                break;
        }

        hero.set_mount(Mount::Boat);
        update_camera();
        v.continent = continent;
    }
}

void Ingame::draw_mobs(bty::Gfx &gfx)
{
    for (auto *mob : get_mobs_in_range(v.x, v.y, 4)) {
        if (mob->dead) {
            continue;
        }
        mob->entity.draw(gfx, map_cam);
    }
}

void Ingame::collide_teleport_cave(const Tile &tile)
{
    auto dest = glm::ivec2 {tile.tx, tile.ty} == gen.teleport_cave_tiles[v.continent][0] ? gen.teleport_cave_tiles[v.continent][1] : gen.teleport_cave_tiles[v.continent][0];
    move_hero_to(dest.x, dest.y, v.continent);
}

void Ingame::update_mobs(float dt)
{
    const auto &hero_pos = hero.get_position();

    int continent = v.continent;

    for (auto *mob : get_mobs_in_range(v.x, v.y, 4)) {
        if (mob->dead) {
            continue;
        }

        const auto mob_pos = mob->entity.get_position();

        float distance_x = std::abs(hero_pos.x - mob_pos.x);
        float distance_y = std::abs(hero_pos.y - mob_pos.y);

        glm::vec2 dir {0.0f, 0.0f};

        if (distance_x > 3.0f) {
            dir.x = hero_pos.x > mob_pos.x ? 1.0f : -1.0f;
        }

        if (distance_y > 3.0f) {
            dir.y = hero_pos.y > mob_pos.y ? 1.0f : -1.0f;
        }

        if (hero.get_mount() == Mount::Walk && distance_x < 12.0f && distance_y < 12.0f) {
            for (auto j = 0u; j < gen.friendly_mobs[v.continent].size(); j++) {
                if (gen.friendly_mobs[v.continent][j] == mob->id) {
                    int size = 0;
                    for (int i = 0; i < 5; i++) {
                        if (v.army[i] != -1) {
                            size++;
                        }
                    }

                    const std::string descriptor = fmt::format("{} {}", bty::get_descriptor(mob->counts[0]), kUnits[mob->army[0]].name_plural);

                    if (size == 5) {
                        ds.show_dialog({
                            .x = 1,
                            .y = 21,
                            .w = 30,
                            .h = 6,
                            .strings = {
                                {1, 1, descriptor},
                                {3, 3, " flee in terror at the\nsight of your vast army."},
                            },
                            .callbacks = {.confirm = [&mob](int) {
                                mob->dead = true;
                            }},
                        });
                    }
                    else {
                        ds.show_dialog({
                            .x = 1,
                            .y = 18,
                            .w = 30,
                            .h = 9,
                            .strings = {
                                {1, 1, descriptor},
                                {3, 3, "with desires of greater\nglory, wish to join you."},
                            },
                            .options = {
                                {13, 6, "Accept"},
                                {13, 7, "Decline"},
                            },
                            .callbacks = {.confirm = [this, mob](int opt) {
                                if (opt == 0) {
                                    add_unit_to_army(mob->army[0], mob->counts[0]);
                                }
                                mob->dead = true;
                            }},
                        });
                    }
                    return;
                }
            }

            s_battle.show(mob->army, mob->counts, false, -1);
            ss.push(&s_battle, std::bind(&Ingame::battle_pop, this, std::placeholders::_1));
            battle_mob = mob->id;
            return;
        }

        mob->entity.update(dt);
        mob->entity.set_flip(hero_pos.x < mob_pos.x);

        move_mob(*mob, dt, dir);
    }
}

void Ingame::collide_sign(const Tile &tile)
{
    int index = 0;
    auto *tiles = map.get_data(v.continent);
    for (int y = 63; y >= 0; y--) {
        for (int x = 0; x < 64; x++) {
            if (tiles[y * 64 + x] == Tile_GenSign) {
                if (x == tile.tx && y == tile.ty) {
                    auto pop_ds = [this](bty::Dialog &) {
                        ds.pop();
                    };
                    ds.show_dialog({
                        .x = 1,
                        .y = 21,
                        .w = 30,
                        .h = 6,
                        .strings = {
                            {1, 1, fmt::format("A sign reads\n\n{}", kSigns[index + v.continent * 22])},
                        },
                        .callbacks = {
                            .up = pop_ds,
                            .down = pop_ds,
                            .left = pop_ds,
                            .right = pop_ds,
                        },
                    });
                    break;
                }
                index++;
            }
        }
    }
}

bool Ingame::events(const Tile &tile, bool &teleport)
{
    bool event_tile {true};

    switch (tile.id) {
        case Tile_Sign:
            [[fallthrough]];
        case Tile_GenSign:
            collide_sign(tile);
            break;
        case Tile_Chest:
            collide_chest(tile);
            break;
        case Tile_Town:
            collide_town(tile);
            break;
        case Tile_CastleB:
            if (tile.tx == 11 && tile.ty == 56) {
                kings_castle.view();
                ss.push(&kings_castle, nullptr);
            }
            else {
                collide_castle(tile);
            }
            break;
        case Tile_AfctRing:
            [[fallthrough]];
        case Tile_AfctAmulet:
            [[fallthrough]];
        case Tile_AfctAnchor:
            [[fallthrough]];
        case Tile_AfctCrown:
            [[fallthrough]];
        case Tile_AfctScroll:
            [[fallthrough]];
        case Tile_AfctShield:
            [[fallthrough]];
        case Tile_AfctSword:
            [[fallthrough]];
        case Tile_AfctBook:
            collide_artifact(tile);
            break;
        case Tile_GenWizardCave:
            wizard();
            break;
        case Tile_ShopCave:
            if (glm::ivec2 {tile.tx, tile.ty} == gen.teleport_cave_tiles[v.continent][0] || glm::ivec2 {tile.tx, tile.ty} == gen.teleport_cave_tiles[v.continent][1]) {
                collide_teleport_cave(tile);
                teleport = true;
            }
            else {
                collide_shop(tile);
            }
            break;
        case Tile_ShopTree:
            [[fallthrough]];
        case Tile_ShopDungeon:
            [[fallthrough]];
        case Tile_ShopWagon:
            collide_shop(tile);
            break;
        default:
            event_tile = false;
            break;
    }

    return event_tile;
}

void Ingame::collide_shop(const Tile &tile)
{
    int shop_id = -1;
    for (auto i = 0u; i < gen.shops[v.continent].size(); i++) {
        if (gen.shops[v.continent][i].x == tile.tx && gen.shops[v.continent][i].y == tile.ty) {
            shop_id = i;
            break;
        }
    }
    if (shop_id == -1) {
        spdlog::warn("Failed to find shop at [{}] {} {}", v.continent, tile.tx, tile.ty);
    }
    else {
        shop.view(gen.shops[v.continent][shop_id]);
        ss.push(&shop, nullptr);
    }
}

void Ingame::collide_chest(const Tile &tile)
{
    map.set_tile(tile, v.continent, Tile_Grass);

    if (v.continent < 3 && tile.tx == gen.sail_map_tiles[v.continent].x && tile.ty == gen.sail_map_tiles[v.continent].y) {
        gen.sail_maps_found[v.continent + 1] = true;
        ds.show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = {
                {3, 2, "Hidden within an ancient"},
                {3, 3, "chest, you find maps and"},
                {1, 4, "charts describing passage to"},
                {10, 6, kContinentNames[v.continent + 1]},
            },
        });
    }
    else if (tile.tx == gen.continent_map_tiles[v.continent].x && tile.ty == gen.continent_map_tiles[v.continent].y) {
        gen.continent_maps_found[v.continent] = true;
        ds.show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = {
                {3, 2, "Peering through a magical"},
                {1, 3, "orb you are able to view the"},
                {2, 4, "entire continent. Your map"},
                {3, 5, "of this area is complete."},
            },
            .callbacks = {
                .confirm = [this](int) {
                    view_continent.update_info(v, true, true);
                    ss.push(&view_continent, nullptr);
                },
            },
        });
    }
    else {
        chest_roll(v, gen, ds, hud);
    }
}

void Ingame::collide_castle(const Tile &tile)
{
    int castle_id = -1;

    for (int i = 0; i < 26; i++) {
        if (kCastleInfo[i].x == tile.tx && kCastleInfo[i].y == 63 - tile.ty && kCastleInfo[i].continent == v.continent) {
            castle_id = i;
        }
    }

    if (castle_id == -1) {
        spdlog::warn("Failed to find castle at [{},{}] in {}", tile.tx, tile.ty, kContinentNames[v.continent]);
        return;
    }

    int occupier = gen.castle_occupants[castle_id];

    if (occupier == -1) {
        s_garrison.view(castle_id);
        ss.push(&s_garrison, nullptr);
        return;
    }

    std::string occ_name = occupier == 0x7F ? "Various groups of monsters" : fmt::format("{} and", kVillains[occupier][0]);
    std::string line_two = occupier == 0x7F ? "occupy this castle" : "army occupy this castle";

    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, fmt::format("Castle {}", kCastleInfo[castle_id].name)},
            {1, 3, occ_name},
            {2, 4, line_two},
        },
        .options = {
            {11, 6, "Lay siege."},
            {11, 7, "Venture on."},
        },
        .callbacks = {
            .confirm = [this, castle_id](int opt) {
                if (opt == 0) {
                    s_battle.show(gen.castle_armies[castle_id], gen.castle_counts[castle_id], true, castle_id);
                    garrison_castle_id = castle_id;
                    ss.push(&s_battle, std::bind(&Ingame::battle_pop, this, std::placeholders::_1));
                }
            },
        },
    });

    v.visited_castles[castle_id] = true;
}

void Ingame::collide_town(const Tile &tile)
{
    int town_id = -1;

    for (int i = 0; i < 26; i++) {
        if (kTownInfo[i].continent == v.continent && kTownInfo[i].x == tile.tx && 63 - kTownInfo[i].y == tile.ty) {
            town_id = i;
            break;
        }
    }

    if (town_id == -1) {
        spdlog::warn("Couldn't find town at {}, {}", tile.tx, tile.ty);
        return;
    }

    v.visited_towns[town_id] = true;

    town.view(town_id, gen.town_units[town_id], gen.town_spells[town_id]);
    ss.push(&town, nullptr);
}

void Ingame::collide_artifact(const Tile &tile)
{
    map.set_tile(tile, v.continent, Tile_Grass);

    static constexpr const int artifact_ids[8] = {
        Tile_AfctScroll,
        Tile_AfctShield,
        Tile_AfctCrown,
        Tile_AfctAmulet,
        Tile_AfctRing,
        Tile_AfctAnchor,
        Tile_AfctBook,
        Tile_AfctSword,
    };

    int artifact = -1;

    for (int i = 0; i < 8; i++) {
        if (tile.id == artifact_ids[i]) {
            artifact = i;
            break;
        }
    }

    gen.artifacts_found[artifact] = true;

    hud.set_puzzle(gen.villains_captured.data(), gen.artifacts_found.data());

    static constexpr const char *const kArtifactMessages[8] = {
        {
            "  Freeing a virtuous maiden\n"
            "   from the clutches of a\n"
            "  despicable criminal, you\n"
            "      have been granted\n\n"
            "  The Articles of Nobility!",
        },
        {
            "  Challenged to a joust by\n"
            " the dread Dark Knight, you\n"
            " quickly dispose of him and\n"
            "           receive\n\n"
            "  The Shield of Protection!\n",
        },
        {
            "  Resting on a throne in a\n"
            "  phantom castle, you have\n"
            "            found\n\n"
            "    The Crown of Command!",
        },
        {
            " Hidden within an enchanted\n"
            "       grove, you find\n\n\n"
            " The Amulet of Augmentation!",
        },
        {
            " Ridding the countryside of\n"
            "   a ferocious beast, the\n"
            "Magistrate presents you with\n\n"
            "    The Ring of Heroism!",
        },
        {
            "You discover ancient scrolls\n"
            " that describe the patterns\n"
            " of the oceans. Mariners, in\n"
            " gratitude, bestow upon you\n\n"
            "  The Anchor of Admirality!",
        },
        {
            " In the study of a deserted\n"
            "  wizard's tower, you have\n"
            "            found\n\n"
            "     The Book of Necros!",
        },
        {
            " Following rumors of a great\n"
            "   and powerful sword, you\n"
            "defeat its fearsome guardian\n"
            "   and gain possession of\n\n"
            "    The Sword of Prowess!",
        },
    };

    static constexpr const char *const kArtifactMessageCommon = {
        "   ...and a piece of the\n"
        " map to the stolen scepter.",
    };

    ds.show_dialog({
        .x = 1,
        .y = 16,
        .w = 30,
        .h = 11,
        .strings = {
            {1, 1, kArtifactMessages[artifact]},
            {1, 8, kArtifactMessageCommon},
        },
    });
}

void Ingame::defeat_pop(int ret)
{
    switch (ret) {
        case 0:           // play again
            ss.pop(0);    // reset
            break;
        default:
            break;
    }
}

void Ingame::battle_pop(int ret)
{
    hud.set_hud_frame();
    hud.set_hero(v.hero, v.rank);
    hud.set_days(v.days);
    hud.set_gold(v.gold);
    hud.set_contract(v.contract);
    hud.set_siege(v.siege);
    hud.set_puzzle(gen.villains_captured.data(), gen.artifacts_found.data());
    switch (ret) {
        case 0:    // victory encounter
            gen.mobs[v.continent][battle_mob % 40].dead = true;
            break;
        case 1:    // victory siege
            s_garrison.view(garrison_castle_id);
            ss.push(&s_garrison, nullptr);
            break;
        case 2:    // defeat/give up
            disgrace();
            break;
        default:
            break;
    }
}

void Ingame::wizard()
{
    s_wizard.view(ds);
    ss.push(&s_wizard, [this](int ret) {
        if (ret == 0) {
            map.set_tile({11, 63 - 19, -1}, 0, Tile_Grass);
        }
    });
}

void Ingame::search_area()
{
    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, "Search..."},
            {1, 3, "It will take 10 days to do a"},
            {1, 4, "search of this area, search?"},
        },
        .options = {
            {14, 6, "No"},
            {14, 7, "Yes"},
        },
        .callbacks = {
            .confirm = [this](int opt) {
                if (opt == 1) {
                    do_search();
                }
            },
        },
    });
}

void Ingame::do_search()
{
    if (sceptre_continent != v.continent) {
        search_fail();
        return;
    }

    int range = 3;
    int offset = range / 2;

    auto tile = hero.get_tile();
    bool found = false;
    for (int i = 0; i <= range; i++) {
        for (int j = 0; j <= range; j++) {
            int x = tile.tx - offset + i;
            int y = tile.ty - offset + j;

            if (x < 0 || x > 63 || y < 0 || y > 63) {
                continue;
            }

            if (x == sceptre_x && y == sceptre_y) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        search_fail();
    }
    else {
        victory();
    }
}

void Ingame::search_fail()
{
    ds.show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 3, "Your search of this area has\n      revealed nothing."},
        },
        .callbacks = {
            .confirm = [this](int) {
                end_week(true);
            },
        },
    });
}

void Ingame::end_week(bool search)
{
    if ((v.days % 5) != 0) {
        v.days = (v.days / 5) * 5;
    }
    else {
        v.days = ((v.days / 5) - 1) * 5;
    }
    if (v.days == 0) {
        defeat();
    }
    else {
        v.weeks_passed++;
        end_week_astrology(search);
        hud.set_gold(v.gold);
    }
    day_timer.reset();
    hud.set_days(v.days);
}

void Ingame::fly_land()
{
    auto mount = hero.get_mount();

    if (mount == Mount::Fly) {
        c2AABB ent_shape = hero.get_aabb();

        int range = 4;
        int offset = range / 2;

        /* No need to check surrounding tiles if we can't land on the current tile anyway. */
        if (map.get_tile(hero.get_center(), v.continent).id != Tile_Grass) {
            goto end_loop;
        }

        /* Make sure we're not intersecting any collidable tiles when we land. */
        for (int i = 0; i <= range; i++) {
            for (int j = 0; j <= range; j++) {
                int x = v.x - offset + i;
                int y = v.y - offset + j;

                if (x == v.x && y == v.y) {
                    continue;
                }

                if (x < 0 || x > 63 || y < 0 || y > 63) {
                    continue;
                }

                c2AABB tile_shape;
                tile_shape.min.x = 48.0f * x;
                tile_shape.min.y = 40.0f * y;
                tile_shape.max.x = tile_shape.min.x + 48.0f;
                tile_shape.max.y = tile_shape.min.y + 40.0f;

                if (map.get_tile(ent_shape.min.x, ent_shape.min.y, v.continent).id != Tile_Grass && !c2AABBtoAABB(ent_shape, tile_shape)) {
                    goto end_loop;
                }
            }
        }
        hero.set_mount(Mount::Walk);
    end_loop:;
    }
    else if (mount == Mount::Walk) {
        bool can_fly = true;
        for (int i = 0; i < 5; i++) {
            if (v.army[i] == -1) {
                continue;
            }
            if (!(kUnits[v.army[i]].abilities & AbilityFly)) {
                can_fly = false;
                break;
            }
        }
        if (can_fly) {
            hero.set_mount(Mount::Fly);
        }
    }
}

void Ingame::day_tick()
{
    v.days--;
    if (v.days == 0) {
        hud.set_days(v.days);
        s_defeat.show(v.hero);
        ss.push(&s_defeat, std::bind(&Ingame::defeat_pop, this, std::placeholders::_1));
    }
    else {
        v.days_passed_this_week++;
        if (v.days_passed_this_week == 5) {
            v.days_passed_this_week = 0;
            v.weeks_passed++;
            end_week_astrology(false);
            hud.set_gold(v.gold);
        }
        hud.set_days(v.days);
    }
}

void Ingame::timestop_tick()
{
    v.timestop--;

    if (v.timestop == 0) {
        hud.clear_timestop();
    }
    else {
        hud.set_timestop(v.timestop);
    }
}

std::vector<Mob *> Ingame::get_mobs_in_range(int x, int y, int range)
{
    std::vector<Mob *> moblist;

    for (auto &mob : gen.mobs[v.continent]) {
        if (mob.dead) {
            continue;
        }
        if (std::abs(x - mob.tile.x) <= range && std::abs(y - mob.tile.y) <= range) {
            moblist.push_back(&mob);
        }
    }

    return moblist;
}

void Ingame::automove_tick()
{
    v.auto_move = false;
}

void Ingame::automove(float dt)
{
    float vel = 120.0f * dt;
    float dx = v.auto_move_dir.x * vel;
    float dy = v.auto_move_dir.y * vel;

    auto aabb = hero.get_aabb();
    auto last_pos = hero.get_position();
    last_tile = map.get_tile(aabb.min.x + 4, aabb.min.y + 4, v.continent);

    Tile center_tile {-1, -1, -1};
    Tile collided_tile {-1, -1, -1};

    move_increment(aabb, dx, 0, center_tile, collided_tile, &Ingame::hero_can_move, false);
    move_increment(aabb, 0, dy, center_tile, collided_tile, &Ingame::hero_can_move, false);

    hero.set_position(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);

    if (center_tile.tx != last_tile.tx || center_tile.ty != last_tile.ty) {
        last_tile = center_tile;
        last_event_tile = {-1, -1, -1};
    }

    update_camera();
}

void Ingame::save_state(const std::string &filename)
{
    const auto path = fmt::format("saves/{}", filename);

    spdlog::info("Saving to {}", path);

    std::ofstream f(path, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!f.good()) {
        spdlog::warn("Failed to open file '{}' for saving", path);
        return;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 40; j++) {
            f.write((char *)gen.mobs[i][j].army.data(), 5 * 4);
            f.write((char *)gen.mobs[i][j].counts.data(), 5 * 4);
            int dead = gen.mobs[i][j].dead;
            f.write((char *)&dead, 4);
            f.write((char *)&gen.mobs[i][j].id, 4);
            f.write((char *)&gen.mobs[i][j].sprite_id, 4);
            f.write((char *)&gen.mobs[i][j].tile, sizeof(Tile));
            auto pos = gen.mobs[i][j].entity.get_position();
            f.write((char *)&pos, 8);
        }
    }

    f.write((char *)&v.hero, 4);
    f.write((char *)&v.rank, 4);
    f.write((char *)&v.days, 4);
    f.write((char *)&v.gold, 4);
    f.write((char *)&v.diff, 4);
    f.write((char *)&v.magic, 4);
    f.write((char *)&v.siege, 4);
    f.write((char *)&v.contract, 4);
    f.write((char *)&v.x, 4);
    f.write((char *)&v.y, 4);
    f.write((char *)&v.continent, 4);
    f.write((char *)&v.leadership, 4);
    f.write((char *)&v.commission, 4);
    f.write((char *)v.army.data(), 4 * 5);
    f.write((char *)v.counts.data(), 4 * 5);
    f.write((char *)v.morales.data(), 4 * 5);
    f.write((char *)&v.followers_killed, 4);
    f.write((char *)&v.score, 4);
    f.write((char *)&v.weeks_passed, 4);
    f.write((char *)&v.timestop, 4);
    f.write((char *)&v.permanent_leadership, 4);
    f.write((char *)&v.known_spells, 4);
    f.write((char *)&v.days_passed_this_week, 4);
    f.write((char *)&v.spell_power, 4);
    f.write((char *)v.visited_castles.data(), 1 * 26);
    f.write((char *)v.visited_towns.data(), 1 * 26);
    f.write((char *)&v.auto_move, 1);
    f.write((char *)&v.auto_move_dir, 8);
    f.write((char *)&v.boat_x, 4);
    f.write((char *)&v.boat_y, 4);
    f.write((char *)&v.boat_c, 4);
    f.write((char *)&v.boat_rented, 1);
    f.write((char *)&sceptre_x, 4);
    f.write((char *)&sceptre_y, 4);
    f.write((char *)&sceptre_continent, 4);
    f.write((char *)&last_water_x, 4);
    f.write((char *)&last_water_y, 4);
    f.write((char *)&last_tile, sizeof(Tile));
    f.write((char *)&last_event_tile, sizeof(Tile));
    f.write((char *)gen.artifacts_found.data(), 1 * 8);
    f.write((char *)gen.continent_maps_found.data(), 1 * 4);
    f.write((char *)gen.sail_maps_found.data(), 1 * 4);
    f.write((char *)gen.villains_found.data(), 1 * 17);
    f.write((char *)gen.villains_captured.data(), 1 * 17);
    for (int i = 0; i < 26; i++) {
        f.write((char *)gen.castle_armies[i].data(), 4 * 5);
        f.write((char *)gen.castle_counts[i].data(), 4 * 5);
        f.write((char *)&gen.castle_occupants[i], 4);
        f.write((char *)gen.garrison_armies[i].data(), 4 * 5);
        f.write((char *)gen.garrison_counts[i].data(), 4 * 5);
        f.write((char *)&gen.town_spells[i], 4);
        f.write((char *)&gen.town_units[i], 4);
    }

    f.write((char *)gen.sail_map_tiles.data(), 24);
    f.write((char *)gen.continent_map_tiles.data(), 32);
    f.write((char *)gen.continent_map_tiles.data(), 64);
    for (int i = 0; i < 4; i++) {
        auto num_shops = gen.shops[i].size();
        f.write((char *)&num_shops, 4);
        f.write((char *)gen.shops[i].data(), sizeof(ShopInfo) * num_shops);
    }

    for (int i = 0; i < 4; i++) {
        auto num_friendly_mobs = gen.friendly_mobs[i].size();
        f.write((char *)&num_friendly_mobs, 4);
        f.write((char *)gen.friendly_mobs[i].data(), 4 * num_friendly_mobs);
    }

    for (int i = 0; i < 4; i++) {
        f.write((char *)v.visited_tiles.data(), 4096);
    }

    for (int i = 0; i < 4; i++) {
        f.write((char *)map.get_data(i), 4096);
    }

    auto hero_pos = hero.get_position();
    auto mount = hero.get_mount();
    bool flip = hero.get_flip();
    f.write((char *)&hero_pos, 8);
    f.write((char *)&mount, 4);
    f.write((char *)&flip, 1);

    auto boat_pos = boat.get_position();
    f.write((char *)&boat_pos, 8);

    auto boat_flip = boat.get_flip();
    f.write((char *)&boat_flip, 1);
}

void Ingame::load_state(const std::string &filename)
{
    const auto path = fmt::format("saves/{}", filename);

    spdlog::info("Loading from {}", path);

    std::ifstream f(path, std::ios::in | std::ios::binary);

    if (!f.good()) {
        spdlog::warn("Failed to open file '{}' for loading", path);
        return;
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 40; j++) {
            gen.mobs[i][j].entity.set_texture(nullptr);
            f.read((char *)gen.mobs[i][j].army.data(), 5 * 4);
            f.read((char *)gen.mobs[i][j].counts.data(), 5 * 4);
            f.read((char *)&gen.mobs[i][j].dead, 4);
            f.read((char *)&gen.mobs[i][j].id, 4);
            f.read((char *)&gen.mobs[i][j].sprite_id, 4);
            f.read((char *)&gen.mobs[i][j].tile, sizeof(Tile));
            glm::vec2 pos;
            f.read((char *)&pos, 8);
            gen.mobs[i][j].entity.set_position(pos);
            if (gen.mobs[i][j].sprite_id != -1 && !gen.mobs[i][j].dead) {
                gen.mobs[i][j].entity.set_texture(unit_textures[gen.mobs[i][j].sprite_id]);
            }
        }
    }

    f.read((char *)&v.hero, 4);
    f.read((char *)&v.rank, 4);
    f.read((char *)&v.days, 4);
    f.read((char *)&v.gold, 4);
    f.read((char *)&v.diff, 4);
    f.read((char *)&v.magic, 4);
    f.read((char *)&v.siege, 4);
    f.read((char *)&v.contract, 4);
    f.read((char *)&v.x, 4);
    f.read((char *)&v.y, 4);
    f.read((char *)&v.continent, 4);
    f.read((char *)&v.leadership, 4);
    f.read((char *)&v.commission, 4);
    f.read((char *)v.army.data(), 4 * 5);
    f.read((char *)v.counts.data(), 4 * 5);
    f.read((char *)v.morales.data(), 4 * 5);
    f.read((char *)&v.followers_killed, 4);
    f.read((char *)&v.score, 4);
    f.read((char *)&v.weeks_passed, 4);
    f.read((char *)&v.timestop, 4);
    f.read((char *)&v.permanent_leadership, 4);
    f.read((char *)&v.known_spells, 4);
    f.read((char *)&v.days_passed_this_week, 4);
    f.read((char *)&v.spell_power, 4);
    f.read((char *)v.visited_castles.data(), 1 * 26);
    f.read((char *)v.visited_towns.data(), 1 * 26);
    f.read((char *)&v.auto_move, 1);
    f.read((char *)&v.auto_move_dir, 8);
    f.read((char *)&v.boat_x, 4);
    f.read((char *)&v.boat_y, 4);
    f.read((char *)&v.boat_c, 4);
    f.read((char *)&v.boat_rented, 1);
    f.read((char *)&sceptre_x, 4);
    f.read((char *)&sceptre_y, 4);
    f.read((char *)&sceptre_continent, 4);
    f.read((char *)&last_water_x, 4);
    f.read((char *)&last_water_y, 4);
    f.read((char *)&last_tile, sizeof(Tile));
    f.read((char *)&last_event_tile, sizeof(Tile));
    f.read((char *)gen.artifacts_found.data(), 1 * 8);
    f.read((char *)gen.continent_maps_found.data(), 1 * 4);
    f.read((char *)gen.sail_maps_found.data(), 1 * 4);
    f.read((char *)gen.villains_found.data(), 1 * 17);
    f.read((char *)gen.villains_captured.data(), 1 * 17);
    for (int i = 0; i < 26; i++) {
        f.read((char *)gen.castle_armies[i].data(), 4 * 5);
        f.read((char *)gen.castle_counts[i].data(), 4 * 5);
        f.read((char *)&gen.castle_occupants[i], 4);
        f.read((char *)gen.garrison_armies[i].data(), 4 * 5);
        f.read((char *)gen.garrison_counts[i].data(), 4 * 5);
        f.read((char *)&gen.town_spells[i], 4);
        f.read((char *)&gen.town_units[i], 4);
    }

    f.read((char *)gen.sail_map_tiles.data(), 24);
    f.read((char *)gen.continent_map_tiles.data(), 32);
    f.read((char *)gen.continent_map_tiles.data(), 64);
    for (int i = 0; i < 4; i++) {
        int num_shops = 0;
        f.read((char *)&num_shops, 4);
        gen.shops[i].resize(num_shops);
        f.read((char *)gen.shops[i].data(), sizeof(ShopInfo) * num_shops);
    }

    for (int i = 0; i < 4; i++) {
        int num_friendly_mobs = 0;
        f.read((char *)&num_friendly_mobs, 4);
        gen.friendly_mobs[i].resize(num_friendly_mobs);
        f.read((char *)gen.friendly_mobs[i].data(), 4 * num_friendly_mobs);
    }

    for (int i = 0; i < 4; i++) {
        f.read((char *)v.visited_tiles[i].data(), 4096);
    }

    for (int i = 0; i < 4; i++) {
        f.read((char *)map.get_data(i), 4096);
    }
    map.create_geometry();

    hud.set_hero(v.hero, v.rank);
    hud.set_days(v.days);
    hud.set_gold(v.gold);
    hud.set_color(bty::get_box_color(v.diff));
    hud.set_magic(v.magic);
    hud.set_siege(v.siege);
    hud.set_contract(v.contract);
    hud.set_puzzle(gen.villains_captured.data(), gen.artifacts_found.data());

    if (v.timestop != 0) {
        hud.set_timestop(v.timestop);
    }

    ds.set_default_color(bty::get_box_color(v.diff));

    glm::vec2 hero_pos;
    f.read((char *)&hero_pos, 8);
    hero.set_position(hero_pos);
    update_camera();

    Mount mount;
    f.read((char *)&mount, 4);
    hero.set_mount(mount);

    bool flip;
    f.read((char *)&flip, 1);
    hero.set_flip(flip);

    day_timer.reset();
    timestop_timer.reset();
    automove_timer.reset();

    glm::vec2 boat_pos;
    f.read((char *)&boat_pos, 8);
    boat.set_position(boat_pos);

    bool boat_flip;
    f.read((char *)&boat_flip, 1);
    boat.set_flip(boat_flip);
}
