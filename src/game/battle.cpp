#include "game/battle.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "data/bounty.hpp"
#include "data/castles.hpp"
#include "data/hero.hpp"
#include "data/spells.hpp"
#include "data/villains.hpp"
#include "engine/assets.hpp"
#include "engine/dialog-stack.hpp"
#include "engine/scene-stack.hpp"
#include "game/army-gen.hpp"
#include "game/game-controls.hpp"
#include "game/game-options.hpp"
#include "game/gen-variables.hpp"
#include "game/hud.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "window/glfw.hpp"

static constexpr char const *kSiegeVictoryMessage = {
    R"raw(          Victory!
          ________
   Well done {},
    you have successfully
 vanquished yet another foe.

   Spoils of War: {} gold.
     And the capture of
	  {}!

For fulfilling your contract
  you receive an additional
     {} gold as bounty
  and a piece of the map to
     the stolen sceptre.)raw",
};

static constexpr char const *kSiegeVictoryMessageNoContract = {
    R"raw(          Victory!
          ________
   Well done {},
    you have successfully
 vanquished yet another foe.

   Spoils of War: {} gold.
     And the capture of
	  {}!


   Since you did not have
    the proper contract,
 the Lord has been set free.)raw",
};

static constexpr char const *kEncounterVictoryMessage = {
    R"raw(          Victory!
          ________
   Well done {},
    you have successfully
 vanquished yet another foe.

   Spoils of War: {} gold.
	)raw",
};

Battle::Battle(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, ViewArmy &view_army_, ViewCharacter &view_character_, GameControls &game_controls, GameOptions &game_options, Hud &hud)
    : ss(ss)
    , ds(ds)
    , v(v)
    , gen(gen)
    , s_view_army(view_army_)
    , s_view_character(view_character_)
    , s_controls(game_controls)
    , game_options(game_options)
    , hud(hud)
{
    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    auto color = bty::BoxColor::Intro;
    auto &font = assets.get_font();

    encounter_bg = assets.get_texture("battle/encounter.png");
    siege_bg = assets.get_texture("battle/siege.png");

    bg_.set_position(8, 24);
    current_friendly = assets.get_texture("battle/active-unit.png", {5, 2});
    current_enemy = assets.get_texture("battle/enemy.png", {10, 1});
    current_.set_texture(current_friendly);
    hit_marker_.set_texture(assets.get_texture("battle/damage-marker.png", {4, 1}));
    hit_marker_.set_animation_repeat(false);

    move_ = assets.get_texture("battle/selection.png", {4, 1});
    melee_ = assets.get_texture("battle/melee.png", {4, 1});
    shoot_ = assets.get_texture("battle/shoot.png", {4, 1});
    magic_ = assets.get_texture("battle/magic.png", {4, 1});

    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            counts_[i][j].set_font(font);
        }
    }
}

void Battle::draw(bty::Gfx &gfx, glm::mat4 &)
{
    gfx.draw_sprite(bg_, camera_);

    hud.draw(gfx, camera_);

    gfx.draw_sprite(current_, camera_);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            gfx.draw_sprite(sprites_[i][j], camera_);
        }
    }

    gfx.draw_sprite(cursor_, camera_);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            gfx.draw_text(counts_[i][j], camera_);
        }
    }

    if (draw_hit_marker) {
        gfx.draw_sprite(hit_marker_, camera_);
    }
}

void Battle::key(int key, int action)
{
    if (action != GLFW_PRESS || in_delay) {
        return;
    }

    switch (key) {
        case GLFW_KEY_LEFT:
            ui_move_cursor_dir(0);
            break;
        case GLFW_KEY_RIGHT:
            ui_move_cursor_dir(1);
            break;
        case GLFW_KEY_UP:
            ui_move_cursor_dir(2);
            break;
        case GLFW_KEY_DOWN:
            ui_move_cursor_dir(3);
            break;
        case GLFW_KEY_ENTER:
            ui_confirm();
            break;
        case GLFW_KEY_SPACE:
            pause_show();
            break;
        case GLFW_KEY_V:
            battle_victory();
            break;
        default:
            break;
    }
}

void Battle::update(float dt)
{
    if (draw_hit_marker) {
        hit_marker_.update(dt);
        if (hit_marker_.is_animation_done()) {
            ui_hide_hit_marker();
        }
    }

    if (in_delay) {
        delay_timer_ += dt;
        if (delay_timer_ >= delay_duration_) {
            in_delay = false;
            delay_timer_ = 0;
            if (delay_callback) {
                auto cb = delay_callback;
                delay_callback = nullptr;
                cb();
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            sprites_[i][j].update(dt);
        }
    }

    cursor_.update(dt);
    current_.update(dt);
}

std::string Battle::battle_get_name() const
{
    if (active_.x == -1 || active_.y == -1) {
        return "INVALID";
    }
    return kUnits[armies_[active_.x][active_.y]].name_plural;
}

void Battle::show(std::array<int, 5> &enemy_army, std::array<int, 5> &enemy_counts, bool siege, int castle_id)
{
    hud.set_blank_frame();

    this->enemy_army = &enemy_army;
    this->enemy_counts = &enemy_counts;
    this->siege = siege;
    this->castle_id = castle_id;

    if (siege) {
        bg_.set_texture(siege_bg);

        /* clang-format off */
		terrain = {{
			1, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 0, 1,
			1, 1, 0, 0, 1, 1,
		}};
        /* clang-format on */
    }
    else {
        bg_.set_texture(encounter_bg);

        /* clang-format off */
		terrain = {{
			0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0,
		}};
        /* clang-format on */
    }

    /* Can happen when the previous battle ended in a draw. */
    /* The battle immediately ends and the player wins. */
    battle_check_end();

    delay_timer_ = 0;

    int *armies[] = {v.army.data(), enemy_army.data()};
    int *counts[] = {v.counts.data(), enemy_counts.data()};

    /* Set armies from shared v. */
    /* Initialise states */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            armies_[i][j] = armies[i][j];
            const auto &unit = kUnits[armies[i][j]];
            auto &us = unit_states_[i][j];
            us.id = armies[i][j];
            us.start_count = counts[i][j];
            us.turn_count = counts[i][j];
            us.count = counts[i][j];
            us.hp = unit.hp;
            us.injury = 0;
            us.ammo = unit.initial_ammo;
            us.out_of_control = (us.hp * us.count) > v.leadership;
            us.flying = !!(unit.abilities & AbilityFly);
            us.moves = unit.initial_moves;
            us.waits = 0;
            us.out_of_control = false;
            us.frozen = false;
            us.retaliated = false;
        }
    }

    static const glm::ivec2 kStartingPositions[2][2][5] = {
        {
            // Encounter
            {
                // Team 0
                {0, 0},
                {0, 1},
                {0, 2},
                {0, 3},
                {0, 4},
            },
            {
                // Team 1
                {5, 0},
                {5, 1},
                {5, 2},
                {5, 3},
                {5, 4},
            },
        },
        {
            // Siege
            {
                // Team 0
                {2, 4},
                {3, 4},
                {1, 3},
                {2, 3},
                {3, 3},
            },
            {
                // Team 1
                {1, 0},
                {2, 0},
                {3, 0},
                {4, 0},
                {2, 1},
            },
        },
    };

    int type = static_cast<int>(siege);    // encounter

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }

            /* Set sprites */
            board_move_unit_to(i, j, kStartingPositions[type][i][j].x, kStartingPositions[type][i][j].y);
            sprites_[i][j].set_texture(unit_textures_[armies_[i][j]]);
            if (i == 1) {
                sprites_[i][j].set_flip(true);
            }

            /* Set counts */
            counts_[i][j].set_position(sprites_[i][j].get_position() + glm::vec2(24.0f, 26.0f));
            counts_[i][j].set_string(std::to_string(unit_states_[i][j].count));
        }
    }

    if (battle_check_end()) {
        return;
    }

    active_ = {0, 0};
    battle_set_move_state();
}

void Battle::battle_set_move_state()
{
    auto &unit = battle_get_unit();

    if (unit.flying) {
        ui_set_status(fmt::format("{} fly", battle_get_name()));
        ui_set_cursor_mode(Cursor::Fly);
    }
    else {
        ui_set_status(fmt::format("{} Attack or Move {}", battle_get_name(), unit.moves));
        ui_set_cursor_mode(Cursor::Move);
    }

    current_.set_position(16.0f + unit.x * 48.0f, 24.0f + unit.y * 40.0f);
    ui_set_cursor_position(unit.x, unit.y);

    if (active_.x == 1) {
        current_.set_texture(current_enemy);
    }
    else {
        current_.set_texture(current_friendly);
    }
}

void Battle::board_move_unit_to(int team, int unit, int x, int y)
{
    if (team > 1) {
        spdlog::warn("Battle::board_move_unit_to: Team {} is invalid", team);
        return;
    }
    if (unit < 0 || unit > 4) {
        spdlog::warn("Battle::board_move_unit_to: Unit {} is invalid", unit);
        return;
    }

    float x_ = 16.0f + x * 48.0f;
    float y_ = 24.0f + y * 40.0f;
    sprites_[team][unit].set_position(x_, y_);
    counts_[team][unit].set_position(x_ + 24.0f, y_ + 26.0f);
    unit_states_[team][unit].x = x;
    unit_states_[team][unit].y = y;
    cursor_distance_x_ = 0;
    cursor_distance_y_ = 0;
}

void Battle::ui_move_cursor_dir(int dir)
{
    const auto &unit {battle_get_unit()};

    switch (dir) {
        case 0:    // left
            if (cx_ == 0) {
                return;
            }
            if (!cursor_constrained) {
                cx_--;
            }
            else if (cursor_distance_x_ > -1) {
                cx_--;
                cursor_distance_x_--;
            }
            break;
        case 1:    // right
            if (cx_ == 5) {
                return;
            }
            if (!cursor_constrained) {
                cx_++;
            }
            else if (cursor_distance_x_ < 1) {
                cx_++;
                cursor_distance_x_++;
            }
            break;
        case 2:    // up
            if (cy_ == 0) {
                return;
            }
            if (!cursor_constrained) {
                cy_--;
            }
            else if (cursor_distance_y_ > -1) {
                cy_--;
                cursor_distance_y_--;
            }
            break;
        case 3:    // down
            if (cy_ == 4) {
                return;
            }
            if (!cursor_constrained) {
                cy_++;
            }
            else if (cursor_distance_y_ < 1) {
                cy_++;
                cursor_distance_y_++;
            }
            break;
        default:
            break;
    }

    ui_set_cursor_position(cx_, cy_);

    if (cursor_constrained) {
        auto [unit, enemy] = board_get_unit_at(cx_, cy_);
        (void)unit;
        if (enemy) {
            ui_set_cursor_mode(Cursor::Melee);
        }
        else {
            ui_set_cursor_mode(Cursor::Move);
        }
    }
}

void Battle::ui_confirm()
{
    switch (cursor_mode) {
        case Cursor::Melee:
            [[fallthrough]];
        case Cursor::Move:
            [[fallthrough]];
        case Cursor::Fly:
            battle_do_action({AidTryMove, active_, {cx_, cy_}});
            break;
        case Cursor::Shoot:
            battle_do_action({AidTryShoot, active_, {cx_, cy_}});
            break;
        case Cursor::Magic:
            ui_confirm_spell();
            break;
        default:
            break;
    }
}

void Battle::battle_reset_moves()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }

            const auto &unit = kUnits[armies_[i][j]];
            auto &us = unit_states_[i][j];
            us.turn_count = us.count;
            us.hp = unit.hp;
            us.injury = 0;
            us.out_of_control = (us.hp * us.count) > v.leadership;
            us.flying = (unit.abilities & AbilityFly) && !board_any_enemy_around();
            us.moves = unit.initial_moves;
            us.waits = 0;
            us.retaliated = false;
        }
    }

    used_spell_this_turn_ = false;
}

void Battle::ui_show_hit_marker(int x, int y)
{
    float x_ = 16.0f + x * 48.0f;
    float y_ = 24.0f + y * 40.0f;

    hit_marker_.reset_animation();
    hit_marker_.set_position(x_, y_);

    draw_hit_marker = true;
}

void Battle::ui_hide_hit_marker()
{
    draw_hit_marker = false;
}

int Battle::battle_attack(int from_team, int from_unit, int to_team, int to_unit, bool shoot, bool magic, bool retaliation)
{
    ui_show_hit_marker(unit_states_[to_team][to_unit].x, unit_states_[to_team][to_unit].y);

    int magic_damage = 0;

    if (!magic) {
        battle_get_unit().moves = 0;
    }
    else {
        int spell_power = v.spell_power;

        if (using_spell_ != -1) {
            switch (using_spell_) {
                case 2:
                    magic_damage = 25 * spell_power;
                    break;
                case 3:
                    magic_damage = 10 * spell_power;
                    break;
                case 6:
                    if (kUnits[armies_[to_team][to_unit]].abilities & AbilityUndead) {
                        magic_damage = 50 * spell_power;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    return battle_damage(from_team, from_unit, to_team, to_unit, shoot, magic, magic_damage, retaliation);
}

std::tuple<int, bool> Battle::board_get_unit_at(int x, int y) const
{
    const glm::ivec2 pos {x, y};

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            if (pos == glm::ivec2 {unit_states_[i][j].x, unit_states_[i][j].y}) {
                if (i != active_.x) {
                    return {j, true};
                }
                return {j, false};
            }
        }
    }

    return {-1, false};
}

int units_killed(int dmg, int hp)
{
    return dmg / hp;
}

int damage_remainder(int dmg, int hp)
{
    return dmg % hp;
}

float morale_modifier(int morale)
{
    return morale == 0 ? 1.0f : morale == 1 ? 1.5f
                                            : 0.5f;
}

int Battle::battle_damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation)
{
    bool has_sword = gen.artifacts_found[ArtiSwordOfProwess];
    bool has_shield = gen.artifacts_found[ArtiShieldOfProtection];

    const int unit_id_a = armies_[from_team][from_unit];
    const int unit_id_b = armies_[to_team][to_unit];

    const auto &unit_a {kUnits[unit_id_a]};
    const auto &unit_b {kUnits[unit_id_b]};

    auto &unit_state_a = unit_states_[from_team][from_unit];
    auto &unit_state_b = unit_states_[to_team][to_unit];

    if (!retaliation) {
        unit_state_a.turn_count = unit_state_a.count;
        unit_state_b.turn_count = unit_state_b.count;
    }

    bool cancel_attack = false;

    int scythe_kills = 0;
    int final_damage = 0;
    int dmg = 0;

    if (is_external) {
        //magic-vs-unit
        final_damage = external_damage;
    }
    else {
        //unit-vs-unit
        if (unit_a.abilities & AbilityScythe) {
            if (((rand() % 100) + 1) > 89) {    // 10% chance
                                                // ceil
                scythe_kills = (unit_state_b.count + 2 - 1) / 2;
            }
        }

        if (is_ranged) {
            --unit_state_a.ammo;
            if (unit_id_a == UnitId::Druids) {
                if (unit_b.abilities & AbilityImmune)
                    cancel_attack = true;
                else
                    dmg = 10;
            }
            if (unit_id_a == UnitId::Archmages) {
                if (unit_b.abilities & AbilityImmune)
                    cancel_attack = true;
                else
                    dmg = 25;
            }
            else
                dmg = (rand() % unit_a.ranged_damage_max) + unit_a.ranged_damage_min;
        }
        else
            dmg = (rand() % unit_a.melee_damage_max) + unit_a.melee_damage_min;

        int total = dmg * unit_state_a.turn_count;
        int skill_diff = unit_a.skill_level + 5 - unit_b.skill_level;
        final_damage = (total * skill_diff) / 10;

        if (from_team == 0) {
            if (!unit_state_a.out_of_control) {
                final_damage = static_cast<int>(morale_modifier(v.morales[from_unit]) * static_cast<float>(final_damage));
            }
        }

        if (from_team == 0 && has_sword)
            final_damage = static_cast<int>(1.5f * static_cast<float>(final_damage));

        if (to_team == 0 && has_shield) {
            final_damage /= 4;
            final_damage *= 3;
            //almost same as multiplying 0.75, but more brutal, as div by 4 can yield 0
        }
    }

    final_damage += unit_state_b.injury;
    final_damage += unit_state_b.hp * scythe_kills;

    int kills = units_killed(final_damage, unit_state_b.hp);
    int injury = damage_remainder(static_cast<int>(final_damage), unit_state_b.hp);

    unit_state_b.injury = injury;

    if (kills < unit_state_b.count) {
        //stack survives
        unit_state_b.count -= kills;
    }
    else {
        //stack dies
        unit_state_b.count = 0;
        unit_state_b.x = -1;
        unit_state_b.y = -1;
        final_damage = unit_state_b.turn_count * unit_state_b.hp;
    }

    int final_kills = std::min(kills, unit_state_b.turn_count);

    if (from_team == 1) {
        v.followers_killed += final_kills;
    }

    /* Leech and absorb */
    if (!is_external) {
        /* Difference between leech and absorb is, leech can only get back to the original
			count. Absorb has no limit. */
        if (unit_a.abilities & AbilityAbsorb)
            unit_state_a.count += final_kills;

        else if (unit_a.abilities & AbilityLeech) {
            unit_state_a.count += final_kills;
            if (unit_state_a.count > unit_state_a.start_count) {
                unit_state_a.count = unit_state_a.start_count;
                unit_state_a.injury = 0;
            }
        }
    }

    return final_kills;
}

/* It's convenient to do this after any status messages are
displayed which rely on the unit ID. */
void Battle::board_clear_dead_units()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            if (unit_states_[i][j].count == 0) {
                armies_[i][j] = -1;
            }
        }
    }
}

void Battle::ui_update_counts()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }

            counts_[i][j].set_string(std::to_string(unit_states_[i][j].count));
        }
    }
}

void Battle::pause_view_army()
{
    int army[] = {
        armies_[0][0],
        armies_[0][1],
        armies_[0][2],
        armies_[0][3],
        armies_[0][4],
    };
    int counts[] = {
        unit_states_[0][0].count,
        unit_states_[0][1].count,
        unit_states_[0][2].count,
        unit_states_[0][3].count,
        unit_states_[0][4].count,
    };
    s_view_army.update_info(army, counts, v.morales.data(), v.diff);
    ss.push(&s_view_army, nullptr);
}

void Battle::battle_do_action(Action action)
{
    switch (action.id) {
        case AidTryMove:
            afn_try_move(action);
            break;
        case AidMove:
            afn_move(action);
            break;
        case AidMeleeAttack:
            afn_attack(action);
            break;
        case AidShootAttack:
            afn_attack(action);
            break;
        case AidRetaliate:
            afn_retaliate(action);
            break;
        case AidWait:
            afn_wait(action);
            break;
        case AidPass:
            afn_pass(action);
            break;
        case AidTryShoot:
            afn_try_shoot(action);
            break;
        default:
            break;
    }
}

void Battle::afn_wait(Action action)
{
    ui_set_status(fmt::format("{} wait", battle_get_name()));
    unit_states_[action.from.x][action.from.y].waits++;
    battle_delay_then([this]() {
        battle_on_move();
    });
}

void Battle::afn_pass(Action action)
{
    ui_set_status(fmt::format("{} pass", battle_get_name()));
    unit_states_[action.from.x][action.from.y].moves = 0;
    battle_delay_then([this]() {
        battle_on_move();
    });
}

void Battle::afn_try_move(Action action)
{
    /* Tile was not empty. */
    if (terrain[action.to.x + action.to.y * 6] != 0) {
        if (unit_states_[action.from.x][action.from.y].flying) {
            ui_set_status(" You can't land on an occupied area!");
        }
        else {
            ui_set_status(" You can't move to an occupied area!");
        }
    }
    else {
        auto [target, enemy] = board_get_unit_at(action.to.x, action.to.y);

        /* Tile contains a unit. */
        if (target != -1) {
            /* Tile was an enemy unit. */
            if (enemy) {
                battle_do_action({AidMeleeAttack, action.from, {enemy ? !active_.x : active_.x, target}});
            }
            /* Targeted self. */
            else if (target == active_.y) {
                /* Can shoot. */
                if (cursor_mode != Cursor::Shoot && battle_get_unit().ammo > 0 && !board_any_enemy_around()) {
                    ui_set_cursor_mode(Cursor::Shoot);
                    ui_set_status(fmt::format("{} Shoot ({} left)", battle_get_name(), battle_get_unit().ammo));
                }
                /* Already shooting, want to wait. */
                else {
                    battle_do_action({AidWait, action.from});
                }
            }
            /* Tile was terrain or friendly unit. */
            else {
                ui_set_status(" You can't move to an occupied area!");
            }
        }
        else {
            battle_do_action({AidMove, action.from, action.to});
        }
    }
}

void Battle::afn_attack(Action action)
{
    bool shoot = cursor_mode == Cursor::Shoot;
    bool magic = cursor_mode == Cursor::Magic;

    ui_set_cursor_mode(Cursor::None);

    int kills = battle_attack(action.from.x, action.from.y, action.to.x, action.to.y, shoot, magic, false);

    ui_set_status(fmt::format("{} attack {}, {} die", battle_get_name(), kUnits[armies_[action.to.x][action.to.y]].name_plural, kills));
    battle_get_unit().moves = 0;

    battle_delay_then([this, action, shoot, magic]() {
        counts_[action.to.x][action.to.y].set_string(std::to_string(unit_states_[action.to.x][action.to.y].count));
        if (!shoot && !magic && !unit_states_[action.to.x][action.to.y].retaliated) {
            unit_states_[action.to.x][action.to.y].retaliated = true;
            battle_do_action({AidRetaliate, action.to, action.from});
        }
        else {
            board_clear_dead_units();
            battle_on_move();
        }
    });
}

void Battle::battle_delay_then(std::function<void()> callback)
{
    in_delay = true;
    delay_timer_ = 0;
    delay_callback = callback;
}

void Battle::afn_retaliate(Action action)
{
    int kills = battle_attack(action.from.x, action.from.y, action.to.x, action.to.y, false, false, true);
    ui_set_status(fmt::format("{} retaliate, killing {}", kUnits[armies_[action.from.x][action.from.y]].name_plural, kills));

    battle_delay_then([this, action]() {
        counts_[action.to.x][action.to.y].set_string(std::to_string(unit_states_[action.to.x][action.to.y].count));
        board_clear_dead_units();
        battle_on_move();
    });
}

void Battle::battle_switch_team()
{
    active_.x = active_.x == 1 ? 0 : 1;

    int index = 0;

    while (armies_[active_.x][index] == -1 && index < 5) {
        index++;
    }

    if (index == 5) {
        spdlog::warn("No units left. Battle over?");
    }
    else {
        active_.y = index;
        battle_reset_moves();
        if (unit_states_[active_.x][active_.y].frozen && !board_any_enemy_around()) {
            ui_set_status(fmt::format("{} are frozen", battle_get_name()));
            battle_delay_then([this]() {
                battle_on_move();
            });
        }
        else {
            battle_set_move_state();
        }
    }

    if (active_.x == 1) {
        current_.set_texture(current_enemy);
    }
    else {
        current_.set_texture(current_friendly);
    }
}

int Battle::battle_get_next_unit() const
{
    for (int i = 0; i < 5; i++) {
        int idx = (active_.y + i + 1) % 5;
        if (armies_[active_.x][idx] == -1) {
            continue;
        }
        if (unit_states_[active_.x][idx].moves == 0 || unit_states_[active_.x][idx].waits == 2) {
            continue;
        }
        return idx;
    }
    return -1;
}

void Battle::battle_on_move()
{
    int next_unit = battle_get_next_unit();
    if (next_unit == -1) {
        battle_switch_team();
        cursor_distance_x_ = 0;
        cursor_distance_y_ = 0;
    }
    else {
        active_.y = next_unit;
        cursor_distance_x_ = 0;
        cursor_distance_y_ = 0;
    }
    battle_set_move_state();
}

void Battle::afn_move(Action action)
{
    if (unit_states_[action.from.x][action.from.y].flying) {
        unit_states_[action.from.x][action.from.y].flying = false;
        ui_set_cursor_mode(Cursor::Move);
    }
    else {
        unit_states_[action.from.x][action.from.y].moves--;
        if (unit_states_[action.from.x][action.from.y].moves == 0) {
            battle_on_move();
        }
    }

    cursor_distance_x_ = 0;
    cursor_distance_y_ = 0;

    board_move_unit_to(action.from.x, action.from.y, action.to.x, action.to.y);
    battle_set_move_state();
}

void Battle::ui_confirm_menu(int opt)
{
    switch (opt) {
        case 0:
            pause_view_army();
            break;
        case 1:
            s_view_character.update_info(v, gen);
            ss.push(&s_view_character, nullptr);
            break;
        case 2:
            pause_use_magic();
            break;
        case 3:
            battle_do_action({AidPass, active_});
            break;
        case 4:
            battle_do_action({AidWait, active_});
            break;
        case 5:
            s_controls.set_battle(true);
            ss.push(&s_controls, [this](int) {
                delay_duration_ = game_options.combat_delay * 0.24f;
            });
            break;
        case 6:
            pause_give_up();
            break;
        default:
            break;
    }
}

void Battle::ui_confirm_give_up(int opt)
{
    switch (opt) {
        case 0:
            break;
        case 1:
            for (int i = 0; i < 5; i++) {
                (*enemy_army)[i] = armies_[1][i];
                (*enemy_counts)[i] = unit_states_[1][i].count;
            }
            ss.pop(2);
            break;
        default:
            break;
    }
}

void Battle::battle_use_spell(int spell)
{
    using_spell_ = spell - 7;

    ui_set_cursor_mode(Cursor::Magic);

    switch (using_spell_) {
        case 0:    // clone
            ui_set_status("     Select your army to clone");
            break;
        case 1:    // teleport
            ui_set_status("       Select army to teleport.");
            break;
        case 2:    // fireball
            ui_set_status("     Select enemy army to blast.");
            break;
        case 3:    // lightning
            ui_set_status("  Select enemy army to electricute.");
            break;
        case 4:    // freeze
            ui_set_status("     Select enemy army to freeze.");
            break;
        case 5:    // resurrect
            ui_set_status("   Select your army to resurrect.");
            break;
        case 6:    // turn undead
            ui_set_status("     Select enemy army to turn.");
            break;
        default:
            break;
    }

    v.spells[spell - 7]--;

    used_spell_this_turn_ = true;
}

void Battle::ui_confirm_spell()
{
    auto [target, enemy] = board_get_unit_at(cx_, cy_);

    bool is_immune = target == -1 ? false : (armies_[enemy ? 1 : 0][target] == UnitId::Dragons);

    switch (using_spell_) {
        case 0:    // clone
            if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else {
                if (enemy) {
                    ui_set_status("    You must select your own army!");
                }
                else {
                    if (is_immune) {
                        ui_set_status("Clone has no effect on Dragons");
                    }
                    else {
                        spell_clone();
                    }
                }
            }
            break;
        case 1:    // teleport
            if (selecting_teleport_location_) {
                if (target != -1) {
                    ui_set_status("  You must teleport to an empty area!");
                }
                else {
                    spell_teleport();
                }
            }
            else if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else if (is_immune) {
                ui_set_status("Teleport has no effect on Dragons");
            }
            else {
                ui_set_status(" Select new location to teleport army.");
                selecting_teleport_location_ = true;
                cursor_.set_texture(move_);
                teleport_team_ = enemy ? 1 : 0;
                teleport_target_ = target;
            }
            break;
        case 2:    // fireball
            if (target != -1 && !enemy) {
                ui_set_status("   You must select an opposing army!");
            }
            else if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else if (is_immune) {
                ui_set_status("Fireball has no effect on Dragons");
            }
            else {
                battle_do_action({AidMagicAttack, active_, {enemy ? !active_.x : active_.x, target}});
            }
            break;
        case 3:    // lightning
            if (target != -1 && !enemy) {
                ui_set_status("   You must select an opposing army!");
            }
            else if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else if (is_immune) {
                ui_set_status("Lightning has no effect on Dragons");
            }
            else {
                battle_do_action({AidMagicAttack, active_, {enemy ? !active_.x : active_.x, target}});
            }
            break;
        case 4:    // freeze
            if (target != -1 && !enemy) {
                ui_set_status("   You must select an opposing army!");
            }
            else if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else if (is_immune) {
                ui_set_status("Fireball has no effect on Dragons");
            }
            else {
                spell_freeze();
            }
            break;
        case 5:    // resurrect
            if (target != -1 && enemy) {
                ui_set_status("    You must select your own army!");
            }
            else if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else if (is_immune) {
                ui_set_status("Resurrect has no effect on Dragons");
            }
            else {
                spell_resurrect();
            }
            break;
        case 6:    // turn undead
            if (target == -1) {
                ui_set_status("      You must target somebody!");
            }
            else if (is_immune) {
                ui_set_status("Turn has no effect on Dragons");
            }
            else if (!enemy) {
                ui_set_status("   You must select an opposing army!");
            }
            else {
                battle_do_action({AidMagicAttack, active_, {enemy ? !active_.x : active_.x, target}});
            }
            break;
        default:
            break;
    }
}

void Battle::spell_teleport()
{
    selecting_teleport_location_ = false;
    board_move_unit_to(teleport_team_, teleport_target_, cx_, cy_);
    current_.set_position(16.0f + battle_get_unit().x * 48.0f, 24.0f + battle_get_unit().y * 40.0f);
    ui_set_status(fmt::format("{} are teleported", kUnits[armies_[teleport_team_][teleport_target_]].name_plural));
    battle_delay_then([this]() {
        battle_set_move_state();
    });
}

void Battle::spell_clone()
{
    int clone_amount = 10 * v.spell_power;
    auto [unit, enemy] = board_get_unit_at(cx_, cy_);
    unit_states_[0][unit].count += clone_amount;
    ui_set_status(fmt::format("{} {} are cloned", clone_amount, kUnits[armies_[0][unit]].name_plural));
    ui_update_counts();
    battle_delay_then([this]() {
        battle_set_move_state();
    });
}

void Battle::spell_freeze()
{
    auto [unit, enemy] = board_get_unit_at(cx_, cy_);
    unit_states_[1][unit].frozen = true;
    ui_set_status(fmt::format("{} are frozen", kUnits[armies_[1][unit]].name_plural));
    battle_delay_then([this]() {
        battle_set_move_state();
    });
}

void Battle::spell_resurrect()
{
    auto [unit, enemy] = board_get_unit_at(cx_, cy_);
    int num_resurrected = 20 * v.spell_power;
    auto &us = unit_states_[0][unit];
    num_resurrected = std::min(num_resurrected, us.start_count - us.count);
    us.count += num_resurrected;
    v.followers_killed = std::max(0, v.followers_killed - num_resurrected);
    ui_set_status(fmt::format("{} {} are resurrected", num_resurrected, kUnits[armies_[0][unit]].name_plural));
    ui_update_counts();
    battle_delay_then([this]() {
        battle_on_move();
    });
}

void Battle::ui_set_cursor_position(int x, int y)
{
    cx_ = x;
    cy_ = y;
    cursor_.set_position(16.0f + cx_ * 48.0f, 24.0f + cy_ * 40.0f);
}

bool Battle::battle_check_end()
{
    int num_dead[2] = {0, 0};
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                num_dead[i]++;
            }
        }
    }

    if (num_dead[0] == 5) {
        for (int i = 0; i < 5; i++) {
            (*enemy_army)[i] = armies_[1][i];
            (*enemy_counts)[i] = unit_states_[1][i].count;
        }
        if (siege) {
            v.siege = false;
        }
        return true;
    }
    else if (num_dead[1] == 5) {
        for (int i = 0; i < 5; i++) {
            v.army[i] = armies_[0][i];
            v.counts[i] = unit_states_[0][i].count;
        }
        battle_victory();
        return true;
    }

    return false;
}

void Battle::battle_victory()
{
    int gold_total = 0;
    for (int i = 0; i < 5; i++) {
        auto &us = unit_states_[1][i];
        gold_total += us.start_count * kUnits[us.id].weekly_cost * 5;
    }

    int a = gold_total;
    gold_total += bty::random(10);
    gold_total += bty::random(a / 8);
    v.gold += gold_total;

    if (siege) {
        if (gen.castle_occupants[castle_id] != 0x7F) {
            int villain = gen.castle_occupants[castle_id];

            if (v.contract == villain) {
                v.gold += kVillainRewards[v.contract];
                ds.show_dialog({
                    .x = 5,
                    .y = 6,
                    .w = 30,
                    .h = 18,
                    .strings = {
                        {1, 1, fmt::format(kSiegeVictoryMessage, kShortHeroNames[v.hero], bty::number_with_ks(gold_total), kVillains[villain][0], kVillainRewards[villain])},
                    },
                    .callbacks = {
                        .confirm = [this](int) {
                            ss.pop(siege);
                        },
                    },
                });

                gen.villains_captured[v.contract] = true;
                v.contract = 17;
            }
            else {
                ds.show_dialog({
                    .x = 5,
                    .y = 6,
                    .w = 30,
                    .h = 18,
                    .strings = {
                        {1, 1, fmt::format(kSiegeVictoryMessageNoContract, kShortHeroNames[v.hero], bty::number_with_ks(gold_total), kVillains[villain][0])},
                    },
                    .callbacks = {
                        .confirm = [this](int) {
                            ss.pop(siege);
                        },
                    },
                });

                std::vector<int> potential_castles;

                /* Relocate the villain. */
                for (int i = 0; i < 26; i++) {
                    if (kCastleInfo[i].continent != v.continent) {
                        continue;
                    }
                    if (gen.castle_occupants[i] == 0x7F) {
                        potential_castles.push_back(i);
                    }
                }
                int new_castle = potential_castles[rand() % potential_castles.size()];
                gen.castle_occupants[new_castle] = villain;

                /* And their army. */
                gen_villain_army(villain, gen.castle_armies[new_castle], gen.castle_counts[new_castle]);
            }
        }
        else {
            ds.show_dialog({
                .x = 5,
                .y = 10,
                .w = 30,
                .h = 9,
                .strings = {
                    {1, 1, fmt::format(kEncounterVictoryMessage, kShortHeroNames[v.hero], bty::number_with_ks(gold_total))},
                },
                .callbacks = {
                    .confirm = [this](int) {
                        ss.pop(siege);
                    },
                },
            });
        }

        gen.castle_occupants[castle_id] = -1;
    }
    else {
        ds.show_dialog({
            .x = 5,
            .y = 10,
            .w = 30,
            .h = 9,
            .strings = {
                {1, 1, fmt::format(kEncounterVictoryMessage, kShortHeroNames[v.hero], bty::number_with_ks(gold_total))},
            },
            .callbacks = {
                .confirm = [this](int) {
                    ss.pop(siege);
                },
            },
        });
    }
}

bool Battle::board_any_enemy_around() const
{
    return board_any_enemy_around(active_.x, active_.y);
}

bool Battle::board_any_enemy_around(int team, int unit) const
{
    int x = unit_states_[team][unit].x;
    int y = unit_states_[team][unit].y;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            auto [unit, enemy] = board_get_unit_at(x - 1 + i, y - 1 + j);
            if (enemy) {
                return enemy;
            }
        }
    }
    return false;
}

void Battle::pause_show()
{
    ds.show_dialog({
        .x = 8,
        .y = 9,
        .w = 24,
        .h = 12,
        .options = {
            {3, 2, "View your army"},
            {3, 3, "View your character"},
            {3, 4, "Use magic"},
            {3, 5, "Pass"},
            {3, 6, "Wait"},
            {3, 7, "Game controls"},
            {3, 9, "Give up"},
        },
        .callbacks = {
            .confirm = std::bind(&Battle::ui_confirm_menu, this, std::placeholders::_1),
        },
    });
}

void Battle::pause_give_up()
{
    ds.show_dialog({
        .x = 9,
        .y = 10,
        .w = 22,
        .h = 9,
        .strings = {
            {1, 1, R"raw(   Giving up will
 forfeit your armies
and send you back to
      the King.)raw"},
        },
        .options = {
            {4, 6, "Continue battle"},
            {4, 7, "Give up"},
        },
        .callbacks = {
            .confirm = std::bind(&Battle::ui_confirm_give_up, this, std::placeholders::_1),
        },
    });
}

void Battle::pause_use_magic()
{
    if (used_spell_this_turn_) {
        ui_set_status("You may only cast one spell per round!", true);
        return;
    }

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

    auto confirm = no_spells ? std::function<void(int)>(nullptr) : std::bind(&Battle::battle_use_spell, this, std::placeholders::_1);

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

    dialog->set_selection(7);

    auto &opt = dialog->get_options();
    for (int i = 0; i < 7; i++) {
        opt[i].disable();
    }

    if (no_spells) {
        ds.show_dialog({
            .x = 0,
            .y = 0,
            .w = 40,
            .h = 3,
            .strings = {{1, 1, "You have no Combat spell to cast!"}},
        });
        for (int i = 7; i < 14; i++) {
            opt[i].disable();
        }
    }
}

void Battle::ui_set_status(const std::string &msg, bool wait_for_enter)
{
    if (wait_for_enter) {
        hud.set_error(msg);
    }
    else {
        hud.set_title(msg);
    }
}

Battle::UnitState &Battle::battle_get_unit()
{
    return unit_states_[active_.x][active_.y];
}

void Battle::afn_try_shoot(Action action)
{
    auto [unit, enemy] = board_get_unit_at(action.to.x, action.to.y);
    if (unit == -1) {
        ui_set_status("      You must target somebody!");
    }
    else {
        if (enemy) {
            battle_do_action({AidShootAttack, action.from, {!active_.x, unit}});
        }
        else if (unit == active_.y) {
            battle_do_action({AidWait, action.from});
        }
        else {
            ui_set_status("   You must select an opposing army!");
        }
    }
}

void Battle::ui_set_cursor_mode(Cursor c)
{
    cursor_mode = c;
    switch (c) {
        case Cursor::Fly:
            cursor_.set_texture(move_);
            cursor_constrained = false;
            break;
        case Cursor::Move:
            cursor_.set_texture(move_);
            cursor_constrained = true;
            break;
        case Cursor::Melee:
            cursor_.set_texture(melee_);
            cursor_constrained = true;
            break;
        case Cursor::Shoot:
            cursor_.set_texture(shoot_);
            cursor_constrained = false;
            break;
        case Cursor::Magic:
            cursor_.set_texture(magic_);
            cursor_constrained = false;
            break;
        default:
            cursor_.set_texture(nullptr);
            break;
    }
}
