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
#include "game/gen-variables.hpp"
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

enum StatusId {
    ATTACK_MOVE,
    ATTACK_SHOOT_MOVE,
    SHOOT,
    CANT_ATTACK_FRIENDLY,
    WAIT,
    FLY,
    ERR_FLY_OCCUPIED,
    ERR_MOVE_OCCUPIED,
    ATTACK,
    RETALIATION,
    NO_COMBAT_SPELL,
    NEED_TARGET,
    NEED_ENEMY,
    SELECT_LIGHTNING,
    LIGHTNING_KILLS,
    INVALID_SPELL_TARGET,
    SELECT_FIREBALL,
    FIREBALL_KILLS,
    SELECT_TURN_UNDEAD,
    TURN_UNDEAD_NO_EFFECT,
    TURN_UNDEAD_KILLS,
    ONE_SPELL_PER_TURN,
    SELECT_TELEPORT,
    SELECT_TELEPORT_LOCATION,
    INVALID_TELEPORT_DESTINATION,
    TELEPORT_USED,
    CLONE_SELECT,
    CLONE_MUST_SELECT_FRIENDLY,
    CLONE_USED,
    FREEZE_SELECT,
    FREEZE_USED,
    RESURRECT_SELECT,
    RESURRECT_USED,
    DRAGON_IMMUNE_CLONE,
    DRAGON_IMMUNE_TELEPORT,
    DRAGON_IMMUNE_FIREBALL,
    DRAGON_IMMUNE_LIGHTNING,
    DRAGON_IMMUNE_FREEZE,
    DRAGON_IMMUNE_RESURRECT,
    DRAGON_IMMUNE_TURN_UNDEAD,
    OUT_OF_CONTROL,
};

static constexpr char const *kStatuses[] = {
    "{} Attack or Move {}",
    "{} Attack, Shoot or Move {}",
    "{} Shoot ({} left)",
    "   You can't attack your own army!",
    "{} wait",
    "{} fly",
    " You can't land on an occupied area!",
    " You can't move to an occupied area!",
    "{} attack {}, {} die",
    "{} retaliate, killing {}",
    "   You have no Combat spell to cast!",
    "      You must target somebody!",
    "   You must select an opposing army!",
    "  Select enemy army to electricute.",
    "Lightning kills {} {}",
    "   You must select an opposing army!",
    "     Select enemy army to blast.",
    "Fireball kills {} {}",
    "     Select enemy army to turn.",
    "Turn has no effect on {}",
    "Turn undead kills {}",
    "You may only cast one spell per round!",
    "       Select army to teleport.",
    " Select new location to teleport army.",
    "  You must teleport to an empty area!",
    "{} are teleported",
    "     Select your army to clone",
    "    You must select your own army!",
    "{} {} are cloned",
    "     Select enemy army to freeze.",
    "{} are frozen",
    "   Select your army to resurrect.",
    "{} {} are resurrected",
    "Clone has no effect on Dragons",
    "Teleport has no effect on Dragons",
    "Fireball has no effect on Dragons",
    "Lightning has no effect on Dragons",
    "Freeze has no effect on Dragons",
    "Resurrect has no effect on Dragons",
    "Turn has no effect on Dragons",
    "{} are out of control!",
};

Battle::Battle(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, ViewArmy &view_army_, ViewCharacter &view_character_)
    : ss(ss)
    , ds(ds)
    , v(v)
    , gen(gen)
    , s_view_army(view_army_)
    , s_view_character(view_character_)
{
    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    auto color = bty::BoxColor::Intro;
    auto &font = assets.get_font();

    encounter_bg = assets.get_texture("battle/encounter.png");
    siege_bg = assets.get_texture("battle/siege.png");

    bg_.set_position(8, 24);
    frame_.set_texture(assets.get_texture("frame/game-empty.png"));
    bar_.set_color(color);
    bar_.set_size(304, 9);
    bar_.set_position(8, 7);
    current_.set_texture(assets.get_texture("battle/active-unit.png", {5, 2}));
    status_.set_font(font);
    status_.set_position(8, 8);
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
    bool tmp_msg {false};
    BattleState tmp_state_;
    if (state_ == BattleState::TemporaryMessage) {
        tmp_msg = true;
        tmp_state_ = state_;
        state_ = last_state_;
    }

    gfx.draw_sprite(bg_, camera_);
    gfx.draw_sprite(frame_, camera_);
    gfx.draw_rect(bar_, camera_);
    gfx.draw_text(status_, camera_);

    gfx.draw_sprite(current_, camera_);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            gfx.draw_sprite(sprites_[i][j], camera_);
        }
    }

    if (state_ == BattleState::Moving || state_ == BattleState::Flying || state_ == BattleState::Waiting || state_ == BattleState::Menu || state_ == BattleState::Shooting || state_ == BattleState::Magic || state_ == BattleState::IsFrozen || state_ == BattleState::Delay || state_ == BattleState::Pass) {
        gfx.draw_sprite(cursor_, camera_);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            gfx.draw_text(counts_[i][j], camera_);
        }
    }

    if (state_ == BattleState::Attack || state_ == BattleState::Retaliation) {
        gfx.draw_sprite(hit_marker_, camera_);
    }

    if (tmp_msg) {
        state_ = tmp_state_;
    }
}

void Battle::key(int key, int action)
{
    switch (state_) {
        case BattleState::Magic:
            [[fallthrough]];
        case BattleState::Shooting:
            [[fallthrough]];
        case BattleState::Flying:
            [[fallthrough]];
        case BattleState::Moving:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_LEFT:
                            move_cursor(0);
                            break;
                        case GLFW_KEY_RIGHT:
                            move_cursor(1);
                            break;
                        case GLFW_KEY_UP:
                            move_cursor(2);
                            break;
                        case GLFW_KEY_DOWN:
                            move_cursor(3);
                            break;
                        case GLFW_KEY_ENTER:
                            confirm();
                            break;
                        case GLFW_KEY_SPACE:
                            pause();
                            break;
                        case GLFW_KEY_V:
                            victory();
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case BattleState::TemporaryMessage:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_SPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            set_state(state_before_menu_);
                            status();
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void Battle::update(float dt)
{
    switch (state_) {
        case BattleState::Waiting:
            delay_timer_ += dt;
            if (delay_timer_ >= delay_duration_) {
                delay_timer_ = 0;
                next_unit();
            }
            break;
        case BattleState::Retaliation:
            [[fallthrough]];
        case BattleState::Attack:
            if (hit_marker_.is_animation_done()) {
                set_state(BattleState::PauseToDisplayDamage);
            }
            else {
                hit_marker_.update(dt);
            }
            break;
        case BattleState::PauseToDisplayDamage:
            delay_timer_ += dt;
            if (delay_timer_ >= delay_duration_) {
                if (using_spell_ != -1) {    // no retaliation on magic
                    using_spell_ = -1;
                    clear_dead_units();
                    update_counts();
                    set_state(state_before_menu_);
                    set_cursor_position(positions_[active_.x][active_.y].x, positions_[active_.x][active_.y].y);
                    update_unit_info();
                }
                else if (was_shooting_) {    // no retaliation on shooting
                    clear_dead_units();
                    update_counts();
                    next_unit();
                    was_shooting_ = false;
                }
                else if (!do_retaliate || retaliated_this_turn_[last_attacked_team_][last_attacked_unit_]) {    // no retaliation on retaliation
                    clear_dead_units();
                    update_counts();
                    next_unit();
                }
                else if (do_retaliate && !retaliated_this_turn_[last_attacked_team_][last_attacked_unit_]) {    // retaliate
                    do_retaliate = false;
                    retaliated_this_turn_[last_attacked_team_][last_attacked_unit_] = true;
                    attack(last_attacked_team_, last_attacked_unit_, last_attacking_team_, last_attacking_unit_);
                    set_state(BattleState::Retaliation);
                    status();
                }
                check_end();
            }
            break;
        case BattleState::Delay:
            delay_timer_ += dt;
            if (delay_timer_ >= delay_duration_) {
                delay_timer_ = 0;
                set_state(state_before_menu_);
                set_cursor_position(positions_[active_.x][active_.y].x, positions_[active_.x][active_.y].y);
            }
            break;
        case BattleState::Pass:
            [[fallthrough]];
        case BattleState::IsFrozen:
            delay_timer_ += dt;
            if (delay_timer_ >= delay_duration_) {
                delay_timer_ = 0;
                next_unit();
            }
            break;
        default:
            break;
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

void Battle::show(std::array<int, 5> &enemy_army, std::array<int, 5> &enemy_counts, bool siege, int castle_id)
{
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
    check_end();

    bar_.set_color(bty::get_box_color(v.diff));

    delay_timer_ = 0;
    last_state_ = BattleState::Moving;
    state_ = BattleState::Moving;
    last_attacking_team_ = -1;
    last_attacking_unit_ = -1;
    last_attacked_team_ = -1;
    last_attacked_unit_ = -1;

    /* Initialise states */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            armies_[i][j] = -1;

            auto &us = unit_states_[i][j];
            us.id = -1;
            us.start_count = 0;
            us.turn_count = 0;
            us.count = 0;
            us.ammo = 0;
            us.hp = 0;
            us.injury = 0;
            us.out_of_control = false;
            us.frozen = false;
        }
    }

    int *armies[] = {v.army.data(), enemy_army.data()};
    int *counts[] = {v.counts.data(), enemy_counts.data()};

    /* Set armies from shared v. */
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
            move_unit_to(i, j, kStartingPositions[type][i][j].x, kStartingPositions[type][i][j].y);
            sprites_[i][j].set_texture(unit_textures_[armies_[i][j]]);
            if (i == 1) {
                sprites_[i][j].set_flip(true);
            }

            /* Set counts */
            counts_[i][j].set_position(sprites_[i][j].get_position() + glm::vec2(24.0f, 26.0f));
            counts_[i][j].set_string(std::to_string(unit_states_[i][j].count));

            /* Set states */
            const auto &unit = kUnits[armies_[i][j]];
            moves_left_[i][j] = unit.initial_moves;
            waits_used_[i][j] = 0;
        }
    }

    if (check_end()) {
        return;
    }

    reset_moves();
    reset_waits();
    active_ = {0, 0};
    update_unit_info();

    cx_ = kStartingPositions[type][0][0].x;
    cy_ = kStartingPositions[type][0][0].y;
    update_cursor();
    update_current();
    status();
}

void Battle::move_unit_to(int team, int unit, int x, int y)
{
    if (team > 1) {
        spdlog::warn("Battle::move_unit_to: Team {} is invalid", team);
        return;
    }
    if (unit < 0 || unit > 4) {
        spdlog::warn("Battle::move_unit_to: Unit {} is invalid", unit);
        return;
    }

    float x_ = 16.0f + x * 48.0f;
    float y_ = 24.0f + y * 40.0f;
    sprites_[team][unit].set_position(x_, y_);
    counts_[team][unit].set_position(x_ + 24.0f, y_ + 26.0f);
    positions_[team][unit] = {x, y};
    cursor_distance_x_ = 0;
    cursor_distance_y_ = 0;
    update_current();
}

void Battle::move_cursor(int dir)
{
    switch (dir) {
        case 0:    // left
            if (cx_ == 0) {
                return;
            }
            if (state_ == BattleState::Flying || state_ == BattleState::Magic || state_ == BattleState::Shooting) {
                cx_--;
            }
            else if (state_ == BattleState::Moving && cursor_distance_x_ > -1) {
                cx_--;
                cursor_distance_x_--;
            }
            break;
        case 1:    // right
            if (cx_ == 5) {
                return;
            }
            if (state_ == BattleState::Flying || state_ == BattleState::Magic || state_ == BattleState::Shooting) {
                cx_++;
            }
            else if (state_ == BattleState::Moving && cursor_distance_x_ < 1) {
                cx_++;
                cursor_distance_x_++;
            }
            break;
        case 2:    // up
            if (cy_ == 0) {
                return;
            }
            if (state_ == BattleState::Flying || state_ == BattleState::Magic || state_ == BattleState::Shooting) {
                cy_--;
            }
            else if (state_ == BattleState::Moving && cursor_distance_y_ > -1) {
                cy_--;
                cursor_distance_y_--;
            }
            break;
        case 3:    // down
            if (cy_ == 4) {
                return;
            }
            if (state_ == BattleState::Flying || state_ == BattleState::Magic || state_ == BattleState::Shooting) {
                cy_++;
            }
            else if (state_ == BattleState::Moving && cursor_distance_y_ < 1) {
                cy_++;
                cursor_distance_y_++;
            }
            break;
        default:
            break;
    }

    update_cursor();

    if (state_ == BattleState::Moving) {
        auto [unit, enemy] = get_unit(cx_, cy_);
        (void)unit;
        if (enemy) {
            cursor_.set_texture(melee_);
        }
        else {
            cursor_.set_texture(move_);
        }
    }
}

void Battle::confirm()
{
    switch (state_) {
        case BattleState::Flying:
            land();
            break;
        case BattleState::Moving:
            move_confirm();
            break;
        case BattleState::Magic:
            magic_confirm();
            break;
        case BattleState::Shooting:
            shoot_confirm();
            break;
        default:
            break;
    }
}

void Battle::land()
{
    auto [unit, enemy] = get_unit(cx_, cy_);
    (void)enemy;

    if (unit != -1 && unit != active_.y) {
        status_.set_string(kStatuses[ERR_FLY_OCCUPIED]);
        return;
    }

    for (int i = 0; i < 30; i++) {
        if (terrain[cx_ + cy_ * 6] != 0) {
            status_.set_string(kStatuses[ERR_FLY_OCCUPIED]);
            return;
        }
    }

    /* Don't count landing in place as flying. */
    if (unit != active_.y) {
        flown_this_turn_[active_.x][active_.y] = true;
    }
    set_state(BattleState::Moving);
    move_unit_to(active_.x, active_.y, cx_, cy_);

    status_.set_string(fmt::format(kStatuses[ATTACK_MOVE], kUnits[armies_[active_.x][active_.y]].name_plural, moves_left_[active_.x][active_.y]));
}

void Battle::move_confirm()
{
    if (cx_ == positions_[active_.x][active_.y].x && cy_ == positions_[active_.x][active_.y].y) {
        if (unit_states_[active_.x][active_.y].ammo == 0 || any_enemy_around()) {
            waits_used_[active_.x][active_.y]++;
            set_state(BattleState::Waiting);
        }
        else {
            set_state(BattleState::Shooting);
        }
        return;
    }

    auto [unit, enemy] = get_unit(cx_, cy_);

    if (enemy) {
        do_retaliate = true;
        set_state(BattleState::Attack);
        return;
    }
    else {
        if (unit != -1) {
            status_.set_string(kStatuses[ERR_MOVE_OCCUPIED]);
            return;
        }
    }

    for (int i = 0; i < 30; i++) {
        if (terrain[cx_ + cy_ * 6] != 0) {
            status_.set_string(kStatuses[ERR_MOVE_OCCUPIED]);
            return;
        }
    }

    move_unit_to(active_.x, active_.y, cx_, cy_);

    moves_left_[active_.x][active_.y]--;

    if (moves_left_[active_.x][active_.y] == 0) {
        next_unit();
    }
    else {
        if (unit_states_[active_.x][active_.y].out_of_control) {
            status_.set_string(fmt::format(kStatuses[OUT_OF_CONTROL], kUnits[armies_[active_.x][active_.y]].name_plural));
        }
        else if (unit_states_[active_.x][active_.y].ammo && !any_enemy_around()) {
            status_.set_string(fmt::format(kStatuses[ATTACK_SHOOT_MOVE], kUnits[armies_[active_.x][active_.y]].name_plural, moves_left_[active_.x][active_.y]));
        }
        else {
            status_.set_string(fmt::format(kStatuses[ATTACK_MOVE], kUnits[armies_[active_.x][active_.y]].name_plural, moves_left_[active_.x][active_.y]));
        }
    }
}

void Battle::next_unit()
{
    if (check_end()) {
        return;
    }

    delay_timer_ = 0;

    bool loop_back_for_waits {false};
    bool next_team {false};

    /* There was a unit waiting */
    for (int i = 0; i < 5; i++) {
        int index = (i + 1 + active_.y) % 5;

        if (armies_[active_.x][index] == -1) {
            continue;
        }

        if (waits_used_[active_.x][index] < 2 && moves_left_[active_.x][index] > 0 && !unit_states_[active_.x][index].frozen) {
            loop_back_for_waits = true;
            active_.y = index;
            break;
        }
    }

    /* Nobody on this team has any moves or waits left */
    if (!loop_back_for_waits) {
        active_.x = active_.x == 1 ? 0 : 1;
        for (int i = 0; i < 5; i++) {
            if (armies_[active_.x][i] == -1) {
                continue;
            }
            active_.y = i;
            if (unit_states_[active_.x][active_.y].frozen && !any_enemy_around()) {
                set_state(BattleState::IsFrozen);
            }
            reset_moves();
            reset_waits();
            next_team = true;
            break;
        }
    }

    /* Next unit in team because somebody hasn't moved or waited yet */
    if (!next_team && !loop_back_for_waits) {
        for (int i = 0; i < 5; i++) {
            if (armies_[active_.x][i] == -1) {
                continue;
            }
            int index = (i + 1 + active_.y) % 5;
            if (waits_used_[active_.x][index] < 2 && moves_left_[active_.x][index] > 0) {
                active_.y = index;
                if (unit_states_[active_.x][active_.y].frozen) {
                    set_state(BattleState::IsFrozen);
                }
                break;
            }
        }
    }

    update_unit_info();
}

void Battle::update_unit_info()
{
    cx_ = positions_[active_.x][active_.y].x;
    cy_ = positions_[active_.x][active_.y].y;
    update_cursor();
    update_current();
    cursor_distance_x_ = 0;
    cursor_distance_y_ = 0;
    const auto &unit = kUnits[armies_[active_.x][active_.y]];
    if ((unit.abilities & AbilityFly) && !flown_this_turn_[active_.x][active_.y]) {
        if (!any_enemy_around()) {
            set_state(BattleState::Flying);
        }
        else {
            set_state(BattleState::Moving);
        }
    }
    else {
        if (unit_states_[active_.x][active_.y].out_of_control) {
            status_.set_string(fmt::format(kStatuses[OUT_OF_CONTROL], kUnits[armies_[active_.x][active_.y]].name_plural));
        }
        else if (unit_states_[active_.x][active_.y].ammo && !any_enemy_around()) {
            status_.set_string(fmt::format(kStatuses[ATTACK_SHOOT_MOVE], kUnits[armies_[active_.x][active_.y]].name_plural, moves_left_[active_.x][active_.y]));
        }
        else {
            status_.set_string(fmt::format(kStatuses[ATTACK_MOVE], kUnits[armies_[active_.x][active_.y]].name_plural, moves_left_[active_.x][active_.y]));
        }
        set_state(BattleState::Moving);
    }
    status();
}

void Battle::status()
{
    const auto &unit = kUnits[armies_[active_.x][active_.y]];
    switch (state_) {
        case BattleState::Moving:
            break;
        case BattleState::Waiting:
            status_wait(unit);
            break;
        case BattleState::Flying:
            status_fly(unit);
            break;
        case BattleState::Attack:
            status_attack(unit);
            break;
        case BattleState::Retaliation:
            status_retaliation(unit);
            break;
        default:
            break;
    }
}

void Battle::status_wait(const Unit &unit)
{
    status_.set_string(fmt::format(kStatuses[WAIT], unit.name_plural));
}

void Battle::status_fly(const Unit &unit)
{
    status_.set_string(fmt::format(kStatuses[FLY], unit.name_plural));
}

void Battle::status_attack(const Unit &unit)
{
    const Unit &target = kUnits[armies_[last_attacked_team_][last_attacked_unit_]];

    if (last_state_ == BattleState::Magic) {
        switch (using_spell_) {
            case 2:
                status_.set_string(fmt::format(kStatuses[FIREBALL_KILLS], last_kills_, target.name_plural));
                break;
            case 3:
                status_.set_string(fmt::format(kStatuses[LIGHTNING_KILLS], last_kills_, target.name_plural));
                break;
            case 6:
                if (!(target.abilities & AbilityUndead)) {
                    status_.set_string(fmt::format(kStatuses[TURN_UNDEAD_NO_EFFECT], target.name_plural));
                }
                else {
                    status_.set_string(fmt::format(kStatuses[TURN_UNDEAD_KILLS], last_kills_, target.name_plural));
                    break;
                }
                break;
            default:
                break;
        }
    }
    else {
        status_.set_string(fmt::format(kStatuses[ATTACK], unit.name_plural, target.name_plural, last_kills_));
    }
}

// FIXME: use the parameter
void Battle::status_retaliation(const Unit &)
{
    const Unit &target = kUnits[armies_[last_attacking_team_][last_attacking_unit_]];
    status_.set_string(fmt::format(kStatuses[RETALIATION], target.name_plural, last_kills_));
}

void Battle::update_cursor()
{
    cursor_.set_position(16.0f + cx_ * 48.0f, 24.0f + cy_ * 40.0f);
}

void Battle::update_current()
{
    current_.set_position(16.0f + positions_[active_.x][active_.y].x * 48.0f, 24.0f + positions_[active_.x][active_.y].y * 40.0f);
}

void Battle::reset_moves()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }

            const auto &unit = kUnits[armies_[i][j]];
            moves_left_[i][j] = unit.initial_moves;
            flown_this_turn_[i][j] = false;
            retaliated_this_turn_[i][j] = false;

            auto &us = unit_states_[i][j];
            us.turn_count = us.count;
            us.hp = unit.hp;
            us.injury = 0;
            us.out_of_control = (us.hp * us.count) > v.leadership;
        }
    }

    used_spell_this_turn_ = false;
}

void Battle::reset_waits()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            waits_used_[i][j] = 0;
        }
    }
}

void Battle::set_state(BattleState state)
{
    auto [unit, enemy] = get_unit(cx_, cy_);

    if (state == BattleState::Menu) {
        state_before_menu_ = state_;
    }

    last_state_ = state_;
    state_ = state;

    switch (state) {
        case BattleState::Waiting:
            status();
            break;
        case BattleState::Moving:
            if (unit_states_[active_.x][active_.y].frozen) {
                set_state(BattleState::IsFrozen);
            }
            cursor_.set_texture(move_);
            status();
            break;
        case BattleState::Flying:
            if (unit_states_[active_.x][active_.y].frozen) {
                set_state(BattleState::IsFrozen);
            }
            cursor_.set_texture(move_);
            status();
            break;
        case BattleState::Attack:
            attack(active_.x, active_.y, active_.x == 1 ? 0 : 1, unit);
            status();
            break;
        case BattleState::Retaliation:
            break;
        case BattleState::Magic:
            cursor_.set_texture(magic_);
            break;
        case BattleState::IsFrozen:
            delay_timer_ = 0;
            status_.set_string(fmt::format(kStatuses[FREEZE_USED], kUnits[armies_[active_.x][active_.y]].name_plural));
            break;
        case BattleState::Shooting:
            cursor_.set_texture(shoot_);
            status_.set_string(fmt::format(kStatuses[SHOOT], kUnits[armies_[active_.x][active_.y]].name_plural, unit_states_[active_.x][active_.y].ammo));
            break;
        default:
            break;
    }
}

void Battle::attack(int from_team, int from_unit, int to_team, int to_unit)
{
    delay_timer_ = 0;

    hit_marker_.reset_animation();
    hit_marker_.set_position(sprites_[to_team][to_unit].get_position());

    last_attacking_team_ = from_team;
    last_attacking_unit_ = from_unit;
    last_attacked_team_ = to_team;
    last_attacked_unit_ = to_unit;

    if (using_spell_ == -1) {
        moves_left_[from_team][from_unit] = 0;
    }

    int spell_damage = 0;
    int spell_power = v.spell_power;

    if (using_spell_ != -1) {
        switch (using_spell_) {
            case 2:
                spell_damage = 25 * spell_power;
                break;
            case 3:
                spell_damage = 10 * spell_power;
                break;
            case 6:
                if (kUnits[armies_[to_team][to_unit]].abilities & AbilityUndead) {
                    spell_damage = 50 * spell_power;
                }
                break;
            default:
                break;
        }
    }

    damage(from_team, from_unit, to_team, to_unit, state_ == BattleState::Shooting, using_spell_ != -1, spell_damage, from_team != active_.x);
}

std::tuple<int, bool> Battle::get_unit(int x, int y) const
{
    const glm::ivec2 pos {x, y};

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            if (pos == positions_[i][j]) {
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

void Battle::damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation)
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

    bool retaliate = false;
    if (!retaliation && !is_ranged && !is_external)
        retaliate = true;

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
        positions_[to_team][to_unit] = {-1, -1};
        final_damage = unit_state_b.turn_count * unit_state_b.hp;
    }

    last_kills_ = std::min(kills, unit_state_b.turn_count);

    if (from_team == 1) {
        v.followers_killed += last_kills_;
    }

    /* Leech and absorb */
    if (!is_external) {
        /* Difference between leech and absorb is, leech can only get back to the original
			count. Absorb has no limit. */
        if (unit_a.abilities & AbilityAbsorb)
            unit_state_a.count += last_kills_;

        else if (unit_a.abilities & AbilityLeech) {
            unit_state_a.count += last_kills_;
            if (unit_state_a.count > unit_state_a.start_count) {
                unit_state_a.count = unit_state_a.start_count;
                unit_state_a.injury = 0;
            }
        }
    }
}

/* It's convenient to do this after any status messages are
displayed which rely on the unit ID. */
void Battle::clear_dead_units()
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

void Battle::update_counts()
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

void Battle::view_army()
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

void Battle::menu_confirm(int opt)
{
    switch (opt) {
        case 0:
            view_army();
            break;
        case 1:
            s_view_character.update_info(v, gen);
            ss.push(&s_view_character, nullptr);
            break;
        case 2:
            use_magic();
            break;
        case 3:
            moves_left_[active_.x][active_.y] = 0;
            status_.set_string(fmt::format("{} pass", kUnits[armies_[active_.x][active_.y]].name_plural));
            set_state(BattleState::Pass);
            break;
        case 4:
            waits_used_[active_.x][active_.y]++;
            set_state(BattleState::Waiting);
            break;
        case 5:
            controls();
            break;
        case 6:
            give_up();
            break;
        default:
            break;
    }
}

void Battle::give_up_confirm(int opt)
{
    switch (opt) {
        case 0:
            set_state(state_before_menu_);
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

void Battle::use_spell(int spell)
{
    using_spell_ = spell - 7;

    set_state(BattleState::Magic);

    switch (using_spell_) {
        case 0:    // clone
            status_.set_string(kStatuses[CLONE_SELECT]);
            break;
        case 1:    // teleport
            status_.set_string(kStatuses[SELECT_TELEPORT]);
            break;
        case 2:    // fireball
            status_.set_string(kStatuses[SELECT_FIREBALL]);
            break;
        case 3:    // lightning
            status_.set_string(kStatuses[SELECT_LIGHTNING]);
            break;
        case 4:    // freeze
            status_.set_string(kStatuses[FREEZE_SELECT]);
            break;
        case 5:    // resurrect
            status_.set_string(kStatuses[RESURRECT_SELECT]);
            break;
        case 6:    // turn undead
            status_.set_string(kStatuses[SELECT_TURN_UNDEAD]);
            break;
        default:
            break;
    }

    v.spells[spell - 7]--;

    used_spell_this_turn_ = true;
}

void Battle::magic_confirm()
{
    auto [target, enemy] = get_unit(cx_, cy_);

    bool is_immune = target == -1 ? false : (armies_[enemy ? 1 : 0][target] == UnitId::Dragons);

    switch (using_spell_) {
        case 0:    // clone
            if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else {
                if (enemy) {
                    status_.set_string(kStatuses[CLONE_MUST_SELECT_FRIENDLY]);
                }
                else {
                    if (is_immune) {
                        status_.set_string(kStatuses[DRAGON_IMMUNE_CLONE]);
                    }
                    else {
                        clone();
                        set_state(BattleState::Delay);
                    }
                }
            }
            break;
        case 1:    // teleport
            if (selecting_teleport_location_) {
                if (target != -1) {
                    status_.set_string(kStatuses[INVALID_TELEPORT_DESTINATION]);
                }
                else {
                    teleport();
                    set_state(BattleState::Delay);
                }
            }
            else if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else if (is_immune) {
                status_.set_string(kStatuses[DRAGON_IMMUNE_TELEPORT]);
            }
            else {
                status_.set_string(kStatuses[SELECT_TELEPORT_LOCATION]);
                selecting_teleport_location_ = true;
                cursor_.set_texture(move_);
                teleport_team_ = enemy ? 1 : 0;
                teleport_target_ = target;
            }
            break;
        case 2:    // fireball
            if (target != -1 && !enemy) {
                status_.set_string(kStatuses[NEED_ENEMY]);
            }
            else if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else if (is_immune) {
                status_.set_string(kStatuses[DRAGON_IMMUNE_FIREBALL]);
            }
            else {
                set_state(BattleState::Attack);
            }
            break;
        case 3:    // lightning
            if (target != -1 && !enemy) {
                status_.set_string(kStatuses[NEED_ENEMY]);
            }
            else if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else if (is_immune) {
                status_.set_string(kStatuses[DRAGON_IMMUNE_LIGHTNING]);
            }
            else {
                set_state(BattleState::Attack);
            }
            break;
        case 4:    // freeze
            if (target != -1 && !enemy) {
                status_.set_string(kStatuses[NEED_ENEMY]);
            }
            else if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else if (is_immune) {
                status_.set_string(kStatuses[DRAGON_IMMUNE_FIREBALL]);
            }
            else {
                freeze();
                set_state(BattleState::Delay);
            }
            break;
        case 5:    // resurrect
            if (target != -1 && enemy) {
                status_.set_string(kStatuses[CLONE_MUST_SELECT_FRIENDLY]);
            }
            else if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else if (is_immune) {
                status_.set_string(kStatuses[DRAGON_IMMUNE_RESURRECT]);
            }
            else {
                resurrect();
                set_state(BattleState::Delay);
            }
            break;
        case 6:    // turn undead
            if (target == -1) {
                status_.set_string(kStatuses[NEED_TARGET]);
            }
            else if (is_immune) {
                status_.set_string(kStatuses[DRAGON_IMMUNE_TURN_UNDEAD]);
            }
            else if (!enemy) {
                status_.set_string(kStatuses[INVALID_SPELL_TARGET]);
            }
            else {
                set_state(BattleState::Attack);
            }
            break;
        default:
            break;
    }
}

void Battle::teleport()
{
    selecting_teleport_location_ = false;
    move_unit_to(teleport_team_, teleport_target_, cx_, cy_);
    status_.set_string(fmt::format(kStatuses[TELEPORT_USED], kUnits[armies_[teleport_team_][teleport_target_]].name_plural));
}

void Battle::clone()
{
    int clone_amount = 10 * v.spell_power;
    auto [unit, enemy] = get_unit(cx_, cy_);
    unit_states_[0][unit].count += clone_amount;
    status_.set_string(fmt::format(kStatuses[CLONE_USED], clone_amount, kUnits[armies_[0][unit]].name_plural));
    update_counts();
}

void Battle::freeze()
{
    auto [unit, enemy] = get_unit(cx_, cy_);
    unit_states_[1][unit].frozen = true;
    status_.set_string(fmt::format(kStatuses[FREEZE_USED], kUnits[armies_[1][unit]].name_plural));
}

void Battle::resurrect()
{
    auto [unit, enemy] = get_unit(cx_, cy_);
    int num_resurrected = 20 * v.spell_power;
    auto &us = unit_states_[0][unit];
    num_resurrected = std::min(num_resurrected, us.start_count - us.count);
    us.count += num_resurrected;
    v.followers_killed = std::max(0, v.followers_killed - num_resurrected);
    status_.set_string(fmt::format(kStatuses[RESURRECT_USED], num_resurrected, kUnits[armies_[0][unit]].name_plural));
    update_counts();
}

void Battle::set_cursor_position(int x, int y)
{
    cx_ = x;
    cy_ = y;
    cursor_distance_x_ = 0;
    cursor_distance_y_ = 0;
    cursor_.set_position(16.0f + cx_ * 48.0f, 24.0f + cy_ * 40.0f);
}

void Battle::shoot_confirm()
{
    auto [unit, enemy] = get_unit(cx_, cy_);

    if (unit == -1) {
        status_.set_string(kStatuses[NEED_TARGET]);
    }
    else {
        if (!enemy) {
            status_.set_string(kStatuses[CANT_ATTACK_FRIENDLY]);
        }
        else {
            was_shooting_ = true;
            set_state(BattleState::Attack);
        }
    }
}

bool Battle::check_end()
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
        victory();
        return true;
    }

    return false;
}

void Battle::victory()
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

bool Battle::any_enemy_around() const
{
    int x = positions_[active_.x][active_.y].x;
    int y = positions_[active_.x][active_.y].y;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            auto [unit, enemy] = get_unit(x - 1 + i, y - 1 + j);
            if (enemy) {
                return enemy;
            }
        }
    }
    return false;
}

void Battle::controls(int selection)
{
    auto *dialog = ds.show_dialog({
        .x = 10,
        .y = 10,
        .w = 20,
        .h = 9,
        .strings = {
            {4, 1, "Game Control"},
            {4, 2, "____________"},
        },
        .options = {
            {4, 4, "Music on"},
            {4, 5, "Sound on"},
            {4, 6, fmt::format("Combat delay {}", delay_)},
        },
        .callbacks = {
            .confirm = [this](int opt) {
                switch (opt) {
                    case 0:
                        break;
                    case 1:
                        break;
                    case 2:
                        delay_ = (delay_ + 1) % 10;
                        controls(opt);
                        break;
                    default:
                        break;
                }
            },
            .left = [this](bty::Dialog &d) {
                delay_ = (delay_ - 1 + 10) % 10;
                d.get_options()[2].set_string(fmt::format("Combat delay {}", delay_));
            },
            .right = [this](bty::Dialog &d) {
                delay_ = (delay_ + 1) % 10;
                d.get_options()[2].set_string(fmt::format("Combat delay {}", delay_));
            },
        },
    });

    dialog->set_selection(selection);

    delay_duration_ = delay_ * 0.24f;
}

void Battle::pause()
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
            .confirm = std::bind(&Battle::menu_confirm, this, std::placeholders::_1),
        },
    });
}

void Battle::give_up()
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
            .confirm = std::bind(&Battle::give_up_confirm, this, std::placeholders::_1),
        },
    });
}

void Battle::use_magic()
{
    if (used_spell_this_turn_) {
        status_.set_string(kStatuses[ONE_SPELL_PER_TURN]);
        set_state(BattleState::TemporaryMessage);
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

    auto confirm = no_spells ? std::function<void(int)>(nullptr) : std::bind(&Battle::use_spell, this, std::placeholders::_1);

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
