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
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/army-gen.hpp"
#include "game/chest.hpp"
#include "game/game-options.hpp"
#include "game/state.hpp"

#define CUTE_C2_IMPLEMENTATION
#include "game/cute_c2.hpp"
#include "game/hud.hpp"
#include "game/shop-gen.hpp"
#include "gfx/gfx.hpp"

Ingame::Ingame(bty::Engine &engine)
    : _engine(engine)
    , _dayTimer(16.0f, std::bind(&Ingame::dayTick, this))
    , _timestopTimer(0.25f, std::bind(&Ingame::timestopTick, this))
    , _automoveTimer(0.0f, std::bind(&Ingame::automoveTick, this))
    , _dbgCollisionRect({0.2f, 0.4f, 0.7f, 0.9f}, {8, 8}, {0, 0})
    , _chestGold(engine)
    , _chestCommission(engine)
    , _chestSpellPower(engine)
    , _chestSpellCapacity(engine)
    , _chestSpell(engine)
{
}

void Ingame::setup()
{
    int diff = State::difficulty;
    int hero = State::hero;

    auto color = bty::getBoxColor(diff);

    this->_spHero.setMoving(false);
    this->_spHero.setFlip(false);
    this->_spHero.setMount(Mount::Walk);

    _dayTimer.reset();
    _timestopTimer.reset();

    State::weeks_passed = 0;
    State::days_passed_this_week = 0;
    State::x = 0;
    State::y = 0;
    State::boat_x = -1;
    State::boat_y = -1;
    State::boat_c = -1;

    for (int i = 0; i < 26; i++) {
        State::visited_castles[i] = false;
    }

    State::auto_move = false;
    State::auto_move_dir = {0, 0};
    _automoveTimer.reset();

    for (int i = 0; i < 14; i++) {
        State::spells[i] = 5;
    }

    for (int i = 0; i < 4; i++) {
        State::visited_tiles[i].clear();
        State::visited_tiles[i].resize(4096);
        std::fill(State::visited_tiles[i].begin(), State::visited_tiles[i].end(), 0xFF);
        State::sail_maps_found[i] = false;
        State::continent_maps_found[i] = false;
        State::tiles[i] = _map.getTiles(i);
        for (auto j = 0u; j < State::mobs[i].size(); j++) {
            State::mobs[i][j].id = i * 40 + j;
            for (int k = 0; k < 5; k++) {
                State::mobs[i][j].army[k] = -1;
                State::mobs[i][j].counts[k] = 0;
            }
        }
    }

    State::sail_maps_found[0] = true;

    for (int i = 0; i < 5; i++) {
        State::army[i] = -1;
        State::counts[i] = 0;
        State::morales[i] = 0;
    }

    for (int i = 0; i < 8; i++) {
        State::artifacts_found[i] = false;
    }

    static constexpr int const kDays[4] = {
        900,
        600,
        400,
        200,
    };

    State::boat_rented = false;
    State::hero = hero;
    State::rank = 0;
    State::days = kDays[diff];
    State::known_spells = 0;
    State::contract = 17;
    State::score = 0;
    State::gold = kStartingGold[hero];
    State::commission = kRankCommission[hero][0];
    State::permanent_leadership = kRankLeadership[hero][0];
    State::leadership = State::permanent_leadership;
    State::max_spells = kRankSpells[hero][0];
    State::spell_power = kRankSpellPower[hero][0];
    State::followers_killed = 0;
    State::magic = hero == 2;
    State::siege = false;
    State::continent = 0;

    switch (hero) {
        case 0:
            State::army[0] = Militias;
            State::counts[0] = 20;
            State::army[1] = Archers;
            State::counts[1] = 2;
            break;
        case 1:
            State::army[0] = Peasants;
            State::counts[0] = 20;
            State::army[1] = Militias;
            State::counts[1] = 20;
            break;
        case 2:
            State::army[0] = Peasants;
            State::counts[0] = 30;
            State::army[1] = Sprites;
            State::counts[1] = 10;
            break;
        case 3:
            State::army[0] = Wolves;
            State::counts[0] = 20;
            break;
        default:
            break;
    }

    State::timestop = 0;

    genTiles();

    auto &hud {_engine.getGUI().getHUD()};
    hud.setColor(color);
    hud.setHudFrame();
    hud.setContract(State::contract);
    hud.setDays(State::days);
    hud.setMagic(State::magic);
    hud.setSiege(State::siege);
    hud.setHero(State::hero, State::rank);
    hud.setGold(State::gold);

    moveHeroTo(11, 58, 0);
}

void Ingame::render()
{
    GFX::instance().setView(_mapView);
    _map.draw(_mapView);
    drawMobs();
    if (State::boat_rented && _spHero.getMount() != Mount::Boat) {
        GFX::instance().drawSprite(_spBoat);
    }
    _spHero.draw();
}

void Ingame::renderLate()
{
    if (_engine.getGameOptions().debug) {
        GFX::instance().drawRect(_dbgCollisionRect);
        GFX::instance().drawText(_dbgTileText);
    }
}

void Ingame::enter()
{
    State::combat = false;
    auto color {bty::getBoxColor(State::difficulty)};
    _paused = false;
    _moveFlags = DIR_FLAG_NONE;
    auto &hud {_engine.getGUI().getHUD()};
    hud.setHudFrame();
    hud.setHero(State::hero, State::rank);
    hud.setDays(State::days);
    hud.setGold(State::gold);
    hud.setContract(State::contract);
    hud.setSiege(State::siege);
    hud.setPuzzle(State::villains_captured.data(), State::artifacts_found.data());
    hud.setColor(color);
    _dlgPause.setColor(color);
    _dlgJoin.setColor(color);
    _dlgSailMap.setColor(color);
    _dlgContMap.setColor(color);
    _dlgArtifact.setColor(color);
    _dlgSearch.setColor(color);
    _dlgSearchFail.setColor(color);
    _dlgBridge.setColor(color);
    _dlgTCGate.setColor(color);
    if (_tempPuzzleContinent != -1) {
        State::continent = _tempPuzzleContinent;
        _tempPuzzleContinent = -1;
        updateCamera();
    }
    _engine.getGUI().showHUD();
}

void Ingame::load()
{
    if (_loaded) {
        return;
    }

    auto &textures {Textures::instance()};

    for (int i = 0; i < UnitId::UnitCount; i++) {
        _texUnits[i] = textures.get(fmt::format("units/{}.png", i), {2, 2});
    }

    _map.load();
    _spHero.load();

    _uiView = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);
    _mapView = _uiView;

    _spBoat.setPosition(11 * 48.0f + 8.0f, 58 * 40.0f + 8.0f);
    _spBoat.setTexture(textures.get("hero/boat-stationary.png", {2, 1}));

    _dbgTileText.create(4, 7, "X -1\nY -1\nT -1");

    setup();
    _engine.getGUI().showHUD();

    static constexpr const char *const kPauseOptions[] = {
        "View your army",
        "View your character",
        "Look at continent map",
        "Use magic",
        "Contract information",
        "Wait to end of week",
        "Look at puzzle pieces",
        "Search the area",
        "Dismiss army",
        "Game controls",
        "Load game",
        "Save game",
    };

    _dlgPause.create(3, 7, 26, 16);
    for (int i = 0; i < sizeof(kPauseOptions) / sizeof(kPauseOptions[0]); i++) {
        _dlgPause.addOption(3, 2 + i, kPauseOptions[i]);
    }
    _dlgPause.bind(Key::Enter, std::bind(&Ingame::handlePauseOptions, this, std::placeholders::_1));
    _dlgPause.bind(Key::Backspace, [this](int) {
        _paused = false;
        _engine.getGUI().popDialog();
    });

    auto popDialog = [this](int) {
        _engine.getGUI().popDialog();
    };

    _dlgJoin.create(1, 18, 30, 9);
    _btJoinDescriptor = _dlgJoin.addString(1, 1);
    _dlgJoin.addString(3, 3, "with desires of greater\nglory, wish to join you.");
    _dlgJoin.addOption(13, 6, "Accept");
    _dlgJoin.addOption(13, 7, "Decline");

    _chestGenerator.addChest(_chestGold);
    _chestGenerator.addChest(_chestCommission);
    _chestGenerator.addChest(_chestSpellPower);
    _chestGenerator.addChest(_chestSpellCapacity);
    _chestGenerator.addChest(_chestSpell, true);

    _dlgSailMap.create(1, 18, 30, 9);
    static constexpr const char *const kSailMapMessage = {
        "  Hidden within an ancient\n"
        "  chest, you find maps and\n"
        "charts describing passage to\n"
        "\n",
    };
    _dlgSailMap.addString(1, 1, kSailMapMessage);
    _btSailMapDest = _dlgSailMap.addString(10, 6);
    _dlgSailMap.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
    });

    _dlgContMap.create(1, 18, 30, 9);
    static constexpr const char *const kContMapMessage = {
        "  Peering through a magical\n"
        "orb you are able to view the\n"
        " entire continent. Your map\n"
        "  of this area is complete.",
    };
    _dlgContMap.addString(1, 1, kContMapMessage);
    _dlgContMap.addString(10, 6);
    _dlgContMap.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
        SceneMan::instance().setScene("viewcontinent");
    });

    static constexpr const char *const kArtifactMessageCommon = {
        "   ...and a piece of the\n"
        " map to the stolen scepter.",
    };

    _dlgArtifact.create(1, 16, 30, 11);
    _dlgArtifact.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
    });
    _dlgArtifact.addString(1, 8, kArtifactMessageCommon);
    _btArtifactMessage = _dlgArtifact.addString(1, 1);

    static constexpr const char *const kSearchMessage = {
        "Search...\n"
        "\n"
        "It will take 10 days to do a\n"
        "search of this area, search?",
    };

    _dlgSearch.create(1, 18, 30, 9);
    _dlgSearch.addString(1, 1, kSearchMessage);
    _dlgSearch.addOption(14, 6, "No");
    _dlgSearch.addOption(14, 7, "Yes");
    _dlgSearch.bind(Key::Enter, [this](int opt) {
        if (opt == 1) {
            _engine.getGUI().popDialog();
            doSearch();
        }
    });

    _dlgSearchFail.create(1, 18, 30, 9);
    _dlgSearchFail.addString(1, 3, "Your search of this area has\n      revealed nothing.");
    _dlgSearchFail.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
        endWeek(true);
    });

    _dlgBridge.create(1, 20, 30, 7);
    _dlgBridge.addString(2, 1, "Bridge in which direction?");
    _dlgBridge.addString(14, 3, " \x81\n\x84 \x82\n \x83");
    _dlgBridge.bind(Key::Up, std::bind(&Ingame::bridgeDir, this, DIR_FLAG_UP));
    _dlgBridge.bind(Key::Down, std::bind(&Ingame::bridgeDir, this, DIR_FLAG_DOWN));
    _dlgBridge.bind(Key::Left, std::bind(&Ingame::bridgeDir, this, DIR_FLAG_LEFT));
    _dlgBridge.bind(Key::Right, std::bind(&Ingame::bridgeDir, this, DIR_FLAG_RIGHT));

    _useMagic = static_cast<UseMagic *>(SceneMan::instance().getScene("usemagic"));
    _useMagic->bindSpell(7, std::bind(&Ingame::spellBridge, this));
    _useMagic->bindSpell(8, std::bind(&Ingame::spellTimestop, this));
    _useMagic->bindSpell(9, std::bind(&Ingame::spellFindVillain, this));
    _useMagic->bindSpell(10, std::bind(&Ingame::spellTCGate, this, false));
    _useMagic->bindSpell(11, std::bind(&Ingame::spellTCGate, this, true));
    _useMagic->bindSpell(12, std::bind(&Ingame::spellInstantArmy, this));
    _useMagic->bindSpell(13, std::bind(&Ingame::spellRaiseControl, this));

    _dlgTCGate.create(1, 3, 30, 24);
    _dlgTCGate.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
    });
    _btTCGate0 = _dlgTCGate.addString(3, 3);
    _btTCGate1 = _dlgTCGate.addString(5, 20);

    _loaded = true;
}

void Ingame::handlePauseOptions(int opt)
{
    _engine.getGUI().popDialog();
    switch (opt) {
        case 0:
            for (int i = 0; i < 5; i++) {
                if (State::counts[i] * kUnits[State::army[i]].hp > State::leadership) {
                    State::morales[i] = 3;
                    continue;
                }
                State::morales[i] = bty::getUnitMorale(i, State::army.data());
            }
            SceneMan::instance().setScene("viewarmy");
            break;
        case 1:
            SceneMan::instance().setScene("viewchar");
            break;
        case 2:
            SceneMan::instance().setScene("viewcontinent");
            break;
        case 3:
            SceneMan::instance().setScene("usemagic");
            break;
        case 4:
            SceneMan::instance().setScene("viewcontract");
            break;
        case 5:
            endWeek(false);
            break;
        case 6:
            moveCameraToSceptre();
            SceneMan::instance().setScene("viewpuzzle");
            break;
        case 7:
            _engine.getGUI().pushDialog(_dlgSearch);
            break;
        case 8:
            dismiss();
            break;
        case 9:
            SceneMan::instance().setScene("controls");
            break;
        case 10:
            _engine.openSaveManager(true);
            break;
        case 11:
            _engine.openSaveManager(false);
            break;
        default:
            break;
    }
    _paused = false;
}

bool Ingame::handleEvent(Event event)
{
    switch (event.id) {
        case EventId::KeyUp:
            return handleKeyUp(event.key);
        case EventId::KeyDown:
            return handleKeyDown(event.key);
        default:
            return false;
    }
}

bool Ingame::handleKeyDown(Key key)
{
    switch (key) {
        case Key::Up:
            _moveFlags |= DIR_FLAG_UP;
            break;
        case Key::Down:
            _moveFlags |= DIR_FLAG_DOWN;
            break;
        case Key::Left:
            _moveFlags |= DIR_FLAG_LEFT;
            break;
        case Key::Right:
            _moveFlags |= DIR_FLAG_RIGHT;
            break;
        case Key::Space:
            pause();
            break;
        case Key::K:
            victory();
            break;
        case Key::R:
            genTiles();
            break;
        case Key::G:
            _spHero.setMount(_spHero.getMount() == Mount::Fly ? Mount::Walk : Mount::Fly);
            break;
        case Key::B:
            _spHero.setMount(_spHero.getMount() == Mount::Walk ? Mount::Boat : Mount::Walk);
            break;
        case Key::E:
            State::days = 1;
            _dayTimer.trigger();
            break;
        case Key::F:
            flyLand();
            break;
        case Key::J:
            sailTo(0);
            break;
        case Key::L:
            sailTo(3);
            break;
        default:
            return false;
    }
    return true;
}

bool Ingame::handleKeyUp(Key key)
{
    switch (key) {
        case Key::Up:
            _moveFlags &= ~(DIR_FLAG_UP);
            break;
        case Key::Down:
            _moveFlags &= ~(DIR_FLAG_DOWN);
            break;
        case Key::Left:
            _moveFlags &= ~(DIR_FLAG_LEFT);
            break;
        case Key::Right:
            _moveFlags &= ~(DIR_FLAG_RIGHT);
            break;
        default:
            return false;
    }
    return true;
}

void Ingame::update(float dt)
{
    if (State::auto_move) {
        automove(dt);
        _automoveTimer.tick(dt);
    }

    if (!State::auto_move) {
        moveHero(dt);
    }

    if (State::timestop) {
        _timestopTimer.tick(dt);
    }
    else if (!_paused && !_engine.getGUI().hasDialog() && !_engine.getGUI().getHUD().getError()) {
        _dayTimer.tick(dt);
        updateMobs(dt);
    }

    if (State::boat_rented) {
        _spBoat.update(dt);
    }

    updateAnimations(dt);
}

void Ingame::genTiles()
{
    for (int i = 0; i < 26; i++) {
        State::castle_occupants[i] = 0x7F;
        for (int j = 0; j < 5; j++) {
            State::castle_armies[i][j] = -1;
            State::castle_counts[i][j] = 0;
            State::garrison_armies[i][j] = -1;
            State::garrison_counts[i][j] = 0;
        }
        State::towns[i].id = i;
        State::towns[i].unit = -1;
        State::towns[i].spell = -1;
        State::towns[i].visited = false;
    }

    for (int i = 0; i < 17; i++) {
        State::villains_found[i] = false;
        State::villains_captured[i] = false;
    }

    for (int i = 0; i < 4; i++) {
        for (auto &mob : State::mobs[i]) {
            mob.dead = true;
        }
        for (auto &mob : State::friendly_mobs[i]) {
            mob = -1;
        }
    }

    _map.reset();

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
        7,
    };

    std::default_random_engine rng_ {};

    rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::shuffle(std::begin(artifacts), std::end(artifacts), rng_);

    int artifactIndex = 0;

    std::uniform_int_distribution<int> unitRng(0, UnitId::UnitCount - 1);
    std::uniform_int_distribution<int> spellRng(0, 13);

    /* Gen sceptre location */
    /* FIXME: I saw this seemingly generate on a sand tile. */
    do {
        _sceptreContinent = rand() % 4;
        _sceptreX = rand() % 64;
        _sceptreY = rand() % 64;
    } while (_map.getTiles(_sceptreContinent)[_sceptreY * 64 + _sceptreX] != Tile_Grass);

    for (int continent = 0; continent < 4; continent++) {
        auto *tiles = _map.getTiles(continent);

        std::vector<glm::ivec2> randomTiles;

        int numMobs = 0;

        int n = 0;
        for (int y = 0; y < 64; y++) {
            for (int x = 0; x < 64; x++) {
                int id = tiles[n];

                if (id == Tile_GenRandom) {
                    randomTiles.push_back({x, y});
                }
                else if (id == Tile_GenTown) {
                    int town = -1;
                    for (int i = 0; i < 26; i++) {
                        if (kTownInfo[i].continent == continent && kTownInfo[i].x == x && 63 - kTownInfo[i].y == y) {
                            town = i;
                        }
                    }
                    if (town != -1) {
                        State::towns[town].unit = unitRng(rng_);

                        /* Hunterville always has Bridge */
                        if (town == 21) {
                            State::towns[town].spell = 7;
                        }
                        else {
                            State::towns[town].spell = spellRng(rng_);
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
                    auto &mob = State::mobs[continent][numMobs++];
                    mob.tile = {x, y};
                    mob.dead = false;

                    genMobArmy(continent, mob.army, mob.counts);
                    bty::sortArmy(mob.army, mob.counts);

                    int highest = 0;
                    for (int i = 0; i < 5; i++) {
                        if (mob.army[i] == -1) {
                            continue;
                        }
                        if (kUnits[mob.army[i]].hp >= kUnits[mob.army[highest]].hp) {
                            highest = i;
                        }
                    }

                    mob.spriteId = mob.army[highest];

                    mob.entity.setTexture(_texUnits[mob.spriteId]);
                    mob.entity.moveToTile({x, y, Tile_Grass});

                    tiles[x + y * 64] = Tile_Grass;
                }
                n++;
            }
        }

        rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::shuffle(std::begin(randomTiles), std::end(randomTiles), rng_);

        unsigned int numUsedTiles = 0;
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
        tile = randomTiles[numUsedTiles++];
        tiles[tile.x + tile.y * 64] = Tile_ShopCave;
        State::teleport_cave_tiles[continent][0] = tile;

        tile = randomTiles[numUsedTiles++];
        tiles[tile.x + tile.y * 64] = Tile_ShopCave;
        State::teleport_cave_tiles[continent][1] = tile;

        /* 1 sail map */
        if (continent != 3) {
            /* Saharia doesn't have a map to anywhere */
            tile = randomTiles[numUsedTiles++];
            tiles[tile.x + tile.y * 64] = Tile_Chest;
            State::sail_map_tiles[continent] = tile;
        }

        /* 1 local map */
        tile = randomTiles[numUsedTiles++];
        tiles[tile.x + tile.y * 64] = Tile_Chest;
        State::continent_map_tiles[continent] = tile;

        /* 2 artifacts */
        tile = randomTiles[numUsedTiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifactIndex++]];

        tile = randomTiles[numUsedTiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifactIndex++]];

        /* Init 11 shops. */
        State::shops[continent].resize(11);
        for (int i = 0; i < 11; i++) {
            auto &shop = State::shops[continent][i];
            shop.count = 0;
            shop.unit = Peasants;
            shop.x = 0;
            shop.y = 0;
        }

        int num_shops = 0;

        /* Gen guaranteed peasant dwelling. */
        if (continent == 0) {
            tiles[27 + 52 * 64] = Tile_ShopWagon;

            auto &shop = State::shops[0][num_shops++];
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
            tile = randomTiles[numUsedTiles++];
            auto &shop = State::shops[continent][num_shops++];
            shop.x = tile.x;
            shop.y = tile.y;
            shop.unit = genShopUnit(i, continent);
            shop.count = genShopCount(shop.unit);
            shop.tileId = getShopTileType(shop.unit);
            tiles[tile.x + tile.y * 64] = shop.tileId;
        }

        /* Gen L high value shops */
        for (int i = 0; i < kNumHighValueShopsPerContinent[continent]; i++) {
            tile = randomTiles[numUsedTiles++];
            auto &shop = State::shops[continent][num_shops++];
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
            tile = randomTiles[numUsedTiles++];

            int id = genMobUnit(continent);
            int count = genMobCount(continent, id);

            auto &mob = State::mobs[continent][numMobs++];
            mob.tile = {tile.x, tile.y};
            mob.dead = false;
            mob.army[0] = id;
            mob.counts[0] = count;
            mob.entity.setTexture(_texUnits[id]);
            mob.entity.moveToTile({tile.x, tile.y, Tile_Grass});
            mob.spriteId = id;

            tiles[tile.x + tile.y * 64] = Tile_Grass;

            State::friendly_mobs[continent].push_back(mob.id);
        }

        /* Gen castle occupants */
        for (int i = 0; i < kVillainsPerContinent[continent]; i++) {
            int castle;
            do {
                castle = bty::random(26);
            } while (State::castle_occupants[castle] != 0x7F || kCastleInfo[castle].continent != continent);

            int villain = i + kVillainIndices[continent];

            State::castle_occupants[castle] = villain;
            genVillainArmy(villain, State::castle_armies[castle], State::castle_counts[castle]);
        }

        /* Turn the rest of the RNG tiles into chests */
        for (auto i = numUsedTiles; i < randomTiles.size(); i++) {
            tiles[randomTiles[i].x + randomTiles[i].y * 64] = Tile_Chest;
        }
    }

    for (unsigned int i = 0; i < 26; i++) {
        if (State::castle_occupants[i] != 0x7F) {
            continue;
        }
        genCastleArmy(kCastleInfo[i].continent, State::castle_armies[i], State::castle_counts[i]);
    }

    _engine.getGUI().getHUD().setPuzzle(State::villains_captured.data(), State::artifacts_found.data());

    _map.createGeometry();
}

void Ingame::updateCamera()
{
    glm::vec2 camCenter = _spHero.getCenter();
    glm::vec3 camPos = {camCenter.x - 130, camCenter.y - 120, 0.0f};
    _mapView = _uiView * glm::translate(-camPos);
}

void Ingame::updateVisitedTiles()
{
    const glm::ivec2 tile {State::x, State::y};
    auto *visited = State::visited_tiles[State::continent].data();
    auto *tiles = _map.getTiles(State::continent);

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

void Ingame::moveHeroTo(int x, int y, int c)
{
    this->_spHero.moveToTile(_map.getTile(x, y, c));
    updateCamera();

    State::x = x;
    State::y = y;
    State::continent = c;

    updateVisitedTiles();
}

void Ingame::useMagic()
{
}

void Ingame::bridgeDir(DirFlags dir)
{
    _engine.getGUI().popDialog();
    switch (dir) {
        case DIR_FLAG_UP:
            placeBridgeAt(State::x, State::y - 1, State::continent, false);
            break;
        case DIR_FLAG_DOWN:
            placeBridgeAt(State::x, State::y + 1, State::continent, false);
            break;
        case DIR_FLAG_LEFT:
            placeBridgeAt(State::x - 1, State::y, State::continent, true);
            break;
        case DIR_FLAG_RIGHT:
            placeBridgeAt(State::x + 1, State::y, State::continent, true);
            break;
        default:
            break;
    }
}

void Ingame::placeBridgeAt(int x, int y, int continent, bool horizontal)
{
    auto destination = _map.getTile(x, y, continent);

    if (destination.id == -1 || destination.id < Tile_WaterIRT || destination.id > Tile_Water) {
        _engine.getGUI().getHUD().setError("Not a suitable location for a bridge!", [this]() {
            _engine.getGUI().getHUD().setError("    What a waste of a good spell!");
        });
    }
    else {
        _map.setTile({x, y, -1}, continent, horizontal ? Tile_BridgeHorizontal : Tile_BridgeVertical);
    }
}

void Ingame::spellBridge()
{
    int c = State::continent;
    int x = State::x;
    int y = State::y;
    _engine.getGUI().pushDialog(_dlgBridge);
}

void Ingame::spellTimestop()
{
    _timestopTimer.reset();
    State::timestop += State::spell_power * 10;
    if (State::timestop > 9999) {
        State::timestop = 9999;
    }
    _engine.getGUI().getHUD().setTimestop(State::timestop);
}

void Ingame::spellFindVillain()
{
    if (State::contract < 17) {
        State::villains_found[State::contract] = true;
    }
    SceneMan::instance().setScene("viewcontract");
}

static void tc_gate_left(bty::Dialog &dialog);
static void tc_gate_right(bty::Dialog &dialog);

void Ingame::confirmTownGate(int opt)
{
    moveHeroTo(kTownGateX[kTownsAlphabetical[opt]], 63 - kTownGateY[kTownsAlphabetical[opt]], kTownInfo[opt].continent);
}

void Ingame::confirmCastleGate(int opt)
{
    moveHeroTo(kCastleInfo[opt].x, (63 - kCastleInfo[opt].y) + 1, kCastleInfo[opt].continent);
}

void Ingame::spellTCGate(bool town)
{
    bool none = true;
    for (int i = 0; i < 26; i++) {
        if ((town && State::towns[i].visited) || (!town && State::visited_castles[i])) {
            none = false;
            break;
        }
    }

    if (none) {
        _engine.getGUI().getHUD().setError(fmt::format("   You have not been to any {}", town ? "towns!" : "castles!"));
        return;
    }

    static constexpr const char *const kTCGateTown0 = {"Towns you have been to"};
    static constexpr const char *const kTCGateTown1 = {"Revisit which town?"};
    static constexpr const char *const kTCGateCastle0 = {"Castles you have been to"};
    static constexpr const char *const kTCGateCastle1 = {"Revisit which castle?"};

    _btTCGate0->setString(town ? kTCGateTown0 : kTCGateCastle0);
    _btTCGate1->setString(town ? kTCGateTown1 : kTCGateCastle1);

    if (town) {
        _dlgTCGate.bind(Key::Enter, std::bind(&Ingame::confirmTownGate, this, std::placeholders::_1));
    }
    else {
        _dlgTCGate.bind(Key::Enter, std::bind(&Ingame::confirmCastleGate, this, std::placeholders::_1));
    }

    _dlgTCGate.clearOptions();

    int index = 0;
    for (int x = 0; x < 2; x++) {
        for (int y = 0; y < 13; y++) {
            int xCoord = 3 + x * 15;
            int yCoord = 6 + y;
            if (town && State::towns[index].visited) {
                _dlgTCGate.addOption(xCoord, yCoord, kTownInfo[index].name);
            }
            else if (State::visited_castles[index]) {
                _dlgTCGate.addOption(xCoord, yCoord, kCastleInfo[index].name);
            }
            index++;
        }
    }

    _engine.getGUI().pushDialog(_dlgTCGate);
}

void tc_gate_left(bty::Dialog &dialog)
{
    int selection = dialog.getSelection();
    selection = (selection - 13 + 26) % 26;
    int nearestIndex = 0;
    int nearestDistance = 26;
    auto &opt = dialog.get_options();
    for (int i = 0; i < 13; i++) {
        int test = selection > 12 ? (13 + i) : (i);
        int distance = std::abs(selection - test);
        if (distance < nearestDistance) {
            if (opt[i].visible()) {
                nearestIndex = test;
                nearestDistance = distance;
            }
        }
    }
    dialog.setSelection(nearestIndex);
}

void tc_gate_right(bty::Dialog &dialog)
{
    int selection = dialog.getSelection();
    selection = (selection + 13) % 26;
    int nearestIndex = 0;
    int nearestDistance = 26;
    auto &opt = dialog.get_options();
    for (int i = 0; i < 13; i++) {
        int test = selection > 12 ? (13 + i) : (i);
        int distance = std::abs(selection - test);
        if (distance < nearestDistance) {
            if (opt[i].visible()) {
                nearestIndex = test;
                nearestDistance = distance;
            }
        }
    }
    dialog.setSelection(nearestIndex);
}

bool Ingame::addUnitToArmy(int id, int count)
{
    if (id < 0 || id >= UnitId::UnitCount) {
        spdlog::warn("addUnitToArmy: id out of range: {}", id);
        return false;
    }

    for (int i = 0; i < 5; i++) {
        if (State::army[i] == id) {
            State::counts[i] += count;
            return true;
        }
    }

    int armySize = 0;

    for (int i = 0; i < 5; i++) {
        if (State::army[i] != -1) {
            armySize++;
        }
    }

    if (armySize == 5) {
        spdlog::warn("addUnitToArmy: army already full");
        return false;
    }

    int index = armySize++;

    State::army[index] = id;
    State::counts[index] = count;

    return true;
}

void Ingame::spellInstantArmy()
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

    static constexpr const char *const kJoinMessage = {
        "{} {}\n"
        "\n"
        "  have joined your army.",
    };

    int unit = kInstantArmyUnits[State::rank][State::hero];
    int amt = State::spell_power * 3 + (rand() % State::spell_power + 2);

    if (!addUnitToArmy(unit, amt)) {
        _engine.getGUI().getHUD().setError(" You do not have any more army slots!");
    }
    else {
        _engine.getGUI().showMessage(1, 21, 30, 6, fmt::format(kJoinMessage, bty::unitDescriptor(amt), kUnits[unit].namePlural));
    }
}

void Ingame::spellRaiseControl()
{
    State::leadership += State::spell_power * 100;
    State::leadership &= 0xFFFF;
}

void Ingame::endWeekAstrology(bool search)
{
    int unit = rand() % UnitId::UnitCount;
    const auto &name = kUnits[unit].nameSingle;

    static constexpr const char *const kAstrologyMessage = {
        "Week #{}\n"
        "\n"
        "Astrologers proclaim:\n"
        "Week of the {}\n"
        "\n"
        "All {} dwellings are \n"
        "repopulated.",
    };

    _engine.getGUI().showMessage(1, 18, 30, 9, fmt::format(kAstrologyMessage, State::weeks_passed, name, name), [this, search]() {
        endWeekBudget(search);
    });

    State::leadership = State::permanent_leadership;

    for (int i = 0; i < 26; i++) {
        if (State::castle_occupants[i] == -1) {
            if (State::garrison_armies[i][0] == -1) {
                genCastleArmy(kCastleInfo[i].continent, State::castle_armies[i], State::castle_counts[i]);
                State::castle_occupants[i] = 0x7F;
            }
        }
    }
}

void Ingame::endWeekBudget(bool search)
{
    int armyTotalCost = 0;
    int commission = State::commission;

    for (int i = 0; i < 26; i++) {
        if (State::castle_occupants[i] == -1) {
            commission += 250;
        }
    }

    for (int i = 0; i < 26; i++) {
        if (State::castle_occupants[i] == -1) {
            State::gold += 250;
        }
    }

    int gold = State::gold;
    int boat = State::boat_rented ? (State::artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500) : 0;
    int balance = (commission + gold) - boat;

    std::string armyInfo;

    for (int i = 0; i < 5; i++) {
        if (State::army[i] == -1) {
            continue;
        }

        const auto &unit = kUnits[State::army[i]];

        int weeklyCost = unit.weeklyCost * State::counts[i];
        if (balance > weeklyCost || balance - weeklyCost == 0) {
            balance -= weeklyCost;
            armyTotalCost += weeklyCost;

            std::string cost = bty::numberK(weeklyCost);
            std::string spaces(14 - (cost.size() + unit.namePlural.size()), ' ');
            armyInfo += fmt::format("{}{}{}\n", unit.namePlural, spaces, cost);
        }
        else {
            State::army[i] = -1;
            State::counts[i] = -1;

            std::string spaces(14 - (5 + unit.namePlural.size()), ' ');
            armyInfo += fmt::format("{}{}Leave\n", unit.namePlural, spaces);
        }
    }

    bty::sortArmy(State::army, State::counts);

    bool outOfGold = (boat + armyTotalCost) > (gold + commission);
    if (!outOfGold) {
        balance = (commission + gold) - (boat + armyTotalCost);
    }

    static constexpr const char *const kBudgetMessage = {
        "Week #{}               Budget\n"
        "\n"
        "On Hand: {}\n"
        "Payment: {:>4}\n"
        "Boat: {:>7}\n"
        "Army: {:>7}\n"
        "Balance: {:>4}",
    };

    State::gold = balance;

    auto &dialog = _engine.getGUI().showMessage(1, 18, 30, 9, fmt::format(kBudgetMessage, State::weeks_passed, bty::numberK(State::gold), commission, boat, armyTotalCost, bty::numberK(balance)));
    dialog.addString(15, 3, armyInfo);

    if (State::army[0] == -1 || outOfGold) {
        disgrace();
    }
}

void Ingame::disgrace()
{
    moveHeroTo(11, 58, 0);
    _spHero.setMount(Mount::Walk);
    _spHero.setFlip(false);
    _spHero.setMoving(false);

    State::army[0] = Peasants;
    State::army[1] = -1;
    State::army[2] = -1;
    State::army[3] = -1;
    State::army[4] = -1;
    State::counts[0] = 20;
    State::counts[1] = 0;
    State::counts[2] = 0;
    State::counts[3] = 0;
    State::counts[4] = 0;

    static constexpr const char *const kDisgraceMessage = {
        "After being disgraced on the\n"
        "    field of battle, King\n"
        " Maximus summons you to his\n"
        "  castle. After a lesson in\n"
        "   tactics, he reluctantly\n"
        "reissues your commission and\n"
        "   sends you on your way.",
    };

    _engine.getGUI().showMessage(1, 18, 30, 9, kDisgraceMessage);
}

void Ingame::defeat()
{
    _engine.getGUI().getHUD().setDays(0);
    SceneMan::instance().setScene("defeat");
}

void Ingame::victory()
{
    _engine.getGUI().getHUD().setBlankFrame();
    SceneMan::instance().setScene("victory");
}

void Ingame::dismiss()
{
    auto dialog = _engine.getGUI().makeDialog(1, 18, 30, 9);

    for (int i = 0; i < 5; i++) {
        if (State::army[i] != -1) {
            dialog->addOption(11, 3 + i, kUnits[State::army[i]].namePlural);
        }
    }

    dialog->addString(5, 1, "Dismiss which army?");
    dialog->bind(Key::Enter, [this, dialog](int opt) {
        _engine.getGUI().popDialog();
        dismissSlot(opt);
        dismiss();
    });
}

void Ingame::dismissSlot(int slot)
{
    int armySize = 0;
    for (int i = 0; i < 5; i++) {
        if (State::army[i] != -1) {
            armySize++;
        }
    }

    if (armySize == 1) {
        _engine.getGUI().getHUD().setError("  You may not dismiss your last army!");
        return;
    }

    State::army[slot] = -1;
    State::counts[slot] = 0;
    bty::sortArmy(State::army, State::counts);
}

bool Ingame::heroCanMove(int id)
{
    return _spHero.canMove(id, 0, 0, 0);
}

bool Ingame::mobCanMove(int id)
{
    return id == Tile_Grass;
}

bool Ingame::moveIncrement(c2AABB &box, float dx, float dy, Tile &centerTile, Tile &collidedTile, bool (Ingame::*canMove)(int), bool mob)
{
    box.min.x += dx;
    box.max.x += dx;
    box.min.y += dy;
    box.max.y += dy;

    /* Test. */
    centerTile = _map.getTile(box.min.x + 4, box.min.y + 4, State::continent);

    if (!mob) {
        /* Tile center is the same as the last event tile we collided with.
		If we're not overlapping any other tiles, don't collide. */
        if (centerTile.tx == _dbgLastEventTile.tx && centerTile.ty == _dbgLastEventTile.ty) {
            return false;
        }
        /* Don't endlessly loop between the two teleport caves just because
		they are technically different tiles. */
        else if (_dbgLastEventTile.id == Tile_ShopCave) {
            for (int i = 0; i < 2; i++) {
                if (_dbgLastEventTile.tx == State::teleport_cave_tiles[State::continent][i].x && _dbgLastEventTile.ty == State::teleport_cave_tiles[State::continent][i].y) {
                    return false;
                }
            }
        }
    }

    /* Collided; undo move and register it. */
    if (!(this->*canMove)(centerTile.id)) {
        box.min.x -= dx;
        box.max.x -= dx;
        box.min.y -= dy;
        box.max.y -= dy;
        collidedTile = centerTile;
        centerTile = _map.getTile(box.min.x + 4, box.min.y + 4, State::continent);
        return true;
    }
    /* Didn't collide; confirm move. */
    else {
        return false;
    }
}

void Ingame::moveMob(Mob &mob, float dt, const glm::vec2 &dir)
{
    float speed = 70.0f;
    float vel = speed * dt;
    float dx = dir.x * vel;
    float dy = dir.y * vel;
    auto lastPos = mob.entity.getPosition();

    /* Create shape. */
    auto aabb = mob.entity.getAABB();

    _dbgLastTile = _map.getTile(aabb.min.x + 4, aabb.min.y + 4, State::continent);

    Tile centerTile {-1, -1, -1};
    Tile collidedTile {-1, -1, -1};

    bool collidedX = moveIncrement(aabb, dx, 0, centerTile, collidedTile, &Ingame::mobCanMove, true);
    bool collidedY = moveIncrement(aabb, 0, dy, centerTile, collidedTile, &Ingame::mobCanMove, true);

    if (centerTile.tx != mob.tile.x || centerTile.ty != mob.tile.y) {
        mob.tile = {centerTile.tx, centerTile.ty};
    }

    mob.entity.setPosition(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);
}

void Ingame::moveHero(float dt)
{
    if (_engine.getGameOptions().debug) {
        _dbgCollisionRect.setColor({0.0f, 0.0f, 0.7f, 0.9f});
    }

    if (_moveFlags == DIR_FLAG_NONE) {
        _spHero.setMoving(false);
        return;
    }

    _spHero.setMoving(true);

    if ((_moveFlags & DIR_FLAG_LEFT) && !(_moveFlags & DIR_FLAG_RIGHT)) {
        _spHero.setFlip(true);
    }
    else if ((_moveFlags & DIR_FLAG_RIGHT) && !(_moveFlags & DIR_FLAG_LEFT)) {
        _spHero.setFlip(false);
    }

    glm::vec2 dir {0.0f};

    if (_moveFlags & DIR_FLAG_UP) {
        dir.y -= 1.0f;
    }
    if (_moveFlags & DIR_FLAG_DOWN) {
        dir.y += 1.0f;
    }
    if (_moveFlags & DIR_FLAG_LEFT) {
        dir.x -= 1.0f;
    }
    if (_moveFlags & DIR_FLAG_RIGHT) {
        dir.x += 1.0f;
    }

    auto mount = _spHero.getMount();
    float speed = mount == Mount::Fly ? 300.0f : 120.0f;
    float vel = speed * dt * _spHero.getSpeedMul();
    float dx = dir.x * vel;
    float dy = dir.y * vel;

    /* Create shape. */
    auto aabb = _spHero.getAABB();
    auto lastPos = _spHero.getPosition();
    _dbgLastTile = _map.getTile(aabb.min.x + 4, aabb.min.y + 4, State::continent);

    Tile centerTile {-1, -1, -1};
    Tile collidedTile {-1, -1, -1};

    bool collidedX = moveIncrement(aabb, dx, 0, centerTile, collidedTile, &Ingame::heroCanMove, false);
    bool collidedY = moveIncrement(aabb, 0, dy, centerTile, collidedTile, &Ingame::heroCanMove, false);

    bool teleport {false};

    /* Deal with the consequences of the collision. */
    if (collidedX || collidedY) {
        /* Try to collide with it, and see if it's an event tile. */
        if (_spHero.getMount() == Mount::Walk && events(collidedTile, teleport)) {
            _dbgLastEventTile = collidedTile;

            if (!teleport && collidedTile.id != Tile_CastleB) {
                /* Move into it. */
                aabb.min.x += dx;
                aabb.max.x += dx;
                aabb.min.y += dy;
                aabb.max.y += dy;
            }
            else if (collidedTile.id == Tile_CastleB) {
                /* Move /away/ from it. */
                aabb.min.y -= dy * 3;
                aabb.max.y -= dy * 3;
            }
            centerTile = _map.getTile(aabb.min.x + 4, aabb.min.y + 4, State::continent);
        }
        /* Walked into the boat tile. */
        else if (collidedTile.tx == State::boat_x && collidedTile.ty == State::boat_y && State::continent == State::boat_c) {
            _spHero.setMount(Mount::Boat);
            /* Move into it. */
            aabb.min.x += dx;
            aabb.max.x += dx;
            aabb.min.y += dy;
            aabb.max.y += dy;
            centerTile = _map.getTile(aabb.min.x + 4, aabb.min.y + 4, State::continent);
        }
        /* Dismount. */
        else if (collidedTile.id == Tile_Grass || bty::is_event_tile(collidedTile.id) && _spHero.getMount() == Mount::Boat) {
            _spHero.setMount(Mount::Walk);
            /* Move into it. */
            aabb.min.x += dx;
            aabb.max.x += dx;
            aabb.min.y += dy;
            aabb.max.y += dy;
            centerTile = _map.getTile(aabb.min.x + 4, aabb.min.y + 4, State::continent);

            State::boat_x = _dbgLastTile.tx;
            State::boat_y = _dbgLastTile.ty;
            State::boat_c = State::continent;

            _spBoat.setFlip(_spHero.getFlip());

            auto hp = glm::vec2(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);
            auto bp = lastPos;

            glm::vec4 a {hp.x, hp.y, 0.0f, 1.0f};
            glm::vec4 b {bp.x, bp.y, 0.0f, 1.0f};

            auto push_away_from_land_dir = glm::normalize(a - b);
            push_away_from_land_dir.x *= -8.0f;
            push_away_from_land_dir.y *= -8.0f;
            _spBoat.setPosition(lastPos + glm::vec2 {push_away_from_land_dir.x, push_away_from_land_dir.y});
        }
    }
    /* Not colliding; if the tile is different to the previous one, update it and
		forget about the last event tile meaning we can once again collide with it. */
    else if (centerTile.tx != _dbgLastTile.tx || centerTile.ty != _dbgLastTile.ty) {
        if (_spHero.getMount() == Mount::Walk && centerTile.id >= Tile_SandELT && centerTile.id <= Tile_Sand) {
            _spHero.setSpeedMul(0.6f);
        }
        else {
            _spHero.setSpeedMul(1.0f);
        }
        _dbgLastTile = centerTile;
        _dbgLastEventTile = {-1, -1, -1};
    }

    State::x = centerTile.tx;
    State::y = centerTile.ty;

    updateVisitedTiles();

    if (!teleport) {
        _spHero.setPosition(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);
    }

    if (_engine.getGameOptions().debug) {
        _dbgCollisionRect.setPosition(aabb.min.x, aabb.min.y);
        if (collidedX && collidedY) {
            _dbgCollisionRect.setColor({0.75f, 0.95f, 0.73f, 0.9f});
        }
        else if (collidedX) {
            _dbgCollisionRect.setColor({0.7f, 0.0f, 0.0f, 0.9f});
        }
        else if (collidedY) {
            _dbgCollisionRect.setColor({0.75f, 0.35f, 0.73f, 0.9f});
        }
    }

    if (_engine.getGameOptions().debug) {
        _dbgTileText.setString(fmt::format("X {}\nY {}\nT {}{}{}\nLast tile {} {} {}", State::x, State::y, _map.getTile(State::x, State::y, State::continent).id, collidedX || collidedY ? "\nC " : "", collidedX || collidedY ? std::to_string(collidedTile.id) : "", _dbgLastTile.tx, _dbgLastTile.ty, _dbgLastTile.id));
    }

    updateCamera();
}

void Ingame::sailTo(int continent)
{
    State::auto_move = true;

    if (continent == State::continent) {
        auto pos = _spHero.getCenter();

        State::auto_move_dir = {0, 0};
        _spHero.setMoving(true);

        if (pos.x < 0) {
            State::auto_move_dir.x = 1;
            _spHero.setFlip(false);
        }
        else if (pos.x > 48 * 64) {
            State::auto_move_dir.x = -1;
            _spHero.setFlip(true);
        }

        if (pos.y < 0) {
            State::auto_move_dir.y = 1;
        }
        else if (pos.y > 40 * 64) {
            State::auto_move_dir.y = -1;
        }
    }
    else {
        _spHero.setMoving(true);
        _spHero.setMoving(true);
        State::auto_move_dir = {0, 0};

        switch (continent) {
            case 0:
                // Up
                _automoveTimer.setDuration(1.0f);
                State::auto_move_dir.y = -1;
                _spHero.setFlip(false);
                _spHero.moveToTile(_map.getTile(11, 64 - 1, State::continent));
                break;
            case 1:
                // Right
                _automoveTimer.setDuration(0.4f);
                State::auto_move_dir.x = 1;
                _spHero.setFlip(false);
                _spHero.moveToTile(_map.getTile(0, 64 - 25, State::continent));
                break;
            case 2:
                // Down
                _automoveTimer.setDuration(0.6f);
                State::auto_move_dir.y = 1;
                _spHero.setFlip(false);
                _spHero.moveToTile(_map.getTile(14, 0, State::continent));
                break;
            case 3:
                // Up
                _automoveTimer.setDuration(1);
                State::auto_move_dir.y = -1;
                _spHero.setFlip(false);
                _spHero.moveToTile(_map.getTile(9, 64 - 1, State::continent));
                break;
            default:
                break;
        }

        _spHero.setMount(Mount::Boat);
        updateCamera();
        State::continent = continent;
        _map.setContinent(continent);
    }
}

void Ingame::drawMobs()
{
    for (auto *mob : getMobsInRange(State::x, State::y, 4)) {
        if (mob->dead) {
            continue;
        }
        mob->entity.draw();
    }
}

void Ingame::collideTeleportCave(const Tile &tile)
{
    auto dest = glm::ivec2 {tile.tx, tile.ty} == State::teleport_cave_tiles[State::continent][0] ? State::teleport_cave_tiles[State::continent][1] : State::teleport_cave_tiles[State::continent][0];
    moveHeroTo(dest.x, dest.y, State::continent);
}

void Ingame::updateMobs(float dt)
{
    const auto &heroPos = _spHero.getPosition();

    int continent = State::continent;

    for (auto *mob : getMobsInRange(State::x, State::y, 4)) {
        if (mob->dead) {
            continue;
        }

        const auto mob_pos = mob->entity.getPosition();

        float distanceX = std::abs(heroPos.x - mob_pos.x);
        float distanceY = std::abs(heroPos.y - mob_pos.y);

        glm::vec2 dir {0.0f, 0.0f};

        if (distanceX > 3.0f) {
            dir.x = heroPos.x > mob_pos.x ? 1.0f : -1.0f;
        }

        if (distanceY > 3.0f) {
            dir.y = heroPos.y > mob_pos.y ? 1.0f : -1.0f;
        }

        if (_spHero.getMount() == Mount::Walk && distanceX < 12.0f && distanceY < 12.0f) {
            for (auto j = 0u; j < State::friendly_mobs[State::continent].size(); j++) {
                if (State::friendly_mobs[State::continent][j] == mob->id) {
                    tryJoin(mob->army[0], mob->counts[0], [this, mob]() {
                        mob->dead = true;
                    });
                    return;
                }
            }

            _engine.startEncounterBattle(mob->id);
            return;
        }

        mob->entity.setFlip(heroPos.x < mob_pos.x);

        moveMob(*mob, dt, dir);
    }
}

void Ingame::collideSign(const Tile &tile)
{
    int index = 0;
    auto *tiles = _map.getTiles(State::continent);
    for (int y = 63; y >= 0; y--) {
        for (int x = 0; x < 64; x++) {
            if (tiles[y * 64 + x] == Tile_GenSign) {
                if (x == tile.tx && y == tile.ty) {
                    _engine.getGUI().showMessage(1, 21, 30, 6, fmt::format("A sign reads\n\n{}", kSigns[index + State::continent * 22]));
                    goto foundSign;
                }
                index++;
            }
        }
    }
foundSign:;
}

bool Ingame::events(const Tile &tile, bool &teleport)
{
    bool eventTile {true};

    switch (tile.id) {
        case Tile_Sign:
            [[fallthrough]];
        case Tile_GenSign:
            collideSign(tile);
            break;
        case Tile_Chest:
            collideChest(tile);
            break;
        case Tile_Town:
            collideTown(tile);
            break;
        case Tile_CastleB:
            if (tile.tx == 11 && tile.ty == 56) {
                SceneMan::instance().setScene("kingscastle");
            }
            else {
                collideCastle(tile);
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
            collideArtifact(tile);
            break;
        case Tile_GenWizardCave:
            SceneMan::instance().setScene("wizard");
            break;
        case Tile_ShopCave:
            if (glm::ivec2 {tile.tx, tile.ty} == State::teleport_cave_tiles[State::continent][0] || glm::ivec2 {tile.tx, tile.ty} == State::teleport_cave_tiles[State::continent][1]) {
                collideTeleportCave(tile);
                teleport = true;
            }
            else {
                collideShop(tile);
            }
            break;
        case Tile_ShopTree:
            [[fallthrough]];
        case Tile_ShopDungeon:
            [[fallthrough]];
        case Tile_ShopWagon:
            collideShop(tile);
            break;
        default:
            eventTile = false;
            break;
    }

    if (eventTile) {
        _moveFlags = DIR_FLAG_NONE;
    }

    return eventTile;
}

void Ingame::collideShop(const Tile &tile)
{
    int shop_id = -1;
    for (auto i = 0u; i < State::shops[State::continent].size(); i++) {
        if (State::shops[State::continent][i].x == tile.tx && State::shops[State::continent][i].y == tile.ty) {
            shop_id = i;
            break;
        }
    }
    if (shop_id == -1) {
        spdlog::warn("Failed to find shop at [{}] {} {}", State::continent, tile.tx, tile.ty);
    }
    else {
        _engine.openShop(State::shops[State::continent][shop_id]);
    }
}

void Ingame::collideChest(const Tile &tile)
{
    _map.setTile(tile, State::continent, Tile_Grass);

    if (State::continent < 3 && tile.tx == State::sail_map_tiles[State::continent].x && tile.ty == State::sail_map_tiles[State::continent].y) {
        State::sail_maps_found[State::continent + 1] = true;
        _btSailMapDest->setString(kContinentNames[State::continent + 1]);
        _engine.getGUI().pushDialog(_dlgSailMap);
    }
    else if (tile.tx == State::continent_map_tiles[State::continent].x && tile.ty == State::continent_map_tiles[State::continent].y) {
        State::continent_maps_found[State::continent] = true;
        _engine.getGUI().pushDialog(_dlgContMap);
    }
    else {
        _chestGenerator.roll();
    }
}

void Ingame::collideCastle(const Tile &tile)
{
    int castleId = -1;

    for (int i = 0; i < 26; i++) {
        if (kCastleInfo[i].x == tile.tx && kCastleInfo[i].y == 63 - tile.ty && kCastleInfo[i].continent == State::continent) {
            castleId = i;
        }
    }

    if (castleId == -1) {
        spdlog::warn("Failed to find castle at [{},{}] in {}", tile.tx, tile.ty, kContinentNames[State::continent]);
        return;
    }

    int occupier = State::castle_occupants[castleId];

    if (occupier == -1) {
        _engine.openGarrison(castleId);
        return;
    }

    std::string occName = occupier == 0x7F ? "Various groups of monsters" : fmt::format("{} and", kVillains[occupier][0]);
    std::string lineTwo = occupier == 0x7F ? "occupy this castle" : "army occupy this castle";

    auto dialog = _engine.getGUI().makeDialog(1, 18, 30, 9);
    dialog->addString(1, 1, fmt::format("Castle {}", kCastleInfo[castleId].name));
    dialog->addString(1, 3, occName);
    dialog->addString(2, 4, lineTwo);
    dialog->addOption(11, 6, "Lay siege.");
    dialog->addOption(11, 7, "Venture on.");
    dialog->onKey([this, dialog, castleId](Key key) {
        switch (key) {
            case Key::Backspace:
                _engine.getGUI().popDialog();
                break;
            case Key::Enter:
                _engine.getGUI().popDialog();
                if (dialog->getSelection() == 0) {
                    _engine.startSiegeBattle(castleId);
                }
                break;
            default:
                return false;
        }
        return true;
    });

    State::visited_castles[castleId] = true;
}

void Ingame::collideTown(const Tile &tile)
{
    int townId = -1;

    for (int i = 0; i < 26; i++) {
        if (kTownInfo[i].continent == State::continent && kTownInfo[i].x == tile.tx && 63 - kTownInfo[i].y == tile.ty) {
            townId = i;
            break;
        }
    }

    if (townId == -1) {
        spdlog::warn("Couldn't find town at {}, {}", tile.tx, tile.ty);
        return;
    }

    State::towns[townId].visited = true;
    _engine.openTown(&State::towns[townId]);
}

void Ingame::collideArtifact(const Tile &tile)
{
    _map.setTile(tile, State::continent, Tile_Grass);

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

    State::artifacts_found[artifact] = true;

    _engine.getGUI().getHUD().setPuzzle(State::villains_captured.data(), State::artifacts_found.data());

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

    _btArtifactMessage->setString(kArtifactMessages[artifact]);
    _engine.getGUI().pushDialog(_dlgArtifact);
}

void Ingame::acceptWizardOffer()
{
    _map.setTile({11, 63 - 19, -1}, 0, Tile_Grass);
}

void Ingame::doSearch()
{
    if (_sceptreContinent != State::continent) {
        _engine.getGUI().pushDialog(_dlgSearchFail);
        return;
    }

    int range = 3;
    int offset = range / 2;

    auto tile = _spHero.getTile();
    bool found = false;
    for (int i = 0; i <= range; i++) {
        for (int j = 0; j <= range; j++) {
            int x = tile.tx - offset + i;
            int y = tile.ty - offset + j;

            if (x < 0 || x > 63 || y < 0 || y > 63) {
                continue;
            }

            if (x == _sceptreX && y == _sceptreY) {
                found = true;
                break;
            }
        }
    }

    if (!found) {
        _engine.getGUI().pushDialog(_dlgSearchFail);
    }
    else {
        victory();
    }
}

void Ingame::endWeek(bool search)
{
    if ((State::days % 5) != 0) {
        State::days = (State::days / 5) * 5;
    }
    else {
        State::days = ((State::days / 5) - 1) * 5;
    }
    if (State::days == 0) {
        defeat();
    }
    else {
        State::weeks_passed++;
        endWeekAstrology(search);
        _engine.getGUI().getHUD().setGold(State::gold);
    }
    _dayTimer.reset();
    _engine.getGUI().getHUD().setDays(State::days);
}

void Ingame::flyLand()
{
    auto mount = _spHero.getMount();

    if (mount == Mount::Fly) {
        c2AABB ent_shape = _spHero.getAABB();

        int range = 4;
        int offset = range / 2;

        /* No need to check surrounding tiles if we can't land on the current tile anyway. */
        if (_map.getTile(_spHero.getCenter(), State::continent).id != Tile_Grass) {
            goto endLoop;
        }

        /* Make sure we're not intersecting any collidable tiles when we land. */
        for (int i = 0; i <= range; i++) {
            for (int j = 0; j <= range; j++) {
                int x = State::x - offset + i;
                int y = State::y - offset + j;

                if (x == State::x && y == State::y) {
                    continue;
                }

                if (x < 0 || x > 63 || y < 0 || y > 63) {
                    continue;
                }

                c2AABB tileShape;
                tileShape.min.x = 48.0f * x;
                tileShape.min.y = 40.0f * y;
                tileShape.max.x = tileShape.min.x + 48.0f;
                tileShape.max.y = tileShape.min.y + 40.0f;

                if (_map.getTile(ent_shape.min.x, ent_shape.min.y, State::continent).id != Tile_Grass && !c2AABBtoAABB(ent_shape, tileShape)) {
                    goto endLoop;
                }
            }
        }
        _spHero.setMount(Mount::Walk);
    endLoop:;
    }
    else if (mount == Mount::Walk) {
        bool canFly = true;
        for (int i = 0; i < 5; i++) {
            if (State::army[i] == -1) {
                continue;
            }
            if (!(kUnits[State::army[i]].abilities & AbilityFly)) {
                canFly = false;
                break;
            }
        }
        if (canFly) {
            _spHero.setMount(Mount::Fly);
        }
    }
}

void Ingame::dayTick()
{
    State::days--;
    if (State::days == 0) {
        defeat();
    }
    else {
        State::days_passed_this_week++;
        if (State::days_passed_this_week == 5) {
            State::days_passed_this_week = 0;
            State::weeks_passed++;
            endWeekAstrology(false);
            _engine.getGUI().getHUD().setGold(State::gold);
        }
        _engine.getGUI().getHUD().setDays(State::days);
    }
}

void Ingame::timestopTick()
{
    State::timestop--;

    if (State::timestop == 0) {
        _engine.getGUI().getHUD().clearTimestop();
    }
    else {
        _engine.getGUI().getHUD().setTimestop(State::timestop);
    }
}

std::vector<Mob *> Ingame::getMobsInRange(int x, int y, int range)
{
    std::vector<Mob *> moblist;

    for (auto &mob : State::mobs[State::continent]) {
        if (mob.dead) {
            continue;
        }
        if (std::abs(x - mob.tile.x) <= range && std::abs(y - mob.tile.y) <= range) {
            moblist.push_back(&mob);
        }
    }

    return moblist;
}

void Ingame::automoveTick()
{
    State::auto_move = false;
}

void Ingame::automove(float dt)
{
    float vel = 120.0f * dt;
    float dx = State::auto_move_dir.x * vel;
    float dy = State::auto_move_dir.y * vel;

    auto aabb = _spHero.getAABB();
    auto lastPos = _spHero.getPosition();
    _dbgLastTile = _map.getTile(aabb.min.x + 4, aabb.min.y + 4, State::continent);

    Tile centerTile {-1, -1, -1};
    Tile collidedTile {-1, -1, -1};

    moveIncrement(aabb, dx, 0, centerTile, collidedTile, &Ingame::heroCanMove, false);
    moveIncrement(aabb, 0, dy, centerTile, collidedTile, &Ingame::heroCanMove, false);

    _spHero.setPosition(aabb.min.x - kEntityOffsetX, aabb.min.y - kEntityOffsetY);

    if (centerTile.tx != _dbgLastTile.tx || centerTile.ty != _dbgLastTile.ty) {
        _dbgLastTile = centerTile;
        _dbgLastEventTile = {-1, -1, -1};
    }

    updateCamera();
}

void Ingame::saveState(std::ofstream &f)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 40; j++) {
            f.write((char *)State::mobs[i][j].army.data(), 5 * 4);
            f.write((char *)State::mobs[i][j].counts.data(), 5 * 4);
            int dead = State::mobs[i][j].dead;
            f.write((char *)&dead, 4);
            f.write((char *)&State::mobs[i][j].id, 4);
            f.write((char *)&State::mobs[i][j].spriteId, 4);
            f.write((char *)&State::mobs[i][j].tile, sizeof(Tile));
            auto pos = State::mobs[i][j].entity.getPosition();
            f.write((char *)&pos, 8);
        }
    }

    f.write((char *)&State::hero, 4);
    f.write((char *)&State::rank, 4);
    f.write((char *)&State::days, 4);
    f.write((char *)&State::gold, 4);
    f.write((char *)&State::difficulty, 4);
    f.write((char *)&State::magic, 4);
    f.write((char *)&State::siege, 4);
    f.write((char *)&State::contract, 4);
    f.write((char *)&State::x, 4);
    f.write((char *)&State::y, 4);
    f.write((char *)&State::continent, 4);
    f.write((char *)&State::leadership, 4);
    f.write((char *)&State::commission, 4);
    f.write((char *)State::army.data(), 4 * 5);
    f.write((char *)State::counts.data(), 4 * 5);
    f.write((char *)State::morales.data(), 4 * 5);
    f.write((char *)&State::followers_killed, 4);
    f.write((char *)&State::score, 4);
    f.write((char *)&State::weeks_passed, 4);
    f.write((char *)&State::timestop, 4);
    f.write((char *)&State::permanent_leadership, 4);
    f.write((char *)&State::known_spells, 4);
    f.write((char *)&State::days_passed_this_week, 4);
    f.write((char *)&State::spell_power, 4);
    f.write((char *)State::visited_castles.data(), 1 * 26);
    f.write((char *)&State::auto_move, 1);
    f.write((char *)&State::auto_move_dir, 8);
    f.write((char *)&State::boat_x, 4);
    f.write((char *)&State::boat_y, 4);
    f.write((char *)&State::boat_c, 4);
    f.write((char *)&State::boat_rented, 1);
    f.write((char *)&_sceptreX, 4);
    f.write((char *)&_sceptreY, 4);
    f.write((char *)&_sceptreContinent, 4);
    f.write((char *)&_lastWaterX, 4);
    f.write((char *)&_lastWaterY, 4);
    f.write((char *)&_dbgLastTile, sizeof(Tile));
    f.write((char *)&_dbgLastEventTile, sizeof(Tile));
    f.write((char *)State::artifacts_found.data(), 1 * 8);
    f.write((char *)State::continent_maps_found.data(), 1 * 4);
    f.write((char *)State::sail_maps_found.data(), 1 * 4);
    f.write((char *)State::villains_found.data(), 1 * 17);
    f.write((char *)State::villains_captured.data(), 1 * 17);

    for (int i = 0; i < 26; i++) {
        f.write((char *)State::castle_armies[i].data(), 4 * 5);
        f.write((char *)State::castle_counts[i].data(), 4 * 5);
        f.write((char *)&State::castle_occupants[i], 4);
        f.write((char *)State::garrison_armies[i].data(), 4 * 5);
        f.write((char *)State::garrison_counts[i].data(), 4 * 5);
    }

    f.write((char *)State::sail_map_tiles.data(), 24);
    f.write((char *)State::continent_map_tiles.data(), 32);
    f.write((char *)State::continent_map_tiles.data(), 64);
    for (int i = 0; i < 4; i++) {
        auto num_shops = State::shops[i].size();
        f.write((char *)&num_shops, 4);
        f.write((char *)State::shops[i].data(), sizeof(ShopInfo) * num_shops);
    }

    for (int i = 0; i < 4; i++) {
        auto num_friendly_mobs = State::friendly_mobs[i].size();
        f.write((char *)&num_friendly_mobs, 4);
        f.write((char *)State::friendly_mobs[i].data(), 4 * num_friendly_mobs);
    }

    for (int i = 0; i < 4; i++) {
        f.write((char *)State::visited_tiles[i].data(), 4096);
    }

    for (int i = 0; i < 4; i++) {
        f.write((char *)_map.getTiles(i), 4096);
    }

    auto heroPos = _spHero.getPosition();
    auto mount = _spHero.getMount();
    bool flip = _spHero.getFlip();
    f.write((char *)&heroPos, 8);
    f.write((char *)&mount, 4);
    f.write((char *)&flip, 1);

    auto boatPos = _spBoat.getPosition();
    f.write((char *)&boatPos, 8);

    auto boatFlip = _spBoat.getFlip();
    f.write((char *)&boatFlip, 1);

    f.write((char *)State::towns.data(), sizeof(TownGen) * State::towns.size());
}

void Ingame::loadState(std::ifstream &f)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 40; j++) {
            State::mobs[i][j].entity.setTexture(nullptr);
            f.read((char *)State::mobs[i][j].army.data(), 5 * 4);
            f.read((char *)State::mobs[i][j].counts.data(), 5 * 4);
            f.read((char *)&State::mobs[i][j].dead, 4);
            f.read((char *)&State::mobs[i][j].id, 4);
            f.read((char *)&State::mobs[i][j].spriteId, 4);
            f.read((char *)&State::mobs[i][j].tile, sizeof(Tile));
            glm::vec2 pos;
            f.read((char *)&pos, 8);
            State::mobs[i][j].entity.setPosition(pos);
            if (State::mobs[i][j].spriteId != -1 && !State::mobs[i][j].dead) {
                State::mobs[i][j].entity.setTexture(_texUnits[State::mobs[i][j].spriteId]);
            }
        }
    }

    f.read((char *)&State::hero, 4);
    f.read((char *)&State::rank, 4);
    f.read((char *)&State::days, 4);
    f.read((char *)&State::gold, 4);
    f.read((char *)&State::difficulty, 4);
    f.read((char *)&State::magic, 4);
    f.read((char *)&State::siege, 4);
    f.read((char *)&State::contract, 4);
    f.read((char *)&State::x, 4);
    f.read((char *)&State::y, 4);
    f.read((char *)&State::continent, 4);
    f.read((char *)&State::leadership, 4);
    f.read((char *)&State::commission, 4);
    f.read((char *)State::army.data(), 4 * 5);
    f.read((char *)State::counts.data(), 4 * 5);
    f.read((char *)State::morales.data(), 4 * 5);
    f.read((char *)&State::followers_killed, 4);
    f.read((char *)&State::score, 4);
    f.read((char *)&State::weeks_passed, 4);
    f.read((char *)&State::timestop, 4);
    f.read((char *)&State::permanent_leadership, 4);
    f.read((char *)&State::known_spells, 4);
    f.read((char *)&State::days_passed_this_week, 4);
    f.read((char *)&State::spell_power, 4);
    f.read((char *)State::visited_castles.data(), 1 * 26);
    f.read((char *)&State::auto_move, 1);
    f.read((char *)&State::auto_move_dir, 8);
    f.read((char *)&State::boat_x, 4);
    f.read((char *)&State::boat_y, 4);
    f.read((char *)&State::boat_c, 4);
    f.read((char *)&State::boat_rented, 1);
    f.read((char *)&_sceptreX, 4);
    f.read((char *)&_sceptreY, 4);
    f.read((char *)&_sceptreContinent, 4);
    f.read((char *)&_lastWaterX, 4);
    f.read((char *)&_lastWaterY, 4);
    f.read((char *)&_dbgLastTile, sizeof(Tile));
    f.read((char *)&_dbgLastEventTile, sizeof(Tile));
    f.read((char *)State::artifacts_found.data(), 1 * 8);
    f.read((char *)State::continent_maps_found.data(), 1 * 4);
    f.read((char *)State::sail_maps_found.data(), 1 * 4);
    f.read((char *)State::villains_found.data(), 1 * 17);
    f.read((char *)State::villains_captured.data(), 1 * 17);

    for (int i = 0; i < 26; i++) {
        f.read((char *)State::castle_armies[i].data(), 4 * 5);
        f.read((char *)State::castle_counts[i].data(), 4 * 5);
        f.read((char *)&State::castle_occupants[i], 4);
        f.read((char *)State::garrison_armies[i].data(), 4 * 5);
        f.read((char *)State::garrison_counts[i].data(), 4 * 5);
    }

    f.read((char *)State::sail_map_tiles.data(), 24);
    f.read((char *)State::continent_map_tiles.data(), 32);
    f.read((char *)State::continent_map_tiles.data(), 64);
    for (int i = 0; i < 4; i++) {
        int num_shops = 0;
        f.read((char *)&num_shops, 4);
        State::shops[i].resize(num_shops);
        f.read((char *)State::shops[i].data(), sizeof(ShopInfo) * num_shops);
    }

    for (int i = 0; i < 4; i++) {
        int num_friendly_mobs = 0;
        f.read((char *)&num_friendly_mobs, 4);
        State::friendly_mobs[i].resize(num_friendly_mobs);
        f.read((char *)State::friendly_mobs[i].data(), 4 * num_friendly_mobs);
    }

    for (int i = 0; i < 4; i++) {
        f.read((char *)State::visited_tiles[i].data(), 4096);
    }

    for (int i = 0; i < 4; i++) {
        f.read((char *)_map.getTiles(i), 4096);
    }

    _map.createGeometry();

    auto &hud {_engine.getGUI().getHUD()};
    hud.setHero(State::hero, State::rank);
    hud.setDays(State::days);
    hud.setGold(State::gold);
    hud.setMagic(State::magic);
    hud.setSiege(State::siege);
    hud.setContract(State::contract);
    hud.setPuzzle(State::villains_captured.data(), State::artifacts_found.data());

    if (State::timestop != 0) {
        hud.setTimestop(State::timestop);
    }

    glm::vec2 heroPos;
    f.read((char *)&heroPos, 8);
    _spHero.setPosition(heroPos);
    updateCamera();

    Mount mount;
    f.read((char *)&mount, 4);
    _spHero.setMount(mount);

    bool flip;
    f.read((char *)&flip, 1);
    _spHero.setFlip(flip);

    _dayTimer.reset();
    _timestopTimer.reset();
    _automoveTimer.reset();

    glm::vec2 boatPos;
    f.read((char *)&boatPos, 8);
    _spBoat.setPosition(boatPos);

    bool boatFlip;
    f.read((char *)&boatFlip, 1);
    _spBoat.setFlip(boatFlip);

    f.read((char *)State::towns.data(), sizeof(TownGen) * State::towns.size());
}

void Ingame::updateAnimations(float dt)
{
    _map.update(dt);
    _spHero.update(dt);

    for (auto &mob : State::mobs[State::continent]) {
        mob.entity.update(dt);
    }
}

void Ingame::winSiegeBattle(int castleId)
{
    State::castle_occupants[castleId] = -1;
    _engine.openGarrison(castleId);
}

void Ingame::winEncounterBattle(int mobId)
{
    State::mobs[State::continent][mobId % 40].dead = true;
}

void Ingame::pause()
{
    _paused = true;
    _moveFlags = DIR_FLAG_NONE;
    _engine.getGUI().pushDialog(_dlgPause);
}

void Ingame::moveCameraToSceptre()
{
    _tempPuzzleContinent = State::continent;
    State::continent = _sceptreContinent;
    _mapView = _uiView * glm::translate(-glm::vec3 {_sceptreX * 48.0f + 24.0f - 120, _sceptreY * 40.0f + 20.0f - 120, 0.0f});
}

void Ingame::setBoatPosition(float x, float y)
{
    _spBoat.setPosition(x, y);
}

void Ingame::tryJoin(int id, int count, std::function<void()> onOption)
{
    static constexpr const char *const kFleeMessage = {
        "{}\n"
        "\n"
        "   flee in terror at the\n"
        "   sight of your vast army.",
    };

    const std::string descriptor = fmt::format("{} {}", bty::unitDescriptor(count), kUnits[id].namePlural);
    _btJoinDescriptor->setString(descriptor);

    int heroArmySize = 0;
    for (int i = 0; i < 5; i++) {
        if (State::army[i] != -1) {
            heroArmySize++;
        }
    }

    if (heroArmySize == 5) {
        _engine.getGUI().showMessage(1, 21, 30, 6, fmt::format(kFleeMessage, descriptor), [this, onOption]() {
            onOption();
        });
    }
    else {
        _engine.getGUI().pushDialog(_dlgJoin);
        _dlgJoin.bind(Key::Enter, [this, onOption, id, count](int opt) {
            _engine.getGUI().popDialog();
            if (opt == 0) {
                addUnitToArmy(id, count);
            }
            onOption();
        });
    }
}
