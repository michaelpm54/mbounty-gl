#include "battle/battle.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

static constexpr char const *kVictoryMessage = {
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
    ERR_OCCUPIED,
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

Battle::Battle(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Battle::load(bty::Assets &assets)
{
    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    auto &state = scene_switcher_->state();
    auto color = bty::get_box_color(state.difficulty_level);
    auto &font = assets.get_font();

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
    magic_ = assets.get_texture("battle/magic.png", {4, 1});

    menu_.create(8, 9, 24, 12, color, assets);
    menu_.add_option(3, 2, "View your army");
    menu_.add_option(3, 3, "View your character");
    menu_.add_option(3, 4, "Use magic");
    menu_.add_option(3, 5, "Pass");
    menu_.add_option(3, 6, "Wait");
    menu_.add_option(3, 7, "Game controls");
    menu_.add_option(3, 9, "Give up");

    give_up_.create(9, 10, 22, 9, color, assets);
    give_up_.add_line(1, 1, R"raw(   Giving up will
 forfeit your armies
and send you back to
      the King.)raw");
    give_up_.add_option(4, 6, "Continue battle");
    give_up_.add_option(4, 7, "Give up");

    view_army_.load(assets, color);
    view_character_.load(assets, color, state.hero_id);

    /* Create "Use magic" menu */
    use_magic_.create(
        10,
        4,
        20,
        22,
        color,
        assets);

    use_magic_.add_line(1, 1, "Adventuring Spells");
    magic_spells_[0] = use_magic_.add_option(4, 3, "");
    magic_spells_[1] = use_magic_.add_option(4, 4, "");
    magic_spells_[2] = use_magic_.add_option(4, 5, "");
    magic_spells_[3] = use_magic_.add_option(4, 6, "");
    magic_spells_[4] = use_magic_.add_option(4, 7, "");
    magic_spells_[5] = use_magic_.add_option(4, 8, "");
    magic_spells_[6] = use_magic_.add_option(4, 9, "");
    use_magic_.add_line(3, 12, "Combat Spells");
    magic_spells_[7] = use_magic_.add_option(4, 14, "");
    magic_spells_[8] = use_magic_.add_option(4, 15, "");
    magic_spells_[9] = use_magic_.add_option(4, 16, "");
    magic_spells_[10] = use_magic_.add_option(4, 17, "");
    magic_spells_[11] = use_magic_.add_option(4, 18, "");
    magic_spells_[12] = use_magic_.add_option(4, 19, "");
    magic_spells_[13] = use_magic_.add_option(4, 20, "");

    victory_.create(5, 10, 30, 9, color, assets);
    victory_.add_line(1, 1, "");

    controls_.create(10, 10, 20, 9, color, assets);
    controls_.add_line(4, 1, "Game Control");
    controls_.add_line(4, 2, "____________");
    controls_.add_option(4, 4, "Music on");
    controls_.add_option(4, 5, "Sound on");
    controls_.add_option(4, 6, "Combat delay 5");

    for (int i = 0; i < 7; i++) {
        use_magic_.set_option_disabled(i, true);
    }

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
    bool tmp_msg {false};
    BattleState tmp_state_;
    if (state_ == BattleState::TemporaryMessage) {
        tmp_msg = true;
        tmp_state_ = state_;
        state_ = last_state_;
    }

    if (state_ == BattleState::ViewArmy) {
        gfx.draw_sprite(frame_, camera_);
        gfx.draw_rect(bar_, camera_);
        gfx.draw_text(status_, camera_);
        view_army_.draw(gfx, camera_);
        return;
    }
    else if (state_ == BattleState::ViewCharacter) {
        gfx.draw_sprite(frame_, camera_);
        gfx.draw_rect(bar_, camera_);
        gfx.draw_text(status_, camera_);
        view_character_.draw(gfx, camera_);
        return;
    }

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

    if (state_ == BattleState::Moving || state_ == BattleState::Flying || state_ == BattleState::Waiting || state_ == BattleState::Menu || state_ == BattleState::Shooting || state_ == BattleState::Magic || state_ == BattleState::IsFrozen || state_ == BattleState::Delay || state_ == BattleState::Pass || state_ == BattleState::Controls) {
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

    if (state_ == BattleState::Menu) {
        menu_.draw(gfx, camera_);
    }
    else if (state_ == BattleState::GiveUp) {
        give_up_.draw(gfx, camera_);
    }
    else if (state_ == BattleState::UseMagic) {
        use_magic_.draw(gfx, camera_);
    }
    else if (state_ == BattleState::Victory) {
        victory_.draw(gfx, camera_);
    }
    else if (state_ == BattleState::Controls) {
        controls_.draw(gfx, camera_);
    }

    if (tmp_msg) {
        state_ = tmp_state_;
    }
}

void Battle::key(int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    switch (state_) {
        case BattleState::Menu:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_W:
                            for (int i = 0; i < 6; i++) {
                                armies_[1][i] = -1;
                            }
                            if (check_end())
                                return;
                            break;
                        case GLFW_KEY_UP:
                            menu_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            menu_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            menu_confirm();
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(last_state_);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case BattleState::GiveUp:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_UP:
                            give_up_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            give_up_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            give_up_confirm();
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(state_before_menu_);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case BattleState::ViewCharacter:
            [[fallthrough]];
        case BattleState::ViewArmy:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            set_state(state_before_menu_);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case BattleState::UseMagic:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            set_state(state_before_menu_);
                            break;
                        case GLFW_KEY_UP:
                            use_magic_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            use_magic_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            use_spell(use_magic_.get_selection());
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
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
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            set_state(BattleState::Menu);
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
        case BattleState::Victory:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_SPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            scene_switcher_->fade_to(SceneId::Game, false);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case BattleState::Controls:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_UP:
                            controls_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            controls_.next();
                            break;
                        case GLFW_KEY_LEFT:
                            if (controls_.get_selection() == 2) {
                                delay_--;
                                if (delay_ < 0) {
                                    delay_ = 9;
                                }
                                delay_duration_ = delay_ * 0.24f;
                            }
                            controls_.set_option(2, fmt::format("Combat delay {}", delay_));
                            break;
                        case GLFW_KEY_RIGHT:
                            if (controls_.get_selection() == 2) {
                                delay_++;
                                if (delay_ >= 10) {
                                    delay_ = 0;
                                }
                                delay_duration_ = delay_ * 0.24f;
                            }
                            controls_.set_option(2, fmt::format("Combat delay {}", delay_));
                            break;
                        case GLFW_KEY_ENTER:
                            controls_confirm();
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(state_before_menu_);
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

bool Battle::loaded()
{
    return loaded_;
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
                hit_marker_.animate(dt);
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
        case BattleState::Menu:
            menu_.animate(dt);
            break;
        case BattleState::GiveUp:
            give_up_.animate(dt);
            break;
        case BattleState::ViewArmy:
            view_army_.update(dt);
            break;
        case BattleState::UseMagic:
            use_magic_.animate(dt);
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
        case BattleState::Controls:
            controls_.animate(dt);
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

    /* Can happen when the previous battle ended in a draw. */
    /* The battle immediately ends and the player wins. */
    check_end();

    auto &state = scene_switcher_->state();
    auto color = bty::get_box_color(state.difficulty_level);
    bar_.set_color(color);
    menu_.set_color(color);
    give_up_.set_color(color);
    view_army_.set_color(color);
    view_character_.set_color(color);
    use_magic_.set_color(color);
    victory_.set_color(color);
    controls_.set_color(color);

    delay_timer_ = 0;
    last_state_ = BattleState::Moving;
    state_ = BattleState::Moving;
    last_attacking_team_ = -1;
    last_attacking_unit_ = -1;
    last_attacked_team_ = -1;
    last_attacked_unit_ = -1;

    /* Initialise states */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 6; j++) {
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

    int *armies[] = {state.army, state.enemy_army.data()};
    int *counts[] = {state.army_counts, state.enemy_counts.data()};

    /* Set armies from shared state. */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            /* Hero has no 6th unit. */
            if (i == 0 && j == 5) {
                continue;
            }
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

    if (check_end()) {
        return;
    }

    reset_moves();
    reset_waits();
    active_ = {0, 0};
    update_unit_info();

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
        status_.set_string(kStatuses[ERR_OCCUPIED]);
        return;
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
    for (int i = 0; i < 6; i++) {
        int index = (i + 1 + active_.y) % 6;

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
        for (int i = 0; i < 6; i++) {
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
        for (int i = 0; i < 6; i++) {
            if (armies_[active_.x][i] == -1) {
                continue;
            }
            int index = (i + 1 + active_.y) % 6;
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
            us.out_of_control = (us.hp * us.count) > leadership;
        }
    }

    used_spell_this_turn_ = false;
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
        case BattleState::ViewArmy:
            view_army();
            break;
        case BattleState::ViewCharacter:
            view_character_.view(scene_switcher_->state());
            break;
        case BattleState::UseMagic:
            update_spells();
            if (used_spell_this_turn_) {
                status_.set_string(kStatuses[ONE_SPELL_PER_TURN]);
                set_state(BattleState::TemporaryMessage);
            }
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
        case BattleState::Victory:
            victory();
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
    int spell_power = scene_switcher_->state().spell_power;

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
        for (int j = 0; j < 6; j++) {
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
                final_damage = static_cast<int>(morale_modifier(scene_switcher_->state().army_morales[from_unit]) * static_cast<float>(final_damage));
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
        scene_switcher_->state().followers_killed += last_kills_;
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

void Battle::menu_confirm()
{
    switch (menu_.get_selection()) {
        case 0:
            set_state(BattleState::ViewArmy);
            break;
        case 1:
            set_state(BattleState::ViewCharacter);
            break;
        case 2:
            set_state(BattleState::UseMagic);
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
            set_state(BattleState::Controls);
            break;
        case 6:
            set_state(BattleState::GiveUp);
            break;
        default:
            break;
    }
}

void Battle::give_up_confirm()
{
    switch (give_up_.get_selection()) {
        case 0:
            set_state(state_before_menu_);
            break;
        case 1:
            scene_switcher_->state().disgrace = true;
            scene_switcher_->fade_to(SceneId::Game, false);
            break;
        default:
            break;
    }
}

void Battle::view_army()
{
    int counts[] = {
        unit_states_[0][0].count,
        unit_states_[0][1].count,
        unit_states_[0][2].count,
        unit_states_[0][3].count,
        unit_states_[0][4].count,
    };
    view_army_.view(&armies_[0][0], counts, scene_switcher_->state().army_morales);
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

    scene_switcher_->state().spells[spell - 7]--;

    used_spell_this_turn_ = true;
}

void Battle::update_spells()
{
    bool no_spells = true;
    int *spells = scene_switcher_->state().spells;

    for (int i = 7; i < 14; i++) {
        use_magic_.set_option_disabled(i, spells[i - 7] == 0);
        if (spells[i - 7] != 0) {
            no_spells = false;
        }
    }

    if (no_spells) {
        status_.set_string(kStatuses[NO_COMBAT_SPELL]);
        set_state(BattleState::TemporaryMessage);
    }

    int n = 0;

    magic_spells_[n]->set_string(fmt::format("{} Bridge", spells[7]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Time Stop", spells[8]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Find Villain", spells[9]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Castle Gate", spells[10]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Town Gate", spells[11]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Instant Army", spells[12]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Raise Control", spells[13]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Clone", spells[0]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Teleport", spells[1]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Fireball", spells[2]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Lightning", spells[3]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Freeze", spells[4]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Resurrect", spells[5]));
    n++;
    magic_spells_[n]->set_string(fmt::format("{} Turn Undead", spells[6]));
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
    int clone_amount = 10 * scene_switcher_->state().spell_power;
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
    int num_resurrected = 20 * scene_switcher_->state().spell_power;
    auto &us = unit_states_[0][unit];
    num_resurrected = std::min(num_resurrected, us.start_count - us.count);
    us.count += num_resurrected;
    scene_switcher_->state().followers_killed = std::max(0, scene_switcher_->state().followers_killed - num_resurrected);
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
        for (int j = 0; j < 6; j++) {
            if (armies_[i][j] == -1) {
                num_dead[i]++;
            }
        }
    }

    auto &state = scene_switcher_->state();

    if (num_dead[0] == 6) {
        state.disgrace = true;
        for (int i = 0; i < 6; i++) {
            state.enemy_army[i] = armies_[1][i];
            state.enemy_counts[i] = unit_states_[1][i].count;
        }
        return true;
    }
    else if (num_dead[1] == 6) {
        for (int i = 0; i < 5; i++) {
            state.army[i] = armies_[0][i];
            state.army_counts[i] = unit_states_[0][i].count;
        }
        set_state(BattleState::Victory);
        return true;
    }

    return false;
}

void Battle::victory()
{
    int gold_total = 0;
    for (int i = 0; i < 6; i++) {
        auto &us = unit_states_[1][i];
        gold_total += us.start_count * kUnits[us.id].weekly_cost * 5;
    }

    int a = gold_total;
    gold_total += bty::random(10);
    gold_total += bty::random(a / 8);

    static constexpr char const *kShortHeroNames[] = {
        "Sir Crimsaun",
        "Lord Palmer",
        "Tynnestra",
        "Moham",
    };

    auto &state = scene_switcher_->state();

    state.gold += gold_total;
    victory_.set_line(0, fmt::format(kVictoryMessage, kShortHeroNames[state.hero_id], bty::number_with_ks(gold_total)));
}

bool Battle::any_enemy_around() const
{
    int x = positions_[active_.x][active_.y].x;
    int y = positions_[active_.x][active_.y].y;
    bool any_enemy_around {false};
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

void Battle::controls_confirm()
{
}
