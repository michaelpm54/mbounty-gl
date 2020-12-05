#include "battle/battle.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

enum StatusId {
    ATTACK_MOVE,
    WAIT,
    FLY,
    ERR_OCCUPIED,
    ATTACK,
    RETALIATION,
};

static constexpr char *const kStatuses[] = {
    "{} attack or move {}",
    "{} wait",
    "{} fly",
    " You can't land on an occupied area!",
    "{} attack {}, {} die",
    "{} retaliate, killing {}",
};

Battle::Battle(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Battle::load(bty::Assets &assets)
{
    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    auto &state = scene_switcher_->state();
    auto &font = assets.get_font();
    auto color = bty::get_box_color(state.difficulty_level);

    bg_.set_texture(assets.get_texture("battle/encounter.png"));
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
    magic_ = shoot_;
    cursor_.set_texture(move_);

    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            counts_[i][j].set_font(font);
        }
    }

    loaded_ = true;
    return true;
}

void Battle::draw(bty::Gfx &gfx)
{
    gfx.draw_sprite(bg_, camera_);
    gfx.draw_sprite(frame_, camera_);
    gfx.draw_rect(bar_, camera_);
    gfx.draw_text(status_, camera_);

    gfx.draw_sprite(current_, camera_);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            gfx.draw_sprite(sprites_[i][j], camera_);
        }
    }

    if (state_ == BattleState::Moving || state_ == BattleState::Flying || state_ == BattleState::Waiting) {
        gfx.draw_sprite(cursor_, camera_);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            gfx.draw_text(counts_[i][j], camera_);
        }
    }

    if (state_ == BattleState::Attack || state_ == BattleState::Retaliation) {
        gfx.draw_sprite(hit_marker_, camera_);
    }
}

void Battle::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (action) {
        case GLFW_PRESS:
            switch (state_) {
                case BattleState::Waiting:
                    return;
                case BattleState::PauseToDisplayDamage:
                    return;
                case BattleState::Attack:
                    return;
                case BattleState::Retaliation:
                    return;
                default:
                    break;
            }
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
                case GLFW_KEY_BACKSPACE:
                    scene_switcher_->fade_to(SceneId::Game, false);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

bool Battle::loaded()
{
    return loaded_;
}

void Battle::update(float dt)
{
    switch (state_) {
        case BattleState::Waiting:
            wait_timer_ += dt;
            if (wait_timer_ >= 1.2f) {
                wait_timer_ = 0;
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
                hit_marker_.animate(dt);
            }
            break;
        case BattleState::PauseToDisplayDamage:
            damage_timer_ += dt;
            if (damage_timer_ >= 1.2f) {
                if (last_state_ == BattleState::Retaliation) {
                    clear_dead_units();
                    update_counts();
                    next_unit();
                }
                else {
                    if (!retaliated_this_turn_[last_attacked_team_][last_attacked_unit_]) {
                        retaliated_this_turn_[last_attacked_team_][last_attacked_unit_] = true;
                        attack(last_attacked_team_, last_attacked_unit_, last_attacking_team_, last_attacking_unit_);
                        set_state(BattleState::Retaliation);
                        status();
                    }
                    else {
                        clear_dead_units();
                        update_counts();
                        next_unit();
                    }
                }
            }
            break;
        default:
            break;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            sprites_[i][j].animate(dt);
        }
    }

    cursor_.animate(dt);
    current_.animate(dt);
}

void Battle::enter(bool reset)
{
    if (!reset) {
        return;
    }

    wait_timer_ = 0;
    last_state_ = BattleState::Moving;
    state_ = BattleState::Moving;
    last_attacking_team_ = -1;
    last_attacking_unit_ = -1;
    last_attacked_team_ = -1;
    last_attacked_unit_ = -1;

    auto &state = scene_switcher_->state();

    /* Initialise states */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            armies_[i][j] = -1;

            auto &us = unit_states_[i][j];
            us.start_count = 0;
            us.turn_count = 0;
            us.count = 0;
            us.ammo = 0;
            us.hp = 0;
            us.injury = 0;
            us.out_of_control = false;
        }
    }

    state.army[0] = Demons;
    state.army[1] = Ghosts;
    state.army[2] = Nomads;
    state.army[3] = -1;
    state.army[4] = -1;

    state.army_counts[0] = 50;
    state.army_counts[1] = 40;
    state.army_counts[2] = 30;
    state.army_counts[3] = 20;
    state.army_counts[4] = 10;

    state.enemy_army[0] = Dragons;
    state.enemy_army[1] = Militias;
    state.enemy_army[2] = -1;
    state.enemy_army[3] = -1;
    state.enemy_army[4] = -1;
    state.enemy_army[5] = -1;

    state.enemy_counts[0] = 10;
    state.enemy_counts[1] = 20;
    state.enemy_counts[2] = 30;
    state.enemy_counts[3] = 40;
    state.enemy_counts[4] = 50;
    state.enemy_counts[5] = 60;

    int *armies[] = {state.army, state.enemy_army.data()};
    int *counts[] = {state.army_counts, state.enemy_counts.data()};

    /* Set armies from shared state. */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            /* Hero has no 6th unit. */
            if (i == 0 && j == 5) {
                continue;
            }
            armies_[i][j] = armies[i][j];
            const auto &unit = kUnits[armies[i][j]];
            auto &us = unit_states_[i][j];
            us.start_count = counts[i][j];
            us.turn_count = counts[i][j];
            us.count = counts[i][j];
            us.hp = unit.hp;
            us.injury = 0;
            us.ammo = unit.initial_ammo;
            us.out_of_control = (us.hp * us.count) > state.leadership;
        }
    }

    static constexpr int kStartingPositionY[2][2][6] = {
        {// Team 0
         {
             // Encounter
             0,
             1,
             2,
             3,
             4,
             -1,
         },
         {
             // Siege
             4,
             4,
             3,
             3,
             3,
             -1,
         }},
        {// Team 1
         {
             // Encounter
             0,
             1,
             2,
             3,
             4,
         },
         {
             // Siege
             1,
             1,
             0,
             0,
             0,
             0,
         }},
    };

    static constexpr int kStartingPositionX[2][2][6] = {
        {// Team 0
         {
             // Encounter
             0,
             0,
             0,
             0,
             0,
             -1,
         },
         {
             // Siege
             2,
             3,
             1,
             2,
             3,
             -1,
         }},
        {// Team 1
         {
             // Encounter
             5,
             5,
             5,
             5,
             5,
         },
         {
             // Siege
             2,
             3,
             1,
             2,
             2,
             3,
         }},
    };

    int type = 0;    // encounter

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }

            /* Set sprites */
            move_unit_to(i, j, kStartingPositionX[i][type][j], kStartingPositionY[i][type][j]);
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

    reset_moves();
    reset_waits();
    active_ = {-1, -1};
    next_unit();

    cx_ = kStartingPositionX[0][type][0];
    cy_ = kStartingPositionY[0][type][0];
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
            if (state_ == BattleState::Flying) {
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
            if (state_ == BattleState::Flying) {
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
            if (state_ == BattleState::Flying) {
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
            if (state_ == BattleState::Flying) {
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

    cursor_.set_position(16.0f + cx_ * 48.0f, 24.0f + cy_ * 40.0f);

    if (state_ == BattleState::Moving) {
        bool enemy;
        (void)get_unit(cx_, cy_, enemy);
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
        default:
            break;
    }
}

void Battle::land()
{
    bool enemy;
    int unit = get_unit(cx_, cy_, enemy);
    if (unit != -1 && unit != active_.y) {
        status_.set_string(kStatuses[ERR_OCCUPIED]);
        return;
    }

    /* Don't count landing in place as flying. */
    if (unit != active_.y) {
        flown_this_turn_[active_.x][active_.y] = true;
    }
    set_state(BattleState::Moving);
    move_unit_to(active_.x, active_.y, cx_, cy_);
    status();
}

void Battle::move_confirm()
{
    if (cx_ == positions_[active_.x][active_.y].x && cy_ == positions_[active_.x][active_.y].y) {
        waits_used_[active_.x][active_.y]++;
        set_state(BattleState::Waiting);
        return;
    }

    bool enemy;
    int unit = get_unit(cx_, cy_, enemy);
    if (enemy) {
        set_state(BattleState::Attack);
        return;
    }
    else {
        if (unit != -1) {
            status_.set_string(kStatuses[ERR_OCCUPIED]);
            return;
        }
    }

    move_unit_to(active_.x, active_.y, cx_, cy_);

    moves_left_[active_.x][active_.y]--;
    if (moves_left_[active_.x][active_.y] == 0) {
        next_unit();
    }
    else {
        status();
    }
}

void Battle::next_unit()
{
    damage_timer_ = 0;

    if (active_ == glm::ivec2 {-1, -1}) {
        active_ = {0, 0};
        update_unit_info();
        return;
    }

    bool loop_back_for_waits {false};
    bool next_team {false};

    /* There was a unit waiting */
    for (int i = 0; i < 6; i++) {
        int index = (i + 1 + active_.y) % 6;

        if (armies_[active_.x][index] == -1) {
            continue;
        }

        if (waits_used_[active_.x][index] < 2 && moves_left_[active_.x][index] > 0) {
            loop_back_for_waits = true;
            active_.y = index;
            break;
        }
    }

    /* Nobody on this team has any moves or waits left */
    if (!loop_back_for_waits) {
        active_.x = active_.x == 1 ? 0 : 1;
        for (int i = 0; i < 6; i++) {
            if (armies_[active_.x][i] == -1) {
                continue;
            }
            active_.y = i;
            reset_moves();
            reset_waits();
            next_team = true;
            break;
        }
    }

    /* Next unit in team because somebody hasn't moved or waited yet */
    if (!next_team && !loop_back_for_waits) {
        for (int i = 0; i < 6; i++) {
            if (armies_[active_.x][i] == -1) {
                continue;
            }
            int index = (i + 1 + active_.y) % 6;
            if (waits_used_[active_.x][index] < 2 && moves_left_[active_.x][index] > 0) {
                active_.y = index;
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
        int x = positions_[active_.x][active_.y].x;
        int y = positions_[active_.x][active_.y].y;
        bool any_enemy_around;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int unit = get_unit(x - 1 + i, y - 1 + j, any_enemy_around);
                if (any_enemy_around) {
                    break;
                }
            }
            if (any_enemy_around) {
                break;
            }
        }
        if (!any_enemy_around) {
            set_state(BattleState::Flying);
        }
        else {
            set_state(BattleState::Moving);
        }
    }
    else {
        set_state(BattleState::Moving);
    }
    cursor_.set_texture(move_);
    status();
}

void Battle::status()
{
    const auto &unit = kUnits[armies_[active_.x][active_.y]];
    switch (state_) {
        case BattleState::Moving:
            status_move(unit);
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

void Battle::status_move(const Unit &unit)
{
    status_.set_string(fmt::format(kStatuses[ATTACK_MOVE], unit.name_plural, moves_left_[active_.x][active_.y]));
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
    status_.set_string(fmt::format(kStatuses[ATTACK], unit.name_plural, target.name_plural, last_kills_));
}

void Battle::status_retaliation(const Unit &unit)
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
    const int leadership = scene_switcher_->state().leadership;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
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
            us.ammo = unit.initial_ammo;
            us.out_of_control = (us.hp * us.count) > leadership;
        }
    }
}

void Battle::reset_waits()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            waits_used_[i][j] = 0;
        }
    }
}

void Battle::set_state(BattleState state)
{
    bool enemy;
    int unit = get_unit(cx_, cy_, enemy);

    last_state_ = state_;
    state_ = state;

    switch (state) {
        case BattleState::Waiting:
            status();
            break;
        case BattleState::Moving:
            break;
        case BattleState::Attack:
            attack(active_.x, active_.y, active_.x == 1 ? 0 : 1, unit);
            status();
            break;
        case BattleState::Retaliation:
            break;
        default:
            break;
    }
}

void Battle::attack(int from_team, int from_unit, int to_team, int to_unit)
{
    damage_timer_ = 0;

    hit_marker_.reset_animation();
    hit_marker_.set_position(sprites_[to_team][to_unit].get_position());

    last_attacking_team_ = from_team;
    last_attacking_unit_ = from_unit;
    last_attacked_team_ = to_team;
    last_attacked_unit_ = to_unit;

    moves_left_[from_team][from_unit] = 0;

    damage(from_team, from_unit, to_team, to_unit, false, false, 0, from_team != active_.x);
}

int Battle::get_unit(int x, int y, bool &enemy) const
{
    const glm::ivec2 pos {x, y};

    enemy = false;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }
            if (pos == positions_[i][j]) {
                if (i != active_.x) {
                    enemy = true;
                }
                return j;
            }
        }
    }

    return -1;
}

int units_killed(int dmg, int hp)
{
    return dmg / hp;
}

int damage_remainder(int dmg, int hp)
{
    return dmg % hp;
}

float morale_modifier()
{
    return 1.0f;
}

void Battle::damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation)
{
    bool has_sword = scene_switcher_->state().artifacts_found[ArtiSwordOfProwess];
    bool has_shield = scene_switcher_->state().artifacts_found[ArtiShieldOfProtection];

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
                final_damage = static_cast<int>(morale_modifier() * static_cast<float>(final_damage));
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

    spdlog::debug("Damage: {}/{}", final_damage, unit_state_b.hp);

    int kills = units_killed(final_damage, unit_state_b.hp);
    int injury = damage_remainder(static_cast<int>(final_damage), unit_state_b.hp);

    spdlog::debug("Killed {} and dealt {} injury", kills, injury);

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

    if (from_team == 0) {
        scene_switcher_->state().followers_killed += kills;
    }

    if (!is_external) {
        /* Difference between leech and absorb is, leech can only get back to the original
			count. Absorb has no limit. */
        if (unit_a.abilities & AbilityAbsorb)
            unit_state_a.count += static_cast<int>(kills);

        else if (unit_a.abilities & AbilityLeech) {
            unit_state_a.count += units_killed(final_damage, unit_state_a.hp);
            if (unit_state_a.count > unit_state_a.start_count) {
                unit_state_a.count = unit_state_a.start_count;
                unit_state_a.injury = 0;
            }
        }
    }

    last_kills_ = std::min(kills, unit_state_b.turn_count);
}

/* It's convenient to do this after any status messages are
displayed which rely on the unit ID. */
void Battle::clear_dead_units()
{
    spdlog::debug("Clearing dead units");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
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
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                continue;
            }

            counts_[i][j].set_string(std::to_string(unit_states_[i][j].count));
        }
    }
}
