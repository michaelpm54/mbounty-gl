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
        for (int j = 0; j < army_sizes_[i]; j++) {
            gfx.draw_sprite(sprites_[i][j], camera_);
        }
    }

    if (state_ == BattleState::Moving || state_ == BattleState::Flying || state_ == BattleState::Waiting) {
        gfx.draw_sprite(cursor_, camera_);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
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
                    next_unit();
                }
                else {
                    set_state(BattleState::Retaliation);
                }
            }
            break;
        default:
            break;
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
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

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
            army_counts_[i][j] = 0;
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

    army_sizes_[0] = 0;
    army_sizes_[1] = 0;

    for (int i = 0; i < 6; i++) {
        if (i < 5 && state.army[i] != -1) {
            army_sizes_[0]++;
            armies_[0][i] = state.army[i];
            army_counts_[0][i] = state.army_counts[i];
        }
        if (state.enemy_army[i] != -1) {
            army_sizes_[1]++;
            armies_[1][i] = state.enemy_army[i];
            army_counts_[1][i] = state.enemy_counts[i];
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

    for (int i = 0; i < army_sizes_[0]; i++) {
        move_unit_to(0, i, kStartingPositionX[0][type][i], kStartingPositionY[0][type][i]);
        sprites_[0][i].set_texture(unit_textures_[state.army[i]]);
    }

    for (int i = 0; i < army_sizes_[1]; i++) {
        move_unit_to(1, i, kStartingPositionX[1][type][i], kStartingPositionY[1][type][i]);
        sprites_[1][i].set_texture(unit_textures_[state.enemy_army[i]]);
        sprites_[1][i].set_flip(true);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
            counts_[i][j].set_position(sprites_[i][j].get_position() + glm::vec2(24.0f, 26.0f));
            counts_[i][j].set_string(std::to_string(army_counts_[i][j]));

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
    if (unit > army_sizes_[team]) {
        spdlog::warn("Battle::move_unit_to: No unit {} in team {}", unit, team);
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
    for (int i = 0; i < army_sizes_[active_.x]; i++) {
        int index = (i + 1 + active_.y) % army_sizes_[active_.x];
        if (waits_used_[active_.x][index] < 2 && moves_left_[active_.x][index] > 0) {
            loop_back_for_waits = true;
            active_.y = index;
            break;
        }
    }

    /* Nobody on this team has any moves or waits left */
    if (!loop_back_for_waits) {
        active_.x = (active_.x + 1) % 2;
        for (int i = 0; i < army_sizes_[active_.x]; i++) {
            if (armies_[active_.x][i] != -1) {
                active_.y = i;
                reset_moves();
                reset_waits();
                next_team = true;
                break;
            }
        }
    }

    /* Next unit in team because somebody hasn't moved or waited yet */
    if (!next_team && !loop_back_for_waits) {
        for (int i = 0; i < army_sizes_[active_.x]; i++) {
            int index = (i + 1 + active_.y) % army_sizes_[active_.x];
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
    status_.set_string(fmt::format(kStatuses[ATTACK], unit.name_plural, target.name_plural, 4));
}

void Battle::status_retaliation(const Unit &unit)
{
    const Unit &target = kUnits[armies_[last_attacking_team_][last_attacking_unit_]];
    status_.set_string(fmt::format(kStatuses[RETALIATION], target.name_plural, 7));
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
        for (int j = 0; j < army_sizes_[i]; j++) {
            const auto &unit = kUnits[armies_[i][j]];
            moves_left_[i][j] = unit.initial_moves;
            flown_this_turn_[i][j] = false;
            retaliated_this_turn_[i][j] = false;
        }
    }
}

void Battle::reset_waits()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
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
            if (!retaliated_this_turn_[last_attacked_team_][last_attacked_unit_]) {
                attack(last_attacked_team_, last_attacked_unit_, last_attacking_team_, last_attacking_unit_);
                retaliated_this_turn_[last_attacked_team_][last_attacked_unit_] = true;
                status();
            }
            else {
                next_unit();
            }
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
}

int Battle::get_unit(int x, int y, bool &enemy) const
{
    const glm::ivec2 pos {x, y};

    enemy = false;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < army_sizes_[i]; j++) {
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
