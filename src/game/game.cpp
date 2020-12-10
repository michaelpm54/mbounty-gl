#include "game/game.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "assets.hpp"
#include "game/army-gen.hpp"
#include "game/chest.hpp"
#include "game/shop-info.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

std::string get_descriptor(int count);

Game::Game(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Game::load(bty::Assets &assets)
{
    bool success {true};

    assets_ = &assets;

    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    auto &state = scene_switcher_->state();
    auto color = bty::get_box_color(state.difficulty_level);

    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    hud_.load(assets, state);

    /* Create various pause-menu menu's */
    view_army_.load(assets, color);
    view_character_.load(assets, color, state.hero_id);
    view_continent_.load(assets, color);
    view_contract_.load(assets, color);
    view_puzzle_.load(assets);

    /* Load map */
    map_.load(assets);

    /* Load hero sprites */
    hero_.load(assets);

    /* Days run out/lose game */
    lose_msg_.create(1, 3, 20, 24, color, assets);
    lose_msg_name_ = lose_msg_.add_line(1, 2, "");
    lose_msg_.add_line(1, 4,
                       R"raw(you have failed to
recover the
Sceptre of Order
in time to save
the land! Beloved
King Maximus has
died and the Demon
King Urthrax
Killspite rules in
his place.  The
Four Continents
lay in ruin about
you, its people
doomed to a life
of misery and
oppression because
you could not find
the Sceptre.)raw");
    lose_pic_.set_texture(assets.get_texture("bg/king-dead.png"));
    lose_pic_.set_position(168, 24);

    town_.load(assets, color, state);
    kings_castle_.load(assets, color, state, hud_);
    shop_.load(assets, color, state, hud_);

    loaded_ = true;
    return success;
}

void Game::enter(bool reset)
{
    dialogs_.clear();
    if (reset) {
        setup_game();
    }
    else {
        auto &state = scene_switcher_->state();
        if (state.disgrace) {
            if (state.enemy_index != -1) {
                for (int i = 0; i < 6; i++) {
                    mobs_[state.continent][state.enemy_index].army[i] = state.enemy_army[i];
                    mobs_[state.continent][state.enemy_index].counts[i] = state.enemy_counts[i];
                }
            }
            disgrace();
        }
        else {
            if (state.enemy_index != -1) {
                mobs_[state.continent][state.enemy_index].dead = true;
            }
            sort_army(scene_switcher_->state().army, scene_switcher_->state().army_counts);
        }
    }
}

void Game::update_camera()
{
    glm::vec2 cam_centre = hero_.get_center();
    camera_pos_ = {cam_centre.x - 120, cam_centre.y - 120, 0.0f};
    game_camera_ = camera_ * glm::translate(-camera_pos_);
}

void Game::draw(bty::Gfx &gfx)
{
    bool tmp_hud {false};
    if (state_ == GameState::HudMessage) {
        state_ = last_state_;
        tmp_hud = true;
    }

    const auto &hero_tile = hero_.get_tile();
    int continent = scene_switcher_->state().continent;

    switch (state_) {
        case GameState::Unpaused:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            break;
        case GameState::ViewArmy:
            map_.draw(game_camera_, continent);
            hud_.draw(gfx, camera_);
            view_army_.draw(gfx, camera_);
            break;
        case GameState::ViewCharacter:
            hud_.draw(gfx, camera_);
            view_character_.draw(gfx, camera_);
            break;
        case GameState::ViewContinent:
            map_.draw(game_camera_, continent);
            hud_.draw(gfx, camera_);
            view_continent_.draw(gfx, camera_);
            break;
        case GameState::ViewContract:
            hud_.draw(gfx, camera_);
            view_contract_.draw(gfx, camera_);
            break;
        case GameState::LoseGame:
            hud_.draw(gfx, camera_);
            lose_msg_.draw(gfx, camera_);
            gfx.draw_sprite(lose_pic_, camera_);
            break;
        case GameState::ViewPuzzle:
            hud_.draw(gfx, camera_);
            view_puzzle_.draw(gfx, camera_);
            break;
        case GameState::Town:
            hud_.draw(gfx, camera_);
            town_.draw(gfx, camera_);
            break;
        case GameState::KingsCastle:
            hud_.draw(gfx, camera_);
            kings_castle_.draw(gfx, camera_);
            break;
        case GameState::Shop:
            hud_.draw(gfx, camera_);
            shop_.draw(gfx, camera_);
            break;
        case GameState::Bridge:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            bridge_prompt_.draw(gfx, camera_);
            break;
        default:
            break;
    }
    if (tmp_hud) {
        state_ = GameState::HudMessage;
    }

    for (auto &[dialog, d] : dialogs_) {
        dialog->draw(gfx, camera_);
    }
}

void Game::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    auto &state = scene_switcher_->state();

    switch (state_) {
        case GameState::Unpaused:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_G:
                            hero_.set_mount(hero_.get_mount() == Mount::Fly ? Mount::Walk : Mount::Fly);
                            break;
                        case GLFW_KEY_P:
                            hero_.set_debug(!hero_.get_debug());
                            for (int i = 0; i < mobs_[state.continent].size(); i++) {
                                if (mobs_[state.continent][i].dead) {
                                    continue;
                                }
                                mobs_[state.continent][i].entity.set_debug(!mobs_[state.continent][i].entity.get_debug());
                            }
                            break;
                        case GLFW_KEY_R:
                            gen_tiles();
                            break;
                        case GLFW_KEY_L:
                            set_state(GameState::LoseGame);
                            break;
                        case GLFW_KEY_B:
                            hero_.set_mount(hero_.get_mount() == Mount::Walk ? Mount::Boat : Mount::Walk);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::ViewContinent:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_SPACE:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                        case GLFW_KEY_ENTER:
                            if (state.local_maps_found[state.continent]) {
                                view_continent_fog_ = !view_continent_fog_;
                                view_continent();
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::ViewArmy:
            [[fallthrough]];
        case GameState::ViewCharacter:
            [[fallthrough]];
        case GameState::ViewPuzzle:
            [[fallthrough]];
        case GameState::ViewContract:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_SPACE:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            if (last_state_ == GameState::Town) {
                                set_state(GameState::Town);
                            }
                            else {
                                set_state(GameState::Unpaused);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::LoseGame:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER:
                            if (lose_state_ == 0) {
                                hud_.set_title("      Press Enter to play again.");
                                lose_state_++;
                            }
                            else {
                                scene_switcher_->fade_to(SceneId::Intro, true);
                            }
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::HudMessage:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            if (hud_message_queue_.empty()) {
                                if (last_state_ == GameState::Bridge) {
                                    last_state_ = GameState::Unpaused;
                                }
                                set_state(last_state_);
                            }
                            else {
                                hud_.set_title(hud_message_queue_.back());
                                hud_message_queue_.pop();
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::Town:
            switch (action) {
                case GLFW_PRESS:
                    town_option(town_.key(key));
                    break;
                default:
                    break;
            }
            break;
        case GameState::Shop:
            if (action == GLFW_PRESS || action == GLFW_RELEASE) {
                if (shop_.key(key, action) == -2) {
                    set_state(GameState::Unpaused);
                }
            }
            break;
        case GameState::KingsCastle:
            switch (action) {
                case GLFW_RELEASE:
                    [[fallthrough]];
                case GLFW_PRESS:
                    kings_castle_option(kings_castle_.key(key, action));
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    if (!paused_) {
        switch (action) {
            case GLFW_PRESS:
                switch (key) {
                    case GLFW_KEY_SPACE:
                        pause();
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    else if (!dialogs_.empty()) {
        std::pair<std::shared_ptr<bty::Dialog>, DialogDef> back;
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_BACKSPACE:
                    dialogs_.pop_back();
                    break;
                case GLFW_KEY_ENTER:
                    back = dialogs_.back();
                    if (dialogs_.back().second.pop_on_confirm) {
                        dialogs_.pop_back();
                    }
                    if (back.second.callbacks.confirm) {
                        back.second.callbacks.confirm(back.first->get_selection());
                    }
                    break;
                case GLFW_KEY_UP:
                    if (dialogs_.back().second.callbacks.up) {
                        dialogs_.back().second.callbacks.up(*dialogs_.back().first);
                    }
                    else {
                        dialogs_.back().first->prev();
                    }
                    break;
                case GLFW_KEY_DOWN:
                    if (dialogs_.back().second.callbacks.down) {
                        dialogs_.back().second.callbacks.down(*dialogs_.back().first);
                    }
                    else {
                        dialogs_.back().first->next();
                    }
                    break;
                case GLFW_KEY_LEFT:
                    if (dialogs_.back().second.callbacks.left) {
                        dialogs_.back().second.callbacks.left(*dialogs_.back().first);
                    }
                    break;
                case GLFW_KEY_RIGHT:
                    if (dialogs_.back().second.callbacks.right) {
                        dialogs_.back().second.callbacks.right(*dialogs_.back().first);
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

bool Game::loaded()
{
    return loaded_;
}

int coord(int x, int y)
{
    return ((x & 0xFF) << 8) | y;
}

void Game::town(const Tile &tile)
{
    int continent = scene_switcher_->state().continent;

    int town = -1;

    for (int i = 0; i < 26; i++) {
        if (kTownInfo[i].continent == continent && kTownInfo[i].x == tile.tx && 63 - kTownInfo[i].y == tile.ty) {
            town = i;
            break;
        }
    }

    if (town == -1) {
        spdlog::warn("Couldn't find town at {}, {}", tile.tx, tile.ty);
        return;
    }

    visited_towns_[town] = true;

    town_.view(town, tile, continent, town_units_[town], town_spells_[town]);
    set_state(GameState::Town);
}

void Game::collide(Tile &tile)
{
    int continent = scene_switcher_->state().continent;

    switch (tile.id) {
        case Tile_Chest:
            chest(tile);
            break;
        case Tile_Town:
            town(tile);
            break;
        case Tile_CastleB:
            if (tile.tx == 11 && tile.ty == 56) {
                set_state(GameState::KingsCastle);
            }
            else {
                castle(tile);
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
            artifact(tile);
            break;
        case Tile_ShopCave:
            if (glm::ivec2 {tile.tx, tile.ty} == teleport_caves_[continent][0] || glm::ivec2 {tile.tx, tile.ty} == teleport_caves_[continent][1]) {
                teleport_cave(tile);
            }
            else {
                shop(tile);
            }
            break;
        case Tile_ShopTree:
            [[fallthrough]];
        case Tile_ShopDungeon:
            [[fallthrough]];
        case Tile_ShopWagon:
            shop(tile);
            break;
        default:
            break;
    }
}

void Game::update(float dt)
{
    paused_ = !dialogs_.empty() || state_ != GameState::Unpaused;

    bool tmp_hud {false};
    if (state_ == GameState::HudMessage) {
        state_ = last_state_;
        tmp_hud = true;
    }

    if (!paused_) {
        if (controls_locked_) {
            auto_move(dt);
        }
        else {
            move_flags_ = DIR_FLAG_NONE;
            if (scene_switcher_->get_key(GLFW_KEY_LEFT)) {
                move_flags_ |= DIR_FLAG_LEFT;
            }
            if (scene_switcher_->get_key(GLFW_KEY_RIGHT)) {
                move_flags_ |= DIR_FLAG_RIGHT;
            }
            if (scene_switcher_->get_key(GLFW_KEY_UP)) {
                move_flags_ |= DIR_FLAG_UP;
            }
            if (scene_switcher_->get_key(GLFW_KEY_DOWN)) {
                move_flags_ |= DIR_FLAG_DOWN;
            }

            if (move_flags_) {
                move_hero(dt);
            }
            else if (!controls_locked_) {
                hero_.set_moving(false);
            }
        }

        map_.update(dt);
        hero_.animate(dt);
        hud_.update(dt);

        if (timestop_) {
            update_timestop(dt);
        }
        else {
            update_day_clock(dt);
            update_mobs(dt);
        }
    }
    else if (state_ == GameState::ViewContinent || state_ == GameState::ViewContract || state_ == GameState::ViewPuzzle) {
        hud_.update(dt);
    }
    if (state_ == GameState::ViewArmy) {
        view_army_.update(dt);
    }
    else if (state_ == GameState::ViewContinent) {
        view_continent_.update(dt);
    }
    else if (state_ == GameState::ViewPuzzle) {
        view_puzzle_.update(dt);
    }
    else if (state_ == GameState::Town) {
        hud_.update(dt);
        town_.update(dt);
    }
    else if (state_ == GameState::Shop) {
        hud_.update(dt);
        shop_.update(dt);
    }
    else if (state_ == GameState::KingsCastle) {
        kings_castle_.update(dt);
    }

    if (tmp_hud) {
        state_ = GameState::HudMessage;
    }

    for (auto &[dialog, _] : dialogs_) {
        dialog->animate(dt);
    }
}

/* clang-format off */
static constexpr int kMoraleGroups[25] = {
	0, 2, 0, 3, 4,
	4, 2, 3, 1, 2,
	1, 2, 2, 4, 1,
	3, 2, 3, 1, 2,
	2, 4, 2, 4, 3,
};

/* 0 = normal
   1 = high
   2 = low
*/
static constexpr int kMoraleChart[25] = {
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 0, 0,
	2, 0, 2, 1, 0,
	2, 2, 2, 0, 0,
};
/* clang-format on */

int check_morale(int me, int *army)
{
    bool normal = false;
    for (int i = 0; i < 5; i++) {
        if (army[i] == -1) {
            continue;
        }
        int result = kMoraleChart[kMoraleGroups[army[me]] + kMoraleGroups[army[i]] * 5];
        if (result == 2) {
            return 1;
        }
        if (result == 0) {
            normal = true;
        }
    }
    if (normal) {
        return 0;
    }
    return 2;
}

void Game::add_unit_to_army(int id, int count)
{
    if (id < 0 || id >= UnitId::UnitCount) {
        spdlog::warn("Game::add_unit_to_army: id out of range: {}", id);
        return;
    }

    int *army = scene_switcher_->state().army;
    int *army_counts = scene_switcher_->state().army_counts;
    int *army_morales = scene_switcher_->state().army_morales;

    for (int i = 0; i < 5; i++) {
        if (army[i] == id) {
            army_counts[i] += count;
            return;
        }
    }

    int army_size = 0;

    for (int i = 0; i < 5; i++) {
        if (scene_switcher_->state().army[i] != -1) {
            army_size++;
        }
    }

    if (army_size == 5) {
        spdlog::warn("Game::add_unit_to_army: army already full");
        return;
    }

    int index = army_size++;

    army[index] = id;
    army_counts[index] = count;
}

void Game::update_visited_tiles()
{
    const auto tile = hero_.get_tile();
    auto *visited = scene_switcher_->state().visited_tiles[scene_switcher_->state().continent].data();
    auto *tiles = map_.get_data(scene_switcher_->state().continent);

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

void Game::end_week_budget()
{
    auto &state = scene_switcher_->state();

    int army_total = 0;
    int commission = state.commission;

    for (int i = 0; i < 26; i++) {
        if (castle_occupants_[i] == -1) {
            commission += 250;
        }
    }

    int gold = state.gold;
    int boat = state.boat_rented ? (state.artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500) : 0;
    int balance = (commission + gold) - boat;

    std::vector<DialogDef::StringDef> strings;

    for (int i = 0; i < 5; i++) {
        if (state.army[i] == -1) {
            continue;
        }

        const auto &unit = kUnits[state.army[i]];

        int weekly_cost = unit.weekly_cost * state.army_counts[i];
        if (balance > weekly_cost || balance - weekly_cost == 0) {
            balance -= weekly_cost;
            army_total += weekly_cost;

            std::string cost = bty::number_with_ks(weekly_cost);
            std::string spaces(14 - (cost.size() + unit.name_plural.size()), ' ');
            strings.push_back({15, 3 + i, fmt::format("{}{}{}", unit.name_plural, spaces, cost)});
        }
        else {
            state.army[i] = -1;
            state.army_counts[i] = -1;

            std::string spaces(14 - (5 + unit.name_plural.size()), ' ');
            strings.push_back({15, 3 + i, fmt::format("{}{}Leave", unit.name_plural, spaces)});
        }
    }

    sort_army(state.army, state.army_counts);

    bool out_of_money = (boat + army_total) > (gold + commission);
    if (!out_of_money) {
        balance = (commission + gold) - (boat + army_total);
    }

    strings.push_back({1, 1, fmt::format("Week #{}", weeks_passed_)});
    strings.push_back({23, 1, "Budget"});
    strings.push_back({1, 3, fmt::format("On Hand: {}", bty::number_with_ks(state.gold))});
    strings.push_back({1, 4, fmt::format("Payment: {:>4}", commission)});
    strings.push_back({1, 5, fmt::format("Boat: {:>7}", boat)});
    strings.push_back({1, 6, fmt::format("Army: {:>7}", army_total)});
    strings.push_back({1, 7, fmt::format("Balance: {:>4}", bty::number_with_ks(balance))});

    state.gold = balance;

    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = strings,
    });

    if (state.army[0] == -1 || out_of_money) {
        disgrace();
    }
}

void Game::end_week_astrology()
{
    int unit = rand() % UnitId::UnitCount;
    const auto &name = kUnits[unit].name_singular;

    show_dialog({
        .x = 1,
        .y = 18,
        .w = 30,
        .h = 9,
        .strings = {
            {1, 1, fmt::format("Week #{}", weeks_passed_)},
            {1, 3, fmt::format("Astrologers proclaim:\nWeek of the {}\n\nAll {} dwellings are\nrepopulated.", name, name)},
        },
        .callbacks = {
            .confirm = [this](int) {
                end_week_budget();
            },
        },
    });

    auto &state = scene_switcher_->state();
    state.leadership = state.permanent_leadership;
}

void Game::sort_army(int *army, int *counts)
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

void Game::setup_game()
{
    set_state(GameState::Unpaused);

    auto &state = scene_switcher_->state();

    castle_armies_.clear();
    castle_counts_.clear();
    castle_armies_.resize(26);
    castle_counts_.resize(26);

    for (int i = 0; i < 26; i++) {
        castle_occupants_[i] = 0x7F;
        for (int j = 0; j < 5; j++) {
            castle_armies_[i][j] = -1;
            castle_counts_[i][j] = 0;
        }
    }

    for (int i = 0; i < 26; i++) {
        visited_towns_[i] = false;
        visited_castles_[i] = false;
    }

    /* Misc */
    controls_locked_ = false;
    control_lock_timer_ = 0;

    for (int i = 0; i < 4; i++) {
        friendlies_[i].clear();
    }

    /* Clear spells */
    for (int i = 0; i < 14; i++) {
        state.spells[i] = 0;
    }
    state.known_spells = 0;

    /* Known villains */
    for (int i = 0; i < 17; i++) {
        state.known_villains[i] = false;
    }

    /* Set days */
    state.days = kDays[state.difficulty_level];

    /* Set hero to starting position */
    hero_.move_to_tile(map_.get_tile(11, 58, scene_switcher_->state().continent));
    update_camera();

    /* Clear visited tiles */
    for (int i = 0; i < 4; i++) {
        state.visited_tiles[i].resize(4096);
        std::fill(state.visited_tiles[i].begin(), state.visited_tiles[i].end(), -1);
    }
    update_visited_tiles();

    /* Update box colours */
    auto color = bty::get_box_color(state.difficulty_level);
    hud_.set_color(color);
    view_army_.set_color(color);
    view_character_.set_color(color);
    view_continent_.set_color(color);
    view_contract_.set_color(color);
    town_.set_color(color);
    kings_castle_.set_color(color);
    shop_.set_color(color);

    /* Add starting army */
    for (int i = 0; i < 5; i++) {
        state.army[i] = -1;
        state.army_counts[i] = 0;
    }

    switch (state.hero_id) {
        case 0:
            state.army[0] = Militias;
            state.army_counts[0] = 20;
            state.army[1] = Archers;
            state.army_counts[1] = 2;
            break;
        case 1:
            state.army[0] = Peasants;
            state.army_counts[0] = 20;
            state.army[1] = Militias;
            state.army_counts[1] = 20;
            break;
        case 2:
            state.army[0] = Peasants;
            state.army_counts[0] = 30;
            state.army[1] = Sprites;
            state.army_counts[1] = 10;
            break;
        case 3:
            state.army[0] = Wolves;
            state.army_counts[0] = 20;
            break;
        default:
            break;
    }

    /* Set starting stats */
    state.contract = 17;
    state.gold = kStartingGold[state.hero_id];
    state.commission = kRankCommission[state.hero_id][0];
    state.permanent_leadership = kRankLeadership[state.hero_id][0];
    state.leadership = state.permanent_leadership;
    state.max_spells = kRankSpells[state.hero_id][0];
    state.spell_power = kRankSpellPower[state.hero_id][0];
    state.followers_killed = 0;

    if (state.hero_id == 2) {
        state.magic = true;
    }

    /* Add continentia to maps */
    for (int i = 0; i < 4; i++) {
        state.maps_found[i] = false;
        state.local_maps_found[i] = false;
    }
    state.maps_found[0] = true;

    /* Villains and artifacts */
    for (int i = 0; i < 17; i++) {
        state.villains_caught[i] = false;
    }
    for (int i = 0; i < 8; i++) {
        state.artifacts_found[i] = false;
    }

    hud_.update_state();

    gen_tiles();
}

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

void Game::gen_tiles()
{
    map_.reset();

    /* Clear mobs */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < mobs_[scene_switcher_->state().continent].size(); j++) {
            mobs_[i][j].dead = true;
        }
    }

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

    rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::shuffle(std::begin(artifacts), std::end(artifacts), rng_);

    int artifact_index = 0;

    for (int i = 0; i < 26; i++) {
        town_units_[i] = Peasants;
    }

    std::uniform_int_distribution<int> unit_gen(0, UnitId::UnitCount - 1);
    std::uniform_int_distribution<int> spell_gen(0, 13);

    for (int continent = 0; continent < 4; continent++) {
        auto *tiles = map_.get_data(continent);

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
                        town_units_[town] = unit_gen(rng_);

                        /* Hunterville always has Bridge */
                        if (town == 21) {
                            town_spells_[town] = 7;
                        }
                        else {
                            town_spells_[town] = spell_gen(rng_);
                        }

                        assert(town_spells_[town] != 14);

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
                    auto &mob = mobs_[continent][num_mobs++];
                    mob.tile = {x, y};
                    mob.dead = false;

                    gen_mob_army(continent, mob.army, mob.counts);
                    sort_army(mob.army.data(), mob.counts.data());

                    int highest = 0;
                    for (int i = 0; i < 5; i++) {
                        if (mob.army[i] == -1) {
                            continue;
                        }
                        if (kUnits[mob.army[i]].hp >= kUnits[mob.army[highest]].hp) {
                            highest = i;
                        }
                    }

                    mob.entity.set_texture(unit_textures_[mob.army[highest]]);
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
        teleport_caves_[continent][0] = tile;

        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = Tile_ShopCave;
        teleport_caves_[continent][1] = tile;

        /* 1 sail map */
        if (continent != 3) {
            /* Saharia doesn't have a map to anywhere */
            tile = random_tiles[used_tiles++];
            tiles[tile.x + tile.y * 64] = Tile_Chest;
            sail_maps_[continent] = tile;
        }

        /* 1 local map */
        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = Tile_Chest;
        local_maps_[continent] = tile;

        /* 2 artifacts */
        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifact_index++]];

        tile = random_tiles[used_tiles++];
        tiles[tile.x + tile.y * 64] = kArtifactTiles[artifacts[artifact_index++]];

        /* Init 11 shops. */
        shops_[continent].resize(11);
        for (int i = 0; i < 11; i++) {
            auto &shop = shops_[continent][i];
            shop.count = 0;
            shop.unit = Peasants;
            shop.x = 0;
            shop.y = 0;
        }

        int num_shops = 0;

        /* Gen guaranteed peasant dwelling. */
        if (continent == 0) {
            tiles[27 + 52 * 64] = Tile_ShopWagon;

            auto &shop = shops_[0][num_shops++];
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
            auto &shop = shops_[continent][num_shops++];
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
            auto &shop = shops_[continent][num_shops++];
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

            auto &mob = mobs_[continent][num_mobs++];
            mob.tile = {tile.x, tile.y};
            mob.dead = false;
            mob.army[0] = id;
            mob.counts[0] = count;
            mob.entity.set_texture(unit_textures_[id]);
            mob.entity.move_to_tile({tile.x, tile.y, Tile_Grass});

            tiles[tile.x + tile.y * 64] = Tile_Grass;

            friendlies_[continent].push_back(&mob);
        }

        /* Gen castle occupants */
        int used_castles = 0;
        for (int i = 0; i < kVillainsPerContinent[continent]; i++) {
            int castle;
            do {
                castle = bty::random(26);
            } while (castle_occupants_[castle] != 0x7F || kCastleInfo[castle].continent != continent);

            int villain = i + kVillainIndices[continent];

            auto &state = scene_switcher_->state();

            castle_occupants_[castle] = villain;
            gen_villain_army(villain, castle_armies_[castle], castle_counts_[castle]);
        }

        /* Turn the rest of the RNG tiles into chests */
        for (int i = used_tiles; i < random_tiles.size(); i++) {
            tiles[random_tiles[i].x + random_tiles[i].y * 64] = Tile_Chest;
        }
    }

    for (unsigned int i = 0; i < 26; i++) {
        if (castle_occupants_[i] != 0x7F) {
            continue;
        }
        gen_castle_army(kCastleInfo[i].continent, castle_armies_[i], castle_counts_[i]);
    }

    map_.create_geometry();
}

void Game::disgrace()
{
    hero_.move_to_tile(map_.get_tile(11, 58, 0));
    hero_.set_mount(Mount::Walk);
    hero_.set_flip(false);
    hero_.set_moving(false);

    auto &state = scene_switcher_->state();

    state.disgrace = false;
    state.army[0] = Peasants;
    state.army[1] = -1;
    state.army[2] = -1;
    state.army[3] = -1;
    state.army[4] = -1;
    state.army_counts[0] = 20;
    state.army_counts[1] = 0;
    state.army_counts[2] = 0;
    state.army_counts[3] = 0;
    state.army_counts[4] = 0;

    update_camera();

    show_dialog({
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

void Game::lose_game()
{
    switch (scene_switcher_->state().hero_id) {
        case 0:
            lose_msg_name_->set_string("Oh, Sir Crimsaun");
            break;
        case 1:
            lose_msg_name_->set_string("Oh, Lord Palmer");
            break;
        case 2:
            lose_msg_name_->set_string("Oh, Moham");
            break;
        case 3:
            lose_msg_name_->set_string("Oh, Tynnestra");
            break;
        default:
            break;
    }
    hud_.set_blank_frame();
    lose_state_ = 0;
}

void Game::view_puzzle()
{
    view_puzzle_.view(scene_switcher_->state().villains_caught, scene_switcher_->state().artifacts_found);
}

void Game::dismiss()
{
    auto &state = scene_switcher_->state();

    std::vector<DialogDef::StringDef> options;

    for (int i = 0; i < 5; i++) {
        if (state.army[i] != -1) {
            options.push_back({11, 3 + i, kUnits[state.army[i]].name_plural});
        }
    }

    show_dialog({
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

void Game::dismiss_slot(int slot)
{
    int num_units = 0;
    for (int i = 0; i < 5; i++) {
        if (scene_switcher_->state().army[i] != -1) {
            num_units++;
        }
    }

    if (num_units == 1) {
        hud_messages({"  You may not dismiss your last army!"});
        return;
    }

    scene_switcher_->state().army[slot] = -1;
    scene_switcher_->state().army_counts[slot] = 0;
    sort_army(scene_switcher_->state().army, scene_switcher_->state().army_counts);
    dismiss();
}

void Game::set_state(GameState state)
{
    last_state_ = state_;
    state_ = state;

    switch (state) {
        case GameState::HudMessage:
            break;
        case GameState::Unpaused:
            if (hud_message_queue_.empty()) {
                hud_.set_hud_frame();
                hud_.update_state();
            }
            break;
        case GameState::LoseGame:
            lose_game();
            break;
        case GameState::ViewArmy:
            view_army();
            break;
        case GameState::ViewCharacter:
            view_character_.view(scene_switcher_->state());
            break;
        case GameState::ViewContinent:
            view_continent();
            break;
        case GameState::ViewContract:
            view_contract();
            break;
        case GameState::ViewPuzzle:
            view_puzzle();
            break;
        case GameState::KingsCastle:
            kings_castle_.view();
            break;
        case GameState::Shop:
            shop_.view(shops_[scene_switcher_->state().continent][shop_index_]);
            break;
        default:
            break;
    }

    clear_movement();
}

void Game::clear_movement()
{
    move_flags_ = DIR_FLAG_NONE;
    hero_.set_moving(false);
}

void Game::sail_to(int continent)
{
    if (continent == scene_switcher_->state().continent) {
        auto pos = hero_.get_center();

        auto_move_dir_ = {0, 0};

        if (pos.x < 0) {
            auto_move_dir_.x = 1;
            hero_.set_flip(false);
        }
        else if (pos.x > 48 * 64) {
            auto_move_dir_.x = -1;
            hero_.set_flip(true);
        }

        if (pos.y < 0) {
            auto_move_dir_.y = 1;
        }
        else if (pos.y > 40 * 64) {
            auto_move_dir_.y = -1;
        }

        control_lock_timer_ = 1;
        controls_locked_ = true;

        hero_.set_moving(true);
    }
    else {
        controls_locked_ = true;
        hero_.set_moving(true);

        auto_move_dir_ = {0, 0};

        switch (continent) {
            case 0:
                // Up
                control_lock_timer_ = 1;
                auto_move_dir_.y = -1;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(11, 64 - 1, continent));
                break;
            case 1:
                // Right
                control_lock_timer_ = 0.4f;
                auto_move_dir_.x = 1;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(0, 64 - 25, continent));
                break;
            case 2:
                // Down
                control_lock_timer_ = 0.6f;
                auto_move_dir_.y = 1;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(14, 0, continent));
                break;
            case 3:
                // Up
                control_lock_timer_ = 1;
                auto_move_dir_.y = -1;
                hero_.set_flip(false);
                hero_.move_to_tile(map_.get_tile(9, 64 - 1, continent));
                break;
            default:
                break;
        }

        hero_.set_mount(Mount::Boat);
        update_camera();
        scene_switcher_->state().continent = continent;
    }
}

void Game::rent_boat()
{
    scene_switcher_->state().boat_rented = !scene_switcher_->state().boat_rented;
}

void Game::town_option(int opt)
{
    switch (opt) {
        case -1:    // internal option
            break;
        case -2:    // close town
            set_state(GameState::Unpaused);
            break;
        case 0:
            next_contract();
            break;
        case 1:
            rent_boat();
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
}

void Game::next_contract()
{
    auto &state = scene_switcher_->state();

    int num_caught = 0;
    for (int i = 0; i < 17; i++) {
        if (state.villains_caught[i]) {
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
        if (!state.villains_caught[i]) {
            contracts[have++] = i;
            if (have == 5) {
                break;
            }
        }
    }

    int current = 0;
    for (int i = 0; i < 5; i++) {
        if (state.contract == contracts[i]) {
            current = i;
        }
    }

    if (state.contract == 17 && contracts[0] == 0) {
        state.contract = 0;
    }
    else {
        state.contract = contracts[(current + 1) % 5];
    }

    hud_.update_state();
    set_state(GameState::ViewContract);
}

void Game::view_contract()
{
    auto &state = scene_switcher_->state();

    std::string castle;

    if (state.contract < 17 && state.known_villains[state.contract]) {
        int castle_id = -1;
        for (int i = 0; i < 26; i++) {
            if (castle_occupants_[i] == state.contract) {
                castle_id = i;
                break;
            }
        }
        if (castle_id != -1) {
            castle = kCastleInfo[castle_id].name;
        }
        else {
            spdlog::warn("Can't find castle for villain {}", state.contract);
            castle = "Unknown";
        }
    }
    else {
        castle = "Unknown";
    }

    view_contract_.view(scene_switcher_->state().contract, castle, hud_.get_contract());
}

void Game::buy_spell()
{
    auto &state = scene_switcher_->state();

    int town = town_.get_town();

    if (state.known_spells < state.max_spells) {
        if (state.gold >= kSpellCosts[town_spells_[town]]) {
            state.gold -= kSpellCosts[town_spells_[town]];
            state.spells[town_spells_[town_.get_town()]]++;
            state.known_spells++;
            town_.update_gold();

            int remaining = state.max_spells - state.known_spells;
            hud_.set_title(fmt::format("     You can learn {} more spell{}.", remaining, remaining != 1 ? "s" : ""));
        }
        else {
            hud_messages({"    You do not have enough gold!"});
        }
    }
    else {
        hud_messages({"  You can not learn anymore spells!"});
    }
}

void Game::buy_siege()
{
    auto &state = scene_switcher_->state();

    if (state.gold < 3000) {
        hud_messages({"    You do not have enough gold!"});
    }
    else {
        state.gold -= 3000;
        state.siege = true;
        hud_.update_state();
        town_.update_gold();
    }
}

void Game::kings_castle_option(int opt)
{
    switch (opt) {
        case -2:
            set_state(GameState::Unpaused);
            break;
        default:
            break;
    }
}

void Game::use_spell(int spell)
{
    auto &state = scene_switcher_->state();

    if (!state.magic) {
        show_dialog({
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

    state.spells[spell + 7]--;

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

void Game::place_bridge_at(int x, int y, int continent, bool horizontal)
{
    auto destination = map_.get_tile(x, y, continent);

    if (destination.id == -1 || destination.id < Tile_WaterIRT || destination.id > Tile_Water) {
        bridge_fail();
    }
    else {
        map_.set_tile({x, y, -1}, continent, horizontal ? Tile_BridgeHorizontal : Tile_BridgeVertical);
    }
}

void Game::bridge_fail()
{
    last_state_ = GameState::Unpaused;
    hud_messages({" Not a suitable location for a bridge!", "    What a waste of a good spell!"});
}

void Game::hud_messages(const std::vector<std::string> &messages)
{
    if (messages.empty()) {
        return;
    }

    for (auto &m : messages) {
        hud_message_queue_.push(m);
    }

    /* Stupid but it has the semantics I want. */
    hud_.set_title(messages[0]);
    hud_message_queue_.pop();

    set_state(GameState::HudMessage);
}

void Game::draw_mobs(bty::Gfx &gfx)
{
    const auto &hero_tile = hero_.get_tile();
    int continent = scene_switcher_->state().continent;

    for (int i = 0; i < mobs_[continent].size(); i++) {
        if (mobs_[continent][i].dead) {
            continue;
        }
        if (std::abs(hero_tile.tx - mobs_[continent][i].tile.x) < 4 && std::abs(hero_tile.ty - mobs_[continent][i].tile.y) < 4) {
            mobs_[continent][i].entity.draw(gfx, game_camera_);
        }
    }
}

void Game::view_army()
{
    auto &state = scene_switcher_->state();
    for (int i = 0; i < 5; i++) {
        if (state.army_counts[i] * kUnits[state.army[i]].hp > state.leadership) {
            state.army_morales[i] = 3;
            continue;
        }

        state.army_morales[i] = check_morale(i, state.army);
    }
    view_army_.view(state.army, state.army_counts, state.army_morales);
}

void Game::artifact(const Tile &tile)
{
    auto &state = scene_switcher_->state();

    map_.set_tile(tile, state.continent, Tile_Grass);

    switch (tile.id) {
        case Tile_AfctRing:
            state.artifacts_found[ArtiRingOfHeroism] = true;
            break;
        case Tile_AfctAmulet:
            state.artifacts_found[ArtiAmuletOfAugmentation] = true;
            break;
        case Tile_AfctAnchor:
            state.artifacts_found[ArtiAnchorOfAdmirality] = true;
            break;
        case Tile_AfctCrown:
            state.artifacts_found[ArtiCrownOfCommand] = true;
            break;
        case Tile_AfctScroll:
            state.artifacts_found[ArtiArticlesOfNobility] = true;
            break;
        case Tile_AfctShield:
            state.artifacts_found[ArtiShieldOfProtection] = true;
            break;
        case Tile_AfctSword:
            state.artifacts_found[ArtiSwordOfProwess] = true;
            break;
        case Tile_AfctBook:
            state.artifacts_found[ArtiBookOfNecros] = true;
            break;
        default:
            break;
    }

    hud_.update_state();
}

void Game::teleport_cave(const Tile &tile)
{
    int continent = scene_switcher_->state().continent;
    glm::ivec2 dest = glm::ivec2 {tile.tx, tile.ty} == teleport_caves_[continent][0] ? teleport_caves_[continent][1] : teleport_caves_[continent][0];
    hero_.move_to_tile(map_.get_tile(dest, continent));
    update_camera();
}

std::string get_descriptor(int count)
{
    static constexpr const char *const kDescriptors[] = {
        "A few",
        "Some",
        "Many",
        "A lot",
        "A horde",
        "A multitude",
    };

    static constexpr int kThresholds[] = {
        10,
        20,
        50,
        100,
        500,
    };

    int descriptor = 0;
    while (count >= kThresholds[descriptor] && descriptor < 5) {
        descriptor++;
    }

    return kDescriptors[descriptor];
}

void Game::shop(const Tile &tile)
{
    auto &state = scene_switcher_->state();

    for (int i = 0; i < shops_[state.continent].size(); i++) {
        if (shops_[state.continent][i].x == tile.tx && shops_[state.continent][i].y == tile.ty) {
            shop_index_ = i;
            break;
        }
    }
    if (shop_index_ == -1) {
        spdlog::warn("Failed to find shop at [{}] {} {}", state.continent, tile.tx, tile.ty);
    }
    else {
        set_state(GameState::Shop);
    }
}

void Game::chest(const Tile &tile)
{
    auto &state = scene_switcher_->state();

    map_.set_tile(tile, state.continent, Tile_Grass);

    if (state.continent < 3 && tile.tx == sail_maps_[state.continent].x && tile.ty == sail_maps_[state.continent].y) {
        state.maps_found[state.continent + 1] = true;
        hud_.set_title("You found a map!");
        show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = {
                {3, 2, "Hidden within an ancient"},
                {3, 3, "chest, you find maps and"},
                {1, 4, "charts describing passage to"},
                {10, 6, kContinents[state.continent + 1]},
            },
        });
    }
    else if (tile.tx == local_maps_[state.continent].x && tile.ty == local_maps_[state.continent].y) {
        state.local_maps_found[state.continent] = true;
        view_continent_fog_ = false;
        show_dialog({
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
                    set_state(GameState::ViewContinent);
                },
            },
        });
    }
    else {
        chest_roll(state, std::bind(&Game::show_dialog, this, std::placeholders::_1));
    }
}

void Game::view_continent()
{
    view_continent_.view(
        hero_.get_tile().tx,
        hero_.get_tile().ty,
        scene_switcher_->state().continent,
        view_continent_fog_ ? scene_switcher_->state().visited_tiles[scene_switcher_->state().continent].data() : map_.get_data(scene_switcher_->state().continent));
}

bty::Dialog *Game::show_dialog(const DialogDef &dialog_)
{
    auto dialog = std::make_shared<bty::Dialog>();
    dialog->create(dialog_.x, dialog_.y, dialog_.w, dialog_.h, bty::get_box_color(scene_switcher_->state().difficulty_level), *assets_);
    for (const auto &str : dialog_.strings) {
        dialog->add_line(str.x, str.y, str.str);
    }
    for (const auto &opt : dialog_.options) {
        dialog->add_option(opt.x, opt.y, opt.str);
    }
    for (auto i = 0u; i < dialog_.visible_options.size(); i++) {
        dialog->set_option_visibility(i++, dialog_.visible_options[i]);
    }
    dialogs_.push_back({dialog, dialog_});
    return dialogs_.back().first.get();
}

static constexpr int kTownsAlphabetical[] = {
    0x03,
    0x0e,
    0x07,
    0x10,
    0x0c,
    0x16,
    0x12,
    0x15,
    0x11,
    0x13,
    0x0d,
    0x05,
    0x09,
    0x0f,
    0x0b,
    0x02,
    0x08,
    0x00,
    0x06,
    0x04,
    0x01,
    0x14,
    0x18,
    0x0a,
    0x17,
    0x19,
    0x01,
    0x02,
    0x04,
    0x05,
    0x06,
    0x08,
};

void Game::town_gate_confirm(int opt)
{
    static constexpr int kTownGateX[] = {
        0x1d,
        0x39,
        0x26,
        0x23,
        0x05,
        0x10,
        0x0c,
        0x09,
        0x0d,
        0x39,
        0x33,
        0x39,
        0x03,
        0x10,
        0x28,
        0x32,
        0x3a,
        0x38,
        0x09,
        0x0d,
        0x06,
        0x0c,
        0x2f,
        0x32,
        0x03,
        0x3a,
    };

    static constexpr int kTownGateY[] = {
        0x0b,
        0x04,
        0x31,
        0x17,
        0x31,
        0x2c,
        0x3c,
        0x26,
        0x1b,
        0x21,
        0x1d,
        0x38,
        0x24,
        0x15,
        0x3a,
        0x0e,
        0x3b,
        0x05,
        0x3b,
        0x08,
        0x03,
        0x04,
        0x23,
        0x08,
        0x09,
        0x2f,
    };

    hero_.move_to_tile(map_.get_tile(kTownGateX[kTownsAlphabetical[opt]], 63 - kTownGateY[kTownsAlphabetical[opt]], kTownInfo[opt].continent));
    scene_switcher_->state().continent = kTownInfo[kTownsAlphabetical[opt]].continent;
    update_camera();
}

void Game::castle_gate_confirm(int opt)
{
    spdlog::debug("Castle gate confirmed: {}", opt);
    hero_.move_to_tile(map_.get_tile(kCastleInfo[opt].x, (63 - kCastleInfo[opt].y) + 1, kCastleInfo[opt].continent));
    scene_switcher_->state().continent = kCastleInfo[opt].continent;
    update_camera();
}

void Game::update_timestop(float dt)
{
    timestop_timer_ += dt;
    if (timestop_timer_ >= 0.25f) {
        timestop_left_--;
        timestop_timer_ = 0;
        hud_.set_timestop(timestop_left_);
    }
    if (timestop_left_ == 0) {
        hud_.clear_timestop();
        timestop_ = false;
    }

    int continent = scene_switcher_->state().continent;

    for (auto &mob : mobs_[continent]) {
        mob.entity.animate(dt);
    }
}

void Game::update_day_clock(float dt)
{
    clock_ += dt;
    if (clock_ >= 16) {
        clock_ = 0;
        int &days = scene_switcher_->state().days;
        days--;
        if (days == 0) {
            set_state(GameState::LoseGame);
        }
        else {
            days_passed_this_week++;
            if (days_passed_this_week == 5) {
                days_passed_this_week = 0;
                weeks_passed_++;
                end_week_astrology();
            }
            hud_.update_state();
        }
    }
}

void Game::update_mobs(float dt)
{
    const auto &hero_tile = hero_.get_tile();
    const auto &hero_pos = hero_.get_position();

    int continent = scene_switcher_->state().continent;

    for (int i = 0; i < mobs_[continent].size(); i++) {
        auto &mob = mobs_[continent][i];
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

            if (hero_.get_mount() == Mount::Walk && distance_x < 12.0f && distance_y < 12.0f) {
                for (int j = 0; j < friendlies_[continent].size(); j++) {
                    if (friendlies_[continent][j] == &mob) {
                        int size = 0;
                        for (int i = 0; i < 5; i++) {
                            if (scene_switcher_->state().army[i] != -1) {
                                size++;
                            }
                        }

                        const std::string descriptor = fmt::format("{} {}", get_descriptor(mob.counts[0]), kUnits[mob.army[0]].name_plural);

                        if (size == 5) {
                            show_dialog({
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
                            show_dialog({
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

                scene_switcher_->state().enemy_army = mob.army;
                scene_switcher_->state().enemy_counts = mob.counts;
                scene_switcher_->state().enemy_index = i;
                scene_switcher_->fade_to(SceneId::Battle, true);
                return;
            }

            mob.entity.animate(dt);
            mob.entity.set_flip(hero_pos.x < mob_pos.x);

            float speed = 70.0f;
            float vel = speed * dt;
            float dx = dir.x * vel;
            float dy = dir.y * vel;

            auto manifold = mob.entity.move(dx, dy, map_, scene_switcher_->state().continent);

            mob.entity.set_position(manifold.new_position);

            for (auto j = 0u; j < mobs_[continent].size(); j++) {
                if (i == j || mobs_[continent][j].dead) {
                    continue;
                }

                auto &other_mob = mobs_[continent][j];
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

void Game::auto_move(float dt)
{
    control_lock_timer_ -= dt;
    if (control_lock_timer_ <= 0) {
        controls_locked_ = false;
    }

    float speed = 100;
    float vel = speed * dt;
    float dx = auto_move_dir_.x * vel;
    float dy = auto_move_dir_.y * vel;

    auto manifold = hero_.move(dx, dy, map_, scene_switcher_->state().continent);
    hero_.set_position(manifold.new_position);
    update_visited_tiles();
    update_camera();
}

void Game::move_hero(float dt)
{
    hero_.set_moving(true);

    if ((move_flags_ & DIR_FLAG_LEFT) && !(move_flags_ & DIR_FLAG_RIGHT)) {
        hero_.set_flip(true);
    }
    else if ((move_flags_ & DIR_FLAG_RIGHT) && !(move_flags_ & DIR_FLAG_LEFT)) {
        hero_.set_flip(false);
    }

    glm::vec2 dir {0.0f};

    if (move_flags_ & DIR_FLAG_UP)
        dir.y -= 1.0f;
    if (move_flags_ & DIR_FLAG_DOWN)
        dir.y += 1.0f;
    if (move_flags_ & DIR_FLAG_LEFT)
        dir.x -= 1.0f;
    if (move_flags_ & DIR_FLAG_RIGHT)
        dir.x += 1.0f;

    float speed = hero_.get_mount() == Mount::Fly ? 200.0f : 100.0f;
    float vel = speed * dt;
    float dx = dir.x * vel;
    float dy = dir.y * vel;

    auto manifold = hero_.move(dx, dy, map_, scene_switcher_->state().continent);
    hero_.set_position(manifold.new_position);

    if (manifold.collided) {
        for (auto &tile : manifold.collided_tiles) {
            collide(tile);
        }
    }
    if (auto ht = hero_.get_center(); !controls_locked_ && (ht.x < 0 || ht.x > 3072 || ht.y < 0 || ht.y > 40 * 64)) {
        std::vector<DialogDef::StringDef> continents;

        for (int i = 0; i < 4; i++) {
            if (scene_switcher_->state().maps_found[i]) {
                continents.push_back({10, 3 + i, kContinents[i]});
            }
        }

        show_dialog({
            .x = 1,
            .y = 18,
            .w = 30,
            .h = 9,
            .strings = {{3, 1, "Sail to which continent?"}},
            .options = continents,
            .callbacks = {
                .confirm = std::bind(&Game::sail_to, this, std::placeholders::_1),
            },
        });
    }
    else {
        if (manifold.changed_tile) {
            hero_.set_tile_info(manifold.new_tile);
            if (hero_.get_mount() == Mount::Boat && manifold.new_tile.id == Tile_Grass) {
                hero_.set_mount(Mount::Walk);
            }
            update_visited_tiles();
        }
    }

    update_camera();
}

void Game::pause()
{
    show_dialog({
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
            .confirm = std::bind(&Game::pause_confirm, this, std::placeholders::_1),
        },
    });
}

void Game::pause_confirm(int opt)
{
    switch (opt) {
        case 0:
            set_state(GameState::ViewArmy);
            break;
        case 1:
            set_state(GameState::ViewCharacter);
            break;
        case 2:
            set_state(GameState::ViewContinent);
            break;
        case 3:
            use_magic();
            break;
        case 4:
            set_state(GameState::ViewContract);
            break;
        case 5:
            /*
                                        9 becomes 5
                                        8 becomes 5
                                        7 becomes 5
                                        etc
                                     */
            if ((scene_switcher_->state().days % 5) != 0) {
                scene_switcher_->state().days = (scene_switcher_->state().days / 5) * 5;
            }
            /*
                                        100 becomes 95
                                        65 becomes 60
                                        10 becomes 5
                                        etc
                                    */
            else {
                scene_switcher_->state().days = ((scene_switcher_->state().days / 5) - 1) * 5;
            }
            if (scene_switcher_->state().days == 0) {
                set_state(GameState::LoseGame);
            }
            else {
                weeks_passed_++;
                end_week_astrology();
            }
            clock_ = 0;
            hud_.update_state();
            break;
        case 6:
            set_state(GameState::ViewPuzzle);
            break;
        case 7:
            break;
        case 8:
            dismiss();
            break;
        default:
            break;
    }
}

void Game::use_magic()
{
    int *spells = scene_switcher_->state().spells;
    std::vector<DialogDef::StringDef> options;

    for (int i = 7; i < 14; i++) {
        options.push_back({4, 3 + i - 7, fmt::format("{} {}", spells[i], kSpellNames[i])});
    }

    for (int i = 0; i < 7; i++) {
        options.push_back({4, 14 + i, fmt::format("{} {}", spells[i], kSpellNames[i])});
    }

    bool no_spells = true;
    for (int i = 7; i < 14; i++) {
        if (spells[i] != 0) {
            no_spells = false;
            break;
        }
    }

    auto confirm = no_spells ? std::function<void(int)>(nullptr) : std::bind(&Game::use_spell, this, std::placeholders::_1);

    auto *dialog = show_dialog({
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

    for (int i = 7; i < 14; i++) {
        dialog->set_option_disabled(i, true);
    }

    if (no_spells) {
        hud_messages({"You have no Adventuring spell to cast!"});
        for (int i = 0; i < 7; i++) {
            dialog->set_option_disabled(i, true);
        }
    }
}

void Game::spell_bridge()
{
    int c = scene_switcher_->state().continent;
    auto hero_tile = hero_.get_tile();
    int x = hero_tile.tx;
    int y = hero_tile.ty;

    show_dialog({
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
                    set_state(GameState::Unpaused);
                    place_bridge_at(x, y - 1, c, false);
                    dialogs_.pop_back();
                },
            },
            .down = {
                [this, x, y, c](bty::Dialog &) {
                    set_state(GameState::Unpaused);
                    place_bridge_at(x, y + 1, c, false);
                    dialogs_.pop_back();
                },
            },
            .left = {
                [this, x, y, c](bty::Dialog &) {
                    set_state(GameState::Unpaused);
                    place_bridge_at(x - 1, y, c, true);
                    dialogs_.pop_back();
                },
            },
            .right = {
                [this, x, y, c](bty::Dialog &) {
                    set_state(GameState::Unpaused);
                    place_bridge_at(x + 1, y, c, true);
                    dialogs_.pop_back();
                },
            },
        },
    });
}

void Game::spell_timestop()
{
    auto &state = scene_switcher_->state();
    timestop_ = true;
    timestop_timer_ = 0;
    timestop_left_ += state.spell_power * 10;
    if (timestop_left_ > 9999) {
        timestop_left_ = 9999;
    }
    hud_.set_timestop(timestop_left_);
    set_state(GameState::Unpaused);
}

void Game::spell_find_villain()
{
    auto &state = scene_switcher_->state();
    if (state.contract < 17) {
        state.known_villains[state.contract] = true;
    }
    set_state(GameState::ViewContract);
}

void tc_gate_left(bty::Dialog &dialog);
void tc_gate_right(bty::Dialog &dialog);

void Game::spell_tc_gate(bool town)
{
    set_state(GameState::Unpaused);

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
            visible_options[n] = town ? visited_towns_[n] : visited_castles_[n];
            n++;
        }
    }

    show_dialog({
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
            .confirm = std::bind(town ? &Game::town_gate_confirm : &Game::castle_gate_confirm, this, std::placeholders::_1),
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
    for (int i = 0; i < 13; i++) {
        int test = selection > 12 ? (13 + i) : (i);
        int distance = std::abs(selection - test);
        if (distance < nearest_distance) {
            if (dialog.get_option_visible(test)) {
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
    for (int i = 0; i < 13; i++) {
        int test = selection > 12 ? (13 + i) : (i);
        int distance = std::abs(selection - test);
        if (distance < nearest_distance) {
            if (dialog.get_option_visible(test)) {
                nearest_index = test;
                nearest_distance = distance;
            }
        }
    }
    dialog.set_selection(nearest_index);
}

void Game::spell_instant_army()
{
    auto &state = scene_switcher_->state();

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

    int unit = kInstantArmyUnits[state.hero_rank][state.hero_id];
    int amt = state.spell_power * 3 + (rand() % state.spell_power + 2);
    add_unit_to_army(unit, amt);

    show_dialog({
        .x = 1,
        .y = 21,
        .w = 30,
        .h = 6,
        .strings = {
            {1, 1, fmt::format("{} {}", get_descriptor(amt), kUnits[unit].name_plural)},
            {3, 3, "have joined your army."},
        },
    });
}

void Game::spell_raise_control()
{
    auto &state = scene_switcher_->state();
    state.leadership += state.spell_power * 100;
    state.leadership &= 0xFFFF;
}

void Game::castle(const Tile &tile)
{
    auto &state = scene_switcher_->state();

    int castle_id = -1;

    for (int i = 0; i < 26; i++) {
        if (kCastleInfo[i].x == tile.tx && kCastleInfo[i].y == 63 - tile.ty && kCastleInfo[i].continent == state.continent) {
            castle_id = i;
        }
    }

    if (castle_id == -1) {
        spdlog::warn("Failed to find castle at [{},{}] in {}", tile.tx, tile.ty, kContinents[state.continent]);
        return;
    }

    int occupier = castle_occupants_[castle_id];

    if (occupier == -1) {
        spdlog::warn("TODO: garrisoned castles");
        return;
    }

    std::string occ_name = occupier == 0x7F ? "Various groups of monsters" : fmt::format("{} and", kVillains[occupier][0]);
    std::string line_two = occupier == 0x7F ? "occupy this castle" : "army occupy this castle";

    show_dialog({
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
            .confirm = [this, castle_id, &state](int opt) {
                if (opt == 0) {
                    for (int i = 0; i < 5; i++) {
                        state.enemy_army[i] = castle_armies_[castle_id][i];
                        state.enemy_counts[i] = castle_counts_[castle_id][i];
                    }
                    state.enemy_index = -1;
                    scene_switcher_->fade_to(SceneId::Battle, true);
                }
            },
        },
    });

    visited_castles_[castle_id] = true;
}
