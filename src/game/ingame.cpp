#include "game/ingame.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <random>

#include "assets.hpp"
#include "bounty.hpp"
#include "game/army-gen.hpp"
#include "game/chest.hpp"
#include "game/hud.hpp"
#include "game/scene-stack.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"

static constexpr int kShopUnits[24] = {
    0x00,
    0x01,
    0x07,
    0x04,
    0x03,
    0x06,
    0x0c,
    0x05,
    0x0b,
    0x09,
    0x0f,
    0x09,
    0x0d,
    0x10,
    0x11,
    0x13,
    0x00,
    0x00,
    0x16,
    0x15,
    0x14,
    0x18,
    0x17,
    0x00,
};

static constexpr int kMaxShopCounts[25] = {
    250,
    200,
    0,
    150,
    150,
    100,
    250,
    200,
    0,
    100,
    0,
    150,
    100,
    25,
    0,
    200,
    100,
    25,
    0,
    25,
    25,
    50,
    50,
    25,
    25,
};

void sort_army(std::array<int, 5> &army, std::array<int, 5> &counts)
{
    int last_free = -1;
    for (int i = 0; i < 5; i++) {
        if (last_free == -1 && army[i] == -1) {
            last_free = i;
        }
        else if (last_free != -1 && army[i] != -1) {
            army[last_free] = army[i];
            army[i] = -1;
            counts[last_free] = counts[i];
            counts[i] = -1;
            last_free = i;
        }
    }
}

Ingame::Ingame(GLFWwindow *window, SceneStack &ss, DialogStack &ds, bty::Assets &assets, Hud &hud)
    : window(window)
    , ss(ss)
    , ds(ds)
    , hud(hud)
    , view_army(ss, assets)
    , view_char(ss, assets)
    , view_continent(ss, assets)
    , view_contract(ss, assets, v, gen, hud.get_contract())
    , view_puzzle(ss, assets)
    , kings_castle(ss, assets, hud, v)
    , shop(ss, assets, v, gen, hud)
    , town(ss, ds, assets, v, gen, hud, view_contract)
    , s_defeat(ss, ds, assets, hud)
    , s_battle(ss, ds, assets, v, gen, view_army, view_char)
{
    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    map.load(assets);
    hero.load(assets);

    ui_cam = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);
    map_cam = ui_cam;
}

void Ingame::setup(int hero, int diff)
{
    auto color = bty::get_box_color(diff);

    ds.set_default_color(color);
    hud.set_color(color);
    hud.set_hud_frame();

    day_clock = 0;

    v.weeks_passed = 0;
    v.days_passed_this_week = 0;
    v.x = 0;
    v.y = 0;

    for (int i = 0; i < 26; i++) {
        v.visited_towns[i] = false;
        v.visited_castles[i] = false;
    }

    v.auto_move = false;
    v.auto_move_dir = {0, 0};
    v.auto_move_timer = 0.0f;

    for (int i = 0; i < 14; i++) {
        v.spells[i] = 5;
    }
    v.num_spells = 0;

    for (int i = 0; i < 4; i++) {
        v.visited_tiles[i].clear();
        v.visited_tiles[i].resize(4096);
        std::fill(v.visited_tiles[i].begin(), v.visited_tiles[i].end(), 0xFF);
        gen.sail_maps_found[i] = i == 0;
        gen.continent_maps_found[i] = false;
        v.tiles[i] = map.get_data(i);
    }

    for (int i = 0; i < 5; i++) {
        v.army[i] = -1;
        v.counts[i] = 0;
        v.morales[i] = 0;
    }

    for (int i = 0; i < 8; i++) {
        gen.artifacts_found[i] = false;
    }

    v.boat_rented = false;
    v.hero = hero;
    v.rank = 0;
    v.days = kDays[diff];
    v.diff = diff;
    v.known_spells = 0;
    v.contract = 17;
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
    timestop_timer = 0;

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

void Ingame::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    map.draw(map_cam, v.continent);
    draw_mobs(gfx);
    hero.draw(gfx, map_cam);
    hud.draw(gfx, ui_cam);
}

void Ingame::key(int key, int action)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            pause();
        }
        else if (key == GLFW_KEY_G) {
            hero.set_mount(hero.get_mount() == Mount::Fly ? Mount::Walk : Mount::Fly);
        }
        else if (key == GLFW_KEY_B) {
            hero.set_mount(hero.get_mount() == Mount::Walk ? Mount::Boat : Mount::Walk);
        }
        else if (key == GLFW_KEY_P) {
            hero.set_debug(!hero.get_debug());
            for (auto i = 0u; i < gen.mobs[v.continent].size(); i++) {
                if (gen.mobs[v.continent][i].dead) {
                    continue;
                }
                gen.mobs[v.continent][i].entity.set_debug(!gen.mobs[v.continent][i].entity.get_debug());
            }
        }
        else if (key == GLFW_KEY_E) {
            v.days = 1;
            day_clock = 15.99f;
        }
    }
}

void Ingame::update(float dt)
{
    if (!ds.empty() || ss.get() != this || hud.get_error()) {
        return;
    }

    if (v.auto_move) {
        auto_move(dt);
    }
    else {
        move_hero(get_move_input(), dt);
    }

    map.update(dt);
    hero.update(dt);

    if (v.timestop) {
        update_timestop_clock(dt);
    }
    else {
        update_day_clock(dt);
        update_mobs(dt);
    }
}

void Ingame::gen_tiles()
{
    for (int i = 0; i < 26; i++) {
        gen.castle_occupants[i] = 0x7F;
        for (int j = 0; j < 5; j++) {
            gen.castle_armies[i][j] = -1;
            gen.castle_counts[i][j] = 0;
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

    static constexpr int kAvailableUnitsPerContinent[4][6] = {
        {
            Peasants,
            Sprites,
            Orcs,
            Skeletons,
            Wolves,
            Gnomes,
        },
        {Dwarves,
         Zombies,
         Nomads,
         Elves,
         Ogres,
         Elves},
        {Ghosts,
         Barbarians,
         Trolls,
         Druids,
         Peasants,
         Peasants},
        {Giants,
         Vampires,
         Archmages,
         Dragons,
         Demons,
         Peasants},
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
                    sort_army(mob.army, mob.counts);

                    int highest = 0;
                    for (int i = 0; i < 5; i++) {
                        if (mob.army[i] == -1) {
                            continue;
                        }
                        if (kUnits[mob.army[i]].hp >= kUnits[mob.army[highest]].hp) {
                            highest = i;
                        }
                    }

                    mob.entity.set_texture(unit_textures[mob.army[highest]]);
                    mob.entity.move_to_tile({x, y, Tile_Grass});

                    tiles[x + y * 64] = Tile_Grass;
                }
                n++;
            }
        }

        rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::shuffle(std::begin(random_tiles), std::end(random_tiles), rng_);

        int used_tiles = 0;
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
            shop.unit = kShopUnits[i + continent * 6];
            int max = kMaxShopCounts[shop.unit];
            int a = (rand() % kMaxShopCounts[shop.unit]) / 8;
            int b = (rand() % kMaxShopCounts[shop.unit]) / 16;
            int c = rand() % 4;
            shop.count = continent + a + b + max;
            tiles[tile.x + tile.y * 64] = kShopTileForUnit[shop.unit];
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
                shop.count = continent + a + b + max;
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
                shop.count = continent + a + b + max;
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

            tiles[tile.x + tile.y * 64] = Tile_Grass;

            gen.friendly_mobs[continent].push_back(num_mobs - 1);
        }

        /* Gen castle occupants */
        int used_castles = 0;
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
        for (int i = used_tiles; i < random_tiles.size(); i++) {
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
    const auto tile = hero.get_tile();
    auto *visited = v.visited_tiles[v.continent].data();
    auto *tiles = map.get_data(v.continent);

    int range = 4;
    int offset = range / 2;

    for (int i = 0; i <= range; i++) {
        for (int j = 0; j <= range; j++) {
            int x = tile.tx - offset + i;
            int y = tile.ty - offset + j;

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
    std::vector<DialogDef::StringDef> options;

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
    auto hero_tile = hero.get_tile();
    int x = hero_tile.tx;
    int y = hero_tile.ty;

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
    timestop_timer = 0;
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
    std::vector<DialogDef::StringDef> options(26);
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

void Ingame::add_unit_to_army(int id, int count)
{
    if (id < 0 || id >= UnitId::UnitCount) {
        spdlog::warn("add_unit_to_army: id out of range: {}", id);
        return;
    }

    for (int i = 0; i < 5; i++) {
        if (v.army[i] == id) {
            v.counts[i] += count;
            return;
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
        return;
    }

    int index = army_size++;

    v.army[index] = id;
    v.counts[index] = count;
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
    add_unit_to_army(unit, amt);

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

void Ingame::spell_raise_control()
{
    v.leadership += v.spell_power * 100;
    v.leadership &= 0xFFFF;
}

void Ingame::end_week_astrology()
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
            .confirm = [this](int) {
                end_week_budget();
            },
        },
    });

    v.leadership = v.permanent_leadership;
}

void Ingame::end_week_budget()
{
    int army_total = 0;
    int commission = v.commission;

    for (int i = 0; i < 26; i++) {
        if (gen.castle_occupants[i] == -1) {
            commission += 250;
        }
    }

    int gold = v.gold;
    int boat = v.boat_rented ? (gen.artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500) : 0;
    int balance = (commission + gold) - boat;

    std::vector<DialogDef::StringDef> strings;

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

    sort_army(v.army, v.counts);

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
}

void Ingame::dismiss()
{
    std::vector<DialogDef::StringDef> options;

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
    sort_army(v.army, v.counts);
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
                        view_continent.update_info(v);
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
                            end_week_astrology();
                            hud.set_gold(v.gold);
                        }
                        day_clock = 0;
                        hud.set_days(v.days);
                        break;
                    case 6:
                        view_puzzle.update_info(gen);
                        ss.push(&view_puzzle, nullptr);
                        break;
                    case 8:
                        dismiss();
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

void Ingame::move_hero(int move_flags, float dt)
{
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

    if (move_flags & DIR_FLAG_UP)
        dir.y -= 1.0f;
    if (move_flags & DIR_FLAG_DOWN)
        dir.y += 1.0f;
    if (move_flags & DIR_FLAG_LEFT)
        dir.x -= 1.0f;
    if (move_flags & DIR_FLAG_RIGHT)
        dir.x += 1.0f;

    float speed = hero.get_mount() == Mount::Fly ? 200.0f : 100.0f;
    float vel = speed * dt;
    float dx = dir.x * vel;
    float dy = dir.y * vel;

    auto manifold = hero.move(dx, dy, map, v.continent);
    hero.set_position(manifold.new_position);

    if (manifold.collided) {
        for (auto &tile : manifold.collided_tiles) {
            collide(tile);
        }
    }

    if (auto ht = hero.get_center(); !v.auto_move && (ht.x < 0 || ht.x > 3072 || ht.y < 0 || ht.y > 40 * 64)) {
        std::vector<DialogDef::StringDef> continents;

        for (int i = 0; i < 4; i++) {
            if (gen.sail_maps_found[i]) {
                continents.push_back({10, 3 + i, kContinents[i]});
            }
        }

        ds.show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = {{3, 1, "Sail to which continent?"}},
            .options = continents,
            .callbacks = {
                .confirm = std::bind(&Ingame::sail_to, this, std::placeholders::_1),
            },
        });
    }
    else {
        if (manifold.changed_tile) {
            hero.set_tile_info(manifold.new_tile);
            if (hero.get_mount() == Mount::Boat && manifold.new_tile.id == Tile_Grass) {
                hero.set_mount(Mount::Walk);
            }
            update_visited_tiles();
        }
    }

    update_camera();
}

void Ingame::update_day_clock(float dt)
{
    day_clock += dt;
    if (day_clock >= 16) {
        day_clock = 0;
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
                end_week_astrology();
                hud.set_gold(v.gold);
            }
            hud.set_days(v.days);
        }
    }
}

void Ingame::update_timestop_clock(float dt)
{
    timestop_timer += dt;

    if (timestop_timer >= 0.25f) {
        v.timestop--;
        timestop_timer = 0;
        hud.set_timestop(v.timestop);
    }

    if (v.timestop == 0) {
        hud.clear_timestop();
    }

    for (auto &mob : gen.mobs[v.continent]) {
        mob.entity.update(dt);
    }
}

void Ingame::sail_to(int continent)
{
    v.auto_move = true;

    if (continent == v.continent) {
        auto pos = hero.get_center();

        v.auto_move_dir = {0, 0};
        v.auto_move_timer = 1;
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
        v.auto_move_timer = 1;
        hero.set_moving(true);
        v.auto_move_dir = {0, 0};

        switch (continent) {
            case 0:
                // Up
                v.auto_move_dir.y = -1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(11, 64 - 1, v.continent));
                break;
            case 1:
                // Right
                v.auto_move_timer = 0.4f;
                v.auto_move_dir.x = 1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(0, 64 - 25, v.continent));
                break;
            case 2:
                // Down
                v.auto_move_timer = 0.6f;
                v.auto_move_dir.y = 1;
                hero.set_flip(false);
                hero.move_to_tile(map.get_tile(14, 0, v.continent));
                break;
            case 3:
                // Up
                v.auto_move_timer = 1;
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
    const auto &hero_tile = hero.get_tile();
    for (int i = 0; i < gen.mobs[v.continent].size(); i++) {
        if (gen.mobs[v.continent][i].dead) {
            continue;
        }
        if (std::abs(hero_tile.tx - gen.mobs[v.continent][i].tile.x) < 4 && std::abs(hero_tile.ty - gen.mobs[v.continent][i].tile.y) < 4) {
            gen.mobs[v.continent][i].entity.draw(gfx, map_cam);
        }
    }
}

void Ingame::collide_teleport_cave(const Tile &tile)
{
    auto dest = glm::ivec2 {tile.tx, tile.ty} == gen.teleport_cave_tiles[v.continent][0] ? gen.teleport_cave_tiles[v.continent][1] : gen.teleport_cave_tiles[v.continent][0];
    move_hero_to(dest.x, dest.y, v.continent);
}

void Ingame::update_mobs(float dt)
{
    const auto &hero_tile = hero.get_tile();
    const auto &hero_pos = hero.get_position();

    int continent = v.continent;

    for (int i = 0; i < gen.mobs[continent].size(); i++) {
        auto &mob = gen.mobs[continent][i];
        if (mob.dead) {
            continue;
        }

        if (std::abs(hero_tile.tx - mob.tile.x) < 4 && std::abs(hero_tile.ty - mob.tile.y) < 4) {
            auto &mob_pos = mob.entity.get_position();

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
                for (int j = 0; j < gen.friendly_mobs[v.continent].size(); j++) {
                    if (gen.friendly_mobs[v.continent][j] == i) {
                        int size = 0;
                        for (int i = 0; i < 5; i++) {
                            if (v.army[i] != -1) {
                                size++;
                            }
                        }

                        const std::string descriptor = fmt::format("{} {}", bty::get_descriptor(mob.counts[0]), kUnits[mob.army[0]].name_plural);

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
                                .callbacks = {.confirm = [this, &mob](int) {
                                    mob.dead = true;
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
                                .callbacks = {.confirm = [this, &mob](int opt) {
                                    if (opt == 0) {
                                        add_unit_to_army(mob.army[0], mob.counts[0]);
                                    }
                                    mob.dead = true;
                                }},
                            });
                        }
                        return;
                    }
                }

                s_battle.show(mob.army, mob.counts, false);
                ss.push(&s_battle, std::bind(&Ingame::battle_pop, this, std::placeholders::_1));
                battle_mob = i;
                return;
            }

            mob.entity.update(dt);
            mob.entity.set_flip(hero_pos.x < mob_pos.x);

            float speed = 70.0f;
            float vel = speed * dt;
            float dx = dir.x * vel;
            float dy = dir.y * vel;

            auto manifold = mob.entity.move(dx, dy, map, v.continent);

            mob.entity.set_position(manifold.new_position);

            for (auto j = 0u; j < gen.mobs[continent].size(); j++) {
                if (i == j || gen.mobs[continent][j].dead) {
                    continue;
                }

                auto &other_mob = gen.mobs[continent][j];
                auto &other_mob_pos = other_mob.entity.get_position();

                distance_x = std::abs(mob_pos.x - other_mob_pos.x);
                distance_y = std::abs(mob_pos.y - other_mob_pos.y);

                if (distance_x > 48.0f * 4 || distance_y > 40.0f * 4) {
                    continue;
                }

                if (dir.x > 0.5f && other_mob_pos.x > mob_pos.x && distance_x < 12.0f) {
                    mob.entity.Transformable::move(-dx, 0.0f);
                }
                else if (dir.x < -0.5f && other_mob_pos.x < mob_pos.x && distance_x < 12.0f) {
                    mob.entity.Transformable::move(-dx, 0.0f);
                }
                if (dir.y > 0.5f && other_mob_pos.y > mob_pos.y && distance_y < 8.0f) {
                    mob.entity.Transformable::move(0.0f, -dy);
                }
                else if (dir.y < -0.5f && other_mob_pos.y < mob_pos.y && distance_y < 8.0f) {
                    mob.entity.Transformable::move(0.0f, -dy);
                }
            }

            auto tile = mob.entity.get_tile();
            mob.tile.x = tile.tx;
            mob.tile.y = tile.ty;
        }
    }
}

void Ingame::auto_move(float dt)
{
    v.auto_move_timer -= dt;
    if (v.auto_move_timer <= 0) {
        v.auto_move = false;
    }

    float speed = 100;
    float vel = speed * dt;
    float dx = v.auto_move_dir.x * vel;
    float dy = v.auto_move_dir.y * vel;

    auto manifold = hero.move(dx, dy, map, v.continent);
    hero.set_position(manifold.new_position);
    update_visited_tiles();
    update_camera();
}

void Ingame::collide(const Tile &tile)
{
    switch (tile.id) {
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
        case Tile_ShopCave:
            if (glm::ivec2 {tile.tx, tile.ty} == gen.teleport_cave_tiles[v.continent][0] || glm::ivec2 {tile.tx, tile.ty} == gen.teleport_cave_tiles[v.continent][1]) {
                collide_teleport_cave(tile);
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
            break;
    }
}

void Ingame::collide_shop(const Tile &tile)
{
    int shop_id = -1;
    for (int i = 0; i < gen.shops[v.continent].size(); i++) {
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
        hud.set_error("You found a map!");
        ds.show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = {
                {3, 2, "Hidden within an ancient"},
                {3, 3, "chest, you find maps and"},
                {1, 4, "charts describing passage to"},
                {10, 6, kContinents[v.continent + 1]},
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
                {3, 2, "Perring through a magical"},
                {1, 3, "orb you are able to view the"},
                {2, 4, "entire continent. Your map"},
                {3, 5, "of this area is complete."},
            },
            .callbacks = {
                .confirm = [this](int opt) {
                    // set_state(GameState::ViewContinent);
                },
            },
        });
    }
    else {
        chest_roll(v, gen, ds);
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
        spdlog::warn("Failed to find castle at [{},{}] in {}", tile.tx, tile.ty, kContinents[v.continent]);
        return;
    }

    int occupier = gen.castle_occupants[castle_id];

    if (occupier == -1) {
        spdlog::warn("TODO: garrisoned castles");
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
                    s_battle.show(gen.castle_armies[castle_id], gen.castle_counts[castle_id], true);
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

    town.view(town_id, tile, v.continent, gen.town_units[town_id], gen.town_spells[town_id]);
    ss.push(&town, nullptr);
}

void Ingame::collide_artifact(const Tile &tile)
{
    map.set_tile(tile, v.continent, Tile_Grass);

    switch (tile.id) {
        case Tile_AfctRing:
            gen.artifacts_found[ArtiRingOfHeroism] = true;
            break;
        case Tile_AfctAmulet:
            gen.artifacts_found[ArtiAmuletOfAugmentation] = true;
            break;
        case Tile_AfctAnchor:
            gen.artifacts_found[ArtiAnchorOfAdmirality] = true;
            break;
        case Tile_AfctCrown:
            gen.artifacts_found[ArtiCrownOfCommand] = true;
            break;
        case Tile_AfctScroll:
            gen.artifacts_found[ArtiArticlesOfNobility] = true;
            break;
        case Tile_AfctShield:
            gen.artifacts_found[ArtiShieldOfProtection] = true;
            break;
        case Tile_AfctSword:
            gen.artifacts_found[ArtiSwordOfProwess] = true;
            break;
        case Tile_AfctBook:
            gen.artifacts_found[ArtiBookOfNecros] = true;
            break;
        default:
            break;
    }

    hud.set_puzzle(gen.villains_captured.data(), gen.artifacts_found.data());
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
    switch (ret) {
        case 0:    // victory encounter
            gen.mobs[v.continent][battle_mob].dead = true;
            break;
        case 1:    // victory siege
            break;
        case 2:    // defeat/give up
            disgrace();
            break;
        default:
            break;
    }
}