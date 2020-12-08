#include "game/game.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "assets.hpp"
#include "game/shop-info.hpp"
#include "gfx/gfx.hpp"
#include "glfw.hpp"
#include "scene-switcher.hpp"
#include "shared-state.hpp"

Game::Game(bty::SceneSwitcher &scene_switcher)
    : scene_switcher_(&scene_switcher)
{
}

bool Game::load(bty::Assets &assets)
{
    bool success {true};

    camera_ = glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f);

    auto &state = scene_switcher_->state();
    auto color = bty::get_box_color(state.difficulty_level);

    for (int i = 0; i < UnitId::UnitCount; i++) {
        unit_textures_[i] = assets.get_texture(fmt::format("units/{}.png", i), {2, 2});
    }

    hud_.load(assets, state);

    /* Create pause menu */
    pause_menu_.create(
        3,
        7,
        26,
        16,
        color,
        assets);

    pause_menu_.add_option(3, 2, "View your army");
    pause_menu_.add_option(3, 3, "View your character");
    pause_menu_.add_option(3, 4, "Look at continent map");
    pause_menu_.add_option(3, 5, "Use magic");
    pause_menu_.add_option(3, 6, "Contract information");
    pause_menu_.add_option(3, 7, "Wait to end of week");
    pause_menu_.add_option(3, 8, "Look at puzzle pieces");
    pause_menu_.add_option(3, 9, "Search the area");
    pause_menu_.add_option(3, 10, "Dismiss army");
    pause_menu_.add_option(3, 11, "Game controls");
    pause_menu_.add_option(3, 13, "Save game");

    /* Create "Use magic" menu */
    use_magic_.create(
        6,
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

    for (int i = 7; i < 14; i++) {
        use_magic_.set_option_disabled(i, true);
    }

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

    /* Create end-of-week messages */
    astrology_.create(1, 18, 30, 9, color, assets);
    budget_.create(1, 18, 30, 9, color, assets);

    astrology_.add_line(1, 1, "");
    astrology_.add_line(1, 3, "");

    budget_.add_line(1, 1, "");
    budget_.add_line(23, 1, "Budget");
    budget_.add_line(1, 3, "");
    budget_.add_line(1, 4, "");
    budget_.add_line(1, 5, "");
    budget_.add_line(1, 6, "");
    budget_.add_line(1, 7, "");
    budget_.add_line(15, 3, "");
    budget_.add_line(15, 4, "");
    budget_.add_line(15, 5, "");
    budget_.add_line(15, 6, "");
    budget_.add_line(15, 7, "");

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

    dismiss_.create(1, 18, 30, 9, color, assets);
    dismiss_.add_line(5, 1, "Dismiss which army?");

    /* Found a map message */
    found_map_.create(1, 18, 30, 9, color, assets);
    found_map_.add_line(1, 2,
                        R"raw(  Hidden within an ancient
  chest, you find maps and
charts describing passage to)raw");
    found_map_continent_ = found_map_.add_line(10, 6, "");

    found_local_map_.create(1, 18, 30, 9, color, assets);
    found_local_map_.add_line(1, 2, R"raw(  Peering through a magical
orb you are able to view the
 entire continent. Your map
  of this area is complete.)raw");

    sail_dialog_.create(1, 18, 30, 9, color, assets);
    sail_dialog_.add_line(3, 1, "Sail to which continent?");

    town_.load(assets, color, state);
    kings_castle_.load(assets, color, state, hud_);
    shop_.load(assets, color, state, hud_);

    untrained_in_magic_.create(6, 10, 20, 10, color, assets);
    untrained_in_magic_.add_line(1, 1,
                                 R"raw( You haven't been
trained in the art
  of spellcasting
  yet. Visit the
 Archmage Aurange
 in Continentia at
  11,19 for this
     ability.)raw");

    bridge_prompt_.create(1, 20, 30, 7, color, assets);
    bridge_prompt_.add_line(1, 1, " Bridge in which direction?");
    bridge_prompt_.add_line(14, 3, " \x81\n\x84 \x82\n \x83");

    disgrace_.create(1, 18, 30, 9, color, assets);
    disgrace_.add_line(1, 1, R"raw(After being disgraced on the
    field of battle, King
 Maximus summons you to his
  castle. After a lesson in
   tactics, he reluctantly
reissues your commission and
   sends you on your way.)raw");

    join_dialog_.create(1, 18, 30, 9, color, assets);
    join_dialog_.add_line(1, 1, "");    // E.g. "Many Sprites"
    join_dialog_.add_line(3, 3, "with desires of greater\nglory, wish to join you.");
    join_dialog_.add_option(13, 6, "Accept");
    join_dialog_.add_option(13, 7, "Decline");

    join_flee_.create(1, 21, 30, 6, color, assets);
    join_flee_.add_line(1, 1, "");    // E.g. "Many Sprites"
    join_flee_.add_line(3, 3, " flee in terror at the\nsight of your vast army.");

    chest_.create(0, 0, 0, 0, color, assets);

    loaded_ = true;
    return success;
}

void Game::enter(bool reset)
{
    if (reset) {
        setup_game();
    }
    else {
        auto &state = scene_switcher_->state();
        if (state.disgrace) {
            set_state(GameState::Disgrace);
            for (int i = 0; i < 6; i++) {
                mobs_[state.continent][state.enemy_index].army[i] = state.enemy_army[i];
                mobs_[state.continent][state.enemy_index].counts[i] = state.enemy_counts[i];
            }
        }
        else {
            mobs_[state.continent][state.enemy_index].dead = true;
            sort_army(scene_switcher_->state().army, scene_switcher_->state().army_counts, 5);
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
        case GameState::Paused:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            pause_menu_.draw(gfx, camera_);
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
        case GameState::UseMagic:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hud_.draw(gfx, camera_);
            use_magic_.draw(gfx, camera_);
            break;
        case GameState::ViewContract:
            hud_.draw(gfx, camera_);
            view_contract_.draw(gfx, camera_);
            break;
        case GameState::WeekPassed:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            if (week_passed_card_ == WeekPassedCard::Astrology) {
                astrology_.draw(gfx, camera_);
            }
            else {
                budget_.draw(gfx, camera_);
            }
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
        case GameState::Dismiss:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            dismiss_.draw(gfx, camera_);
            break;
        case GameState::FoundSailMap:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            found_map_.draw(gfx, camera_);
            break;
        case GameState::FoundLocalMap:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            found_local_map_.draw(gfx, camera_);
            break;
        case GameState::SailNext:
            map_.draw(game_camera_, continent);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            sail_dialog_.draw(gfx, camera_);
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
        case GameState::UntrainedInMagic:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hud_.draw(gfx, camera_);
            untrained_in_magic_.draw(gfx, camera_);
            break;
        case GameState::Bridge:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            bridge_prompt_.draw(gfx, camera_);
            break;
        case GameState::Disgrace:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            disgrace_.draw(gfx, camera_);
            break;
        case GameState::JoinDialog:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            join_dialog_.draw(gfx, camera_);
            break;
        case GameState::JoinFlee:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            join_flee_.draw(gfx, camera_);
            break;
        case GameState::ChestMessage:
            map_.draw(game_camera_, continent);
            draw_mobs(gfx);
            hero_.draw(gfx, game_camera_);
            hud_.draw(gfx, camera_);
            chest_.draw(gfx, camera_);
            break;
        default:
            break;
    }
    if (tmp_hud) {
        state_ = GameState::HudMessage;
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
                        case GLFW_KEY_SPACE:
                            set_state(GameState::Paused);
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
        case GameState::Paused:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_SPACE:
                            set_state(GameState::Unpaused);
                            break;
                        case GLFW_KEY_UP:
                            pause_menu_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            pause_menu_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            switch (pause_menu_.get_selection()) {
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
                                    set_state(GameState::UseMagic);
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
                                        set_state(GameState::WeekPassed);
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
                                    set_state(GameState::Dismiss);
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
                            spdlog::debug("Enter");
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
        case GameState::Disgrace:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_SPACE:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::JoinFlee:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_SPACE:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            mobs_[state.continent][join_unit_].dead = true;
                            join_unit_ = -1;
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
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
        case GameState::UseMagic:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
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
        case GameState::WeekPassed:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER:
                            if (week_passed_card_ == WeekPassedCard::Budget) {
                                week_passed_card_ = WeekPassedCard::Astrology;
                                set_state(GameState::Unpaused);
                            }
                            else {
                                week_passed_card_ = WeekPassedCard::Budget;
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
        case GameState::Dismiss:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_UP:
                            dismiss_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            dismiss_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            dismiss_slot(dismiss_.get_selection());
                            break;
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                        default:
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
        case GameState::FoundSailMap:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::FoundLocalMap:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            set_state(GameState::ViewContinent);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::ChestMessage:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_UP:
                            chest_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            chest_.next();
                            break;
                        case GLFW_KEY_BACKSPACE:
                            [[fallthrough]];
                        case GLFW_KEY_ENTER:
                            chest_confirm();
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::SailNext:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            sail_to(scene_switcher_->state().continent);
                            break;
                        case GLFW_KEY_ENTER:
                            set_state(GameState::Unpaused);
                            sail_to(sail_dialog_.get_selection());
                            break;
                        case GLFW_KEY_UP:
                            sail_dialog_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            sail_dialog_.next();
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
        case GameState::UntrainedInMagic:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::Bridge:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_ENTER:
                            [[fallthrough]];
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                        case GLFW_KEY_LEFT:
                            bridge(MOVE_FLAGS_LEFT);
                            break;
                        case GLFW_KEY_RIGHT:
                            bridge(MOVE_FLAGS_RIGHT);
                            break;
                        case GLFW_KEY_UP:
                            bridge(MOVE_FLAGS_UP);
                            break;
                        case GLFW_KEY_DOWN:
                            bridge(MOVE_FLAGS_DOWN);
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case GameState::JoinDialog:
            switch (action) {
                case GLFW_PRESS:
                    switch (key) {
                        case GLFW_KEY_BACKSPACE:
                            set_state(GameState::Unpaused);
                            break;
                        case GLFW_KEY_UP:
                            join_dialog_.prev();
                            break;
                        case GLFW_KEY_DOWN:
                            join_dialog_.next();
                            break;
                        case GLFW_KEY_ENTER:
                            join_confirm();
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

    town_.view(town, tile, continent, town_units_[town], town_spells_[town], castle_occupations_[town]);
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
    bool tmp_hud {false};
    if (state_ == GameState::HudMessage) {
        state_ = last_state_;
        tmp_hud = true;
    }

    if (state_ == GameState::Unpaused) {
        if (controls_locked_) {
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
        else if (!tmp_hud) {
            move_flags_ = MOVE_FLAGS_NONE;
            if (scene_switcher_->get_key(GLFW_KEY_LEFT)) {
                move_flags_ |= MOVE_FLAGS_LEFT;
            }
            if (scene_switcher_->get_key(GLFW_KEY_RIGHT)) {
                move_flags_ |= MOVE_FLAGS_RIGHT;
            }
            if (scene_switcher_->get_key(GLFW_KEY_UP)) {
                move_flags_ |= MOVE_FLAGS_UP;
            }
            if (scene_switcher_->get_key(GLFW_KEY_DOWN)) {
                move_flags_ |= MOVE_FLAGS_DOWN;
            }
        }

        if (move_flags_) {
            hero_.set_moving(true);

            if ((move_flags_ & MOVE_FLAGS_LEFT) && !(move_flags_ & MOVE_FLAGS_RIGHT)) {
                hero_.set_flip(true);
            }
            else if ((move_flags_ & MOVE_FLAGS_RIGHT) && !(move_flags_ & MOVE_FLAGS_LEFT)) {
                hero_.set_flip(false);
            }

            glm::vec2 dir {0.0f};

            if (move_flags_ & MOVE_FLAGS_UP)
                dir.y -= 1.0f;
            if (move_flags_ & MOVE_FLAGS_DOWN)
                dir.y += 1.0f;
            if (move_flags_ & MOVE_FLAGS_LEFT)
                dir.x -= 1.0f;
            if (move_flags_ & MOVE_FLAGS_RIGHT)
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
            else if (manifold.out_of_bounds) {
                set_state(GameState::SailNext);
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
        else if (!controls_locked_) {
            hero_.set_moving(false);
        }

        map_.update(dt);
        hero_.animate(dt);
        hud_.update(dt);

        const auto &hero_tile = hero_.get_tile();
        const auto &hero_pos = hero_.get_position();

        int continent = scene_switcher_->state().continent;

        if (timestop_) {
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

            for (auto &mob : mobs_[continent]) {
                mob.entity.animate(dt);
            }
        }
        else {
            clock_ += dt;
            if (clock_ >= 14) {
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
                        set_state(GameState::WeekPassed);
                        weeks_passed_++;
                        end_of_week(false);
                    }
                    hud_.update_state();
                }
            }

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
                                join_unit_ = i;
                                int size = 0;
                                for (int i = 0; i < 5; i++) {
                                    if (scene_switcher_->state().army[i] != -1) {
                                        size++;
                                    }
                                }
                                if (size == 5) {
                                    set_state(GameState::JoinFlee);
                                }
                                else {
                                    set_state(GameState::JoinDialog);
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
    }
    else if (state_ == GameState::Paused || state_ == GameState::ViewContinent || state_ == GameState::UseMagic || state_ == GameState::ViewContract || state_ == GameState::ViewPuzzle || state_ == GameState::Dismiss) {
        hud_.update(dt);
        pause_menu_.animate(dt);
    }
    if (state_ == GameState::ViewArmy) {
        view_army_.update(dt);
    }
    else if (state_ == GameState::ViewContinent) {
        view_continent_.update(dt);
    }
    else if (state_ == GameState::UseMagic) {
        use_magic_.animate(dt);
    }
    else if (state_ == GameState::ViewPuzzle) {
        view_puzzle_.update(dt);
    }
    else if (state_ == GameState::Dismiss) {
        hero_.animate(dt);
        dismiss_.animate(dt);
    }
    else if (state_ == GameState::SailNext) {
        sail_dialog_.animate(dt);
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
    else if (state_ == GameState::JoinDialog) {
        join_dialog_.animate(dt);
    }
    else if (state_ == GameState::ChestMessage) {
        chest_.animate(dt);
    }

    if (tmp_hud) {
        state_ = GameState::HudMessage;
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
    /*
		If there's a low, return it. If it's not low, keep checking
		until we run out of units. At that point, if at any point we hit a normal,
		return the normal. If we didn't hit normal or low, it's high.
	*/
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

    int *army = scene_switcher_->state().army;
    int *army_counts = scene_switcher_->state().army_counts;
    int *army_morales = scene_switcher_->state().army_morales;

    army[index] = id;
    army_counts[index] = count;

    for (int i = 0; i < 5; i++) {
        const auto &unit = kUnits[id];

        if (army_counts[count] * unit.hp > scene_switcher_->state().leadership) {
            army_morales[i] = 3;
            continue;
        }

        for (int j = 0; j < 5; j++) {
            army_morales[i] = check_morale(i, army);
        }
    }
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

void Game::update_spells()
{
    bool no_spells = true;
    int *spells = scene_switcher_->state().spells;

    for (int i = 0; i < 7; i++) {
        use_magic_.set_option_disabled(i, spells[(i + 7) % 14] == 0);
        if (spells[(i + 7) % 14] != 0) {
            no_spells = false;
        }
    }

    if (no_spells) {
        hud_.set_title("You have no Adventuring spell to cast!");
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

void Game::end_of_week(bool search)
{
    std::string week = fmt::format("Week #{}", weeks_passed_);

    int unit = rand() % UnitId::UnitCount;

    const auto &name = kUnits[unit].name_singular;

    astrology_.set_line(0, week);
    astrology_.set_line(1, fmt::format("Astrologers proclaim:\nWeek of the {}\n\nAll {} dwellings are\nrepopulated.", name, name));

    auto &state = scene_switcher_->state();

    std::string on_hand = bty::number_with_ks(state.gold);

    int army_total = 0;
    int commission = state.commission;
    int gold = state.gold;
    int boat = state.boat_rented ? (state.artifacts_found[ArtiAnchorOfAdmirality] ? 100 : 500) : 0;
    int balance = (commission + gold) - boat;

    for (int i = 0; i < 5; i++) {
        if (state.army[i] == -1) {
            budget_.set_line(7 + i, "");
            continue;
        }

        const auto &unit = kUnits[state.army[i]];
        int weekly_cost = unit.weekly_cost * state.army_counts[i];
        if (balance > weekly_cost || balance - weekly_cost == 0) {
            balance -= weekly_cost;
            army_total += weekly_cost;
            std::string cost = bty::number_with_ks(weekly_cost);
            std::string spaces(14 - (cost.size() + unit.name_plural.size()), ' ');
            budget_.set_line(7 + i, fmt::format("{}{}{}", unit.name_plural, spaces, cost));
        }
        else {
            std::string leave = "Leave";
            std::string spaces(14 - (leave.size() + unit.name_plural.size()), ' ');
            budget_.set_line(7 + i, fmt::format("{}{}{}", unit.name_plural, spaces, leave));
            state.army[i] = -1;
            state.army_counts[i] = -1;
        }
    }

    sort_army(scene_switcher_->state().army, scene_switcher_->state().army_counts, 5);

    bool out_of_money = (boat + army_total) > (gold + commission);
    if (!out_of_money) {
        balance = (commission + gold) - (boat - army_total);
    }

    state.gold = balance;

    budget_.set_line(0, week);
    budget_.set_line(2, fmt::format("On Hand: {:>4}", on_hand));
    budget_.set_line(3, fmt::format("Payment: {:>4}", commission));
    budget_.set_line(4, fmt::format("Boat: {:>7}", boat));
    budget_.set_line(5, fmt::format("Army: {:>7}", army_total));
    budget_.set_line(6, fmt::format("Balance: {:>4}", bty::number_with_ks(balance)));

    if ((state.army[0] == -1 || out_of_money) && search) {
        set_state(GameState::Disgrace);
    }
}

void Game::sort_army(int *army, int *counts, int max)
{
    int last_free = -1;
    for (int i = 0; i < max; i++) {
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

    /* Misc */
    controls_locked_ = false;
    control_lock_timer_ = 0;

    for (int i = 0; i < 4; i++) {
        friendlies_[i].clear();
    }

    /* Clear spells */
    for (int i = 0; i < 14; i++) {
        state.spells[i] = 2;
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
    pause_menu_.set_color(color);
    use_magic_.set_color(color);
    view_army_.set_color(color);
    view_character_.set_color(color);
    view_continent_.set_color(color);
    view_contract_.set_color(color);
    astrology_.set_color(color);
    budget_.set_color(color);
    town_.set_color(color);
    sail_dialog_.set_color(color);
    dismiss_.set_color(color);
    kings_castle_.set_color(color);
    join_dialog_.set_color(color);
    join_flee_.set_color(color);
    shop_.set_color(color);
    found_map_.set_color(color);
    found_local_map_.set_color(color);
    chest_.set_color(color);

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
    state.leadership = kRankLeadership[state.hero_id][0];
    state.max_spells = kRankSpells[state.hero_id][0];
    state.spell_power = kRankSpellPower[state.hero_id][0];
    state.followers_killed = 0;

    /* Reset dialogs */
    pause_menu_.set_selection(0);
    use_magic_.set_selection(0);
    dismiss_.set_selection(0);

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

static constexpr int kMaxMobCounts[4][UnitId::UnitCount] = {
    {
        10,
        20,
        10,
        5,
        5,
        5,
        10,
        5,
        0,
        5,
        0,
        4,
        4,
        2,
        0,
        2,
        2,
        2,
        0,
        1,
        1,
        2,
        1,
        1,
        1,
    },
    {
        20,
        50,
        20,
        15,
        10,
        10,
        25,
        15,
        0,
        10,
        0,
        8,
        10,
        4,
        0,
        4,
        4,
        4,
        0,
        3,
        2,
        4,
        2,
        2,
        1,
    },
    {
        50,
        100,
        50,
        30,
        25,
        25,
        50,
        30,
        0,
        25,
        0,
        15,
        20,
        10,
        0,
        8,
        10,
        8,
        0,
        6,
        4,
        10,
        5,
        4,
        1,
    },
    {
        100,
        127,
        100,
        80,
        50,
        75,
        100,
        80,
        0,
        50,
        0,
        30,
        50,
        20,
        0,
        15,
        20,
        15,
        0,
        10,
        8,
        25,
        10,
        8,
        2,
    },
};

int unit_count_gen(int continent, int unit)
{
    int bVar1 = kMaxMobCounts[continent][unit];
    int cVar3 = ((bVar1 >> 3) ? (rand() % (bVar1 >> 3)) : 0);
    int uVar2 = rand() % 2;
    return uVar2 & 0xffffff00 | (unsigned int)(bVar1 + uVar2 + cVar3);
}

static constexpr int kMobRollChances[16] = {
    60,
    20,
    10,
    3,
    90,
    70,
    20,
    6,
    100,
    95,
    50,
    10,
    101,
    100,
    90,
    40,
};

static constexpr int kMobIdRange[16] = {
    Peasants,
    Sprites,
    Orcs,
    Skeletons,
    Wolves,
    Gnomes,
    Dwarves,
    Zombies,
    Nomads,
    Elves,
    Ogres,
    Ghosts,
    Barbarians,
    Trolls,
    Giants,
    Vampires,
};

static constexpr int kMobIdRndHigh[4] = {
    Archmages,
    Druids,
    Dragons,
    Demons,
};

int mob_id_gen(int continent)
{
    int id = rand() % 11;
    int chance = rand() % 100;
    int tries = 0;
    while (true) {
        if (3 < tries) {
            return kMobIdRndHigh[id >> 2];
        }
        if ((chance + 1 & 0xff) < kMobRollChances[continent + tries * 4])
            break;
        tries++;
    }
    return tries * 4 & 0xffffff00 | kMobIdRange[((id >> 2) & 0xff) + tries * 4];
}

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
        std::vector<int> castle_indices;

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
                    int castle = -1;
                    for (int i = 0; i < 26; i++) {
                        if (kCastleInfo[i].continent == continent && kCastleInfo[i].x == x && 63 - kCastleInfo[i].y == y) {
                            castle = i;
                            break;
                        }
                    }
                    if (castle != -1) {
                        castle_indices.push_back(castle);
                        tiles[x + y * 64] = Tile_CastleB;
                    }
                    else {
                        /* Not king's castle */
                        if (continent != 0 && x != 11 && y != 56) {
                            spdlog::warn("({}) Unknown castle at {}, {}", continent, x, 63 - y);
                        }
                        else {
                            tiles[x + y * 64] = Tile_CastleB;
                        }
                    }
                }
                else if (id == Tile_GenMonster) {
                    auto &mob = mobs_[continent][num_mobs++];
                    mob.tile = {x, y};
                    mob.dead = false;

                    for (int i = 0; i < 6; i++) {
                        mob.army[i] = -1;
                        mob.counts[i] = 0;
                    regen:
                        int id = mob_id_gen(continent);
                        for (int j = 0; j < 6; j++) {
                            if (mob.army[j] == id) {
                                goto regen;
                            }
                        }
                        mob.army[i] = id;
                        mob.counts[i] = unit_count_gen(continent, mob.army[i]);
                    }

                    /* Cut 1-2 of them in half, or chop the last two off. */
                    int roll = rand() % 3;
                    if (roll == 0) {
                        int n = rand() % 4;
                        mob.counts[n] /= 2;
                        if (mob.counts[n] == 0) {
                            mob.counts[n] = (rand() % 2) + 1;
                        }

                        /* Cut 1 of them in half, or chop the last one off. */
                        /* Note: modified by me to give a higher chance of chopping one off. */
                        /* I seemed to be getting 5 unit armies too often. */
                        /* was: rand % 3 */
                        int a = rand() % 4;
                        if (a == 0) {
                            n = rand() % 5;
                            mob.counts[n] /= 2;
                            if (mob.counts[n] == 0) {
                                mob.counts[n] = (rand() % 2) + 1;
                            }
                        }
                        /* Chop the last one off. */
                        else {
                            mob.army[4] = -1;
                            mob.counts[4] = 0;
                        }
                    }
                    /* Chop the last two off. */
                    else {
                        mob.army[3] = -1;
                        mob.army[4] = -1;
                        mob.counts[3] = 0;
                        mob.counts[4] = 0;
                    }

                    sort_army(mob.army.data(), mob.counts.data(), 6);

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

        rng_.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
        std::shuffle(std::begin(castle_indices), std::end(castle_indices), rng_);

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

            int id = mob_id_gen(continent);
            int count = unit_count_gen(continent, id);

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

        /* Init castle occupants */
        for (int i = 0; i < kCastlesPerContinent[continent]; i++) {
            int index = kCastleIndices[continent] + i;
            castle_occupations_[index] = {
                index,
                -1,
                {{Peasants, Peasants, Peasants, Peasants, Peasants}},
                {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF}}};
        }

        /* Gen castle occupants */
        int used_castles = 0;
        for (int i = 0; i < kVillainsPerContinent[continent]; i++) {
            int castle = castle_indices[used_castles++];

            CastleOccupation occ;
            occ.index = castle;
            occ.occupier = i + kVillainIndices[continent];

            for (int j = 0; j < 6; j++) {
                occ.army[j] = unit_gen(rng_);
                occ.army_counts[j] = (rand() % 100) + 1;
            }

            castle_occupations_[castle] = occ;
        }

        /* Turn the rest of the RNG tiles into chests */
        for (int i = used_tiles; i < random_tiles.size(); i++) {
            tiles[random_tiles[i].x + random_tiles[i].y * 64] = Tile_Chest;
        }
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
    state.army_counts[0] = 0;
    state.army_counts[0] = 0;
    state.army_counts[0] = 0;
    state.army_counts[0] = 0;

    update_camera();
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
    int opt = dismiss_.get_selection();
    dismiss_.clear_options();

    auto &state = scene_switcher_->state();

    for (int i = 0; i < 5; i++) {
        if (state.army[i] != -1) {
            dismiss_.add_option(11, 3 + i, kUnits[state.army[i]].name_plural);
        }
    }

    if (opt > 0) {
        dismiss_.set_selection(opt - 1);
    }
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
    sort_army(scene_switcher_->state().army, scene_switcher_->state().army_counts, 5);
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
        case GameState::UseMagic:
            update_spells();
            break;
        case GameState::ViewContract:
            view_contract();
            break;
        case GameState::WeekPassed:
            weeks_passed_++;
            end_of_week(false);
            break;
        case GameState::ViewPuzzle:
            view_puzzle();
            break;
        case GameState::Dismiss:
            dismiss();
            hud_.update_state();
            break;
        case GameState::Disgrace:
            disgrace();
            break;
        case GameState::SailNext:
            sail_next();
            break;
        case GameState::KingsCastle:
            kings_castle_.view();
            break;
        case GameState::Shop:
            shop_.view(shops_[scene_switcher_->state().continent][shop_index_]);
            break;
        case GameState::JoinDialog:
            setup_join_dialog();
            break;
        case GameState::JoinFlee:
            setup_join_flee();
            break;
        default:
            break;
    }

    clear_movement();
}

void Game::clear_movement()
{
    move_flags_ = MOVE_FLAGS_NONE;
    hero_.set_moving(false);
}

void Game::sail_next()
{
    sail_dialog_.clear_options();

    auto &state = scene_switcher_->state();

    for (int i = 0; i < 4; i++) {
        if (state.maps_found[i]) {
            sail_dialog_.add_option(10, 3 + i, kContinents[i]);
        }
    }
}

void Game::sail_to(int continent)
{
    if (continent == scene_switcher_->state().continent) {
        auto pos = hero_.get_position();

        auto_move_dir_ = {0, 0};

        if (pos.x <= 0) {
            auto_move_dir_.x = 1;
            hero_.set_flip(false);
        }
        else if (pos.x >= 48 * 64) {
            auto_move_dir_.x = -1;
            hero_.set_flip(true);
        }

        if (pos.y <= 0) {
            auto_move_dir_.y = 1;
        }
        else if (pos.y >= 40 * 64) {
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
            if (castle_occupations_[i].occupier == state.contract) {
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

    bool no_spells {true};
    for (int i = 0; i < 7; i++) {
        if (state.spells[i + 7]) {
            no_spells = false;
            break;
        }
    }
    if (no_spells) {
        return;
    }

    if (0 && !state.magic) {
        set_state(GameState::UntrainedInMagic);
        return;
    }

    assert(spell + 7 > -1 && spell + 7 < 14);
    state.spells[spell + 7]--;

    switch (spell) {
        case 0:
            set_state(GameState::Bridge);
            break;
        case 1:
            timestop_ = true;
            timestop_timer_ = 0;
            timestop_left_ += state.spell_power * 10;
            if (timestop_left_ > 9999) {
                timestop_left_ = 9999;
            }
            hud_.set_timestop(timestop_left_);
            set_state(GameState::Unpaused);
            break;
        default:
            break;
    }
}

void Game::bridge(int direction)
{
    const auto &hero_tile = hero_.get_tile();
    const int continent = scene_switcher_->state().continent;

    if (direction == MOVE_FLAGS_LEFT) {
        auto tile = map_.get_tile(hero_tile.tx - 1, hero_tile.ty, continent);
        if (tile.id != -1 && tile.id >= Tile_WaterIRT && tile.id <= Tile_Water) {
            map_.set_tile(tile, continent, Tile_BridgeHorizontal);
            set_state(GameState::Unpaused);
        }
        else {
            bridge_fail();
        }
    }
    else if (direction == MOVE_FLAGS_RIGHT) {
        auto tile = map_.get_tile(hero_tile.tx + 1, hero_tile.ty, continent);
        if (tile.id != -1 && tile.id >= Tile_WaterIRT && tile.id <= Tile_Water) {
            map_.set_tile(tile, continent, Tile_BridgeHorizontal);
            set_state(GameState::Unpaused);
        }
        else {
            bridge_fail();
        }
    }
    else if (direction == MOVE_FLAGS_UP) {
        auto tile = map_.get_tile(hero_tile.tx, hero_tile.ty - 1, continent);
        if (tile.id != -1 && tile.id >= Tile_WaterIRT && tile.id <= Tile_Water) {
            map_.set_tile(tile, continent, Tile_BridgeVertical);
            set_state(GameState::Unpaused);
        }
        else {
            bridge_fail();
        }
    }
    else if (direction == MOVE_FLAGS_DOWN) {
        auto tile = map_.get_tile(hero_tile.tx, hero_tile.ty + 1, continent);
        if (tile.id != -1 && tile.id >= Tile_WaterIRT && tile.id <= Tile_Water) {
            map_.set_tile(tile, continent, Tile_BridgeVertical);
            set_state(GameState::Unpaused);
        }
        else {
            bridge_fail();
        }
    }
    else {
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

void Game::join_confirm()
{
    auto &state = scene_switcher_->state();

    switch (join_dialog_.get_selection()) {
        case 0:
            add_unit_to_army(mobs_[state.continent][join_unit_].army[0], mobs_[state.continent][join_unit_].counts[0]);
            break;
        default:
            break;
    }

    mobs_[state.continent][join_unit_].dead = true;

    join_unit_ = -1;

    set_state(GameState::Unpaused);
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

void Game::setup_join_dialog()
{
    int continent = scene_switcher_->state().continent;

    join_dialog_.set_line(0, fmt::format("{} {}", get_descriptor(mobs_[continent][join_unit_].counts[0]), kUnits[mobs_[continent][join_unit_].army[0]].name_plural));
}

void Game::setup_join_flee()
{
    int continent = scene_switcher_->state().continent;

    join_flee_.set_line(0, fmt::format("{} {}", get_descriptor(mobs_[continent][join_unit_].counts[0]), kUnits[mobs_[continent][join_unit_].army[0]].name_plural));
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

    if (tile.tx == sail_maps_[state.continent].x && tile.ty == sail_maps_[state.continent].y) {
        state.maps_found[state.continent + 1] = true;
        hud_.set_title("You found a map!");
        found_map_continent_->set_string(kContinents[state.continent + 1]);
        set_state(GameState::FoundSailMap);
    }
    else if (tile.tx == local_maps_[state.continent].x && tile.ty == local_maps_[state.continent].y) {
        state.local_maps_found[state.continent] = true;
        view_continent_fog_ = false;
        set_state(GameState::FoundLocalMap);
    }
    else {
        chest_.clear();
        chest_.clear_options();

        static constexpr int kChestChanceGold[] = {
            61,
            66,
            76,
            71,
        };

        static constexpr int kChestChanceCommission[] = {
            81,
            86,
            86,
            81,
        };

        static constexpr int kChestChanceSpellPower[] = {
            0,
            87,
            88,
            86,
        };

        static constexpr int kChestChanceSpellCapacity[] = {
            86,
            92,
            93,
            91,
        };

        static constexpr int kChestChanceAddSpell[] = {
            101,
            101,
            101,
            101,
        };

        int roll = rand() % 100;
        if (roll < kChestChanceGold[state.continent]) {
            static constexpr int kGoldBase[] = {
                5,
                16,
                21,
                31,
            };

            static constexpr int kGoldExtra[] = {
                0,
                4,
                9,
                19,
            };

            roll = rand() % kGoldBase[state.continent];
            int gold = kGoldExtra[state.continent] + (roll + 1) * 100;
            int leadership = state.artifacts_found[ArtiRingOfHeroism] ? gold / 25 : gold / 50;

            chest_.set_size(30, 9);
            chest_.set_position(1, 18);
            chest_.clear();
            chest_.add_line(1, 1, R"raw(After scouring the area,
you fall upon a hidden
treasure cache. You may:)raw");
            chest_.add_option(3, 4, fmt::format("Take the {} gold.", gold));
            chest_.add_option(3, 5, fmt::format(R"raw(Distribute the gold to the
 peasants, increasing your
 leadership by {}.)raw",
                                                leadership));
            set_state(GameState::ChestMessage);

            chest_gold_ = gold;
            chest_leadership_ = leadership;
        }
        else if (roll < kChestChanceCommission[state.continent]) {
            static constexpr int kCommissionBase[] = {
                41,
                51,
                101,
                45,
            };

            static constexpr int kCommissionExtra[] = {
                9,
                49,
                99,
                199,
            };

            roll = rand() % kCommissionBase[state.continent];
            int commission = kCommissionExtra[state.continent] + roll + 1;
            if (commission > 999) {
                commission = 999;
            }

            chest_.set_size(30, 9);
            chest_.set_position(1, 18);
            chest_.clear();
            chest_.add_line(1, 1, fmt::format(R"raw(  After surveying the area,
   you discover that it is
  rich in mineral deposits.

  The King rewards you for
   your find by increasing
  your weekly income by {}.)raw",
                                              commission));
            set_state(GameState::ChestMessage);
            state.commission += commission;
        }
        else if (roll < kChestChanceSpellPower[state.continent]) {
            chest_.set_size(30, 9);
            chest_.set_position(1, 18);
            chest_.clear();
            chest_.add_line(1, 1, R"raw(  Traversing the area, you
  stumble upon a timeworn
   canister. Curious, you
     unstop the bottle,
 releasing a powerful genie,
    who raises your Spell
   Power by 1 and vanishes.)raw");
            set_state(GameState::ChestMessage);
            state.spell_power++;
        }
        else if (roll < kChestChanceSpellCapacity[state.continent]) {
            static constexpr int kSpellCapacityBase[] = {
                1,
                1,
                2,
                2,
            };

            int capacity = state.artifacts_found[ArtiRingOfHeroism] ? kSpellCapacityBase[state.continent] * 2 : kSpellCapacityBase[state.continent];

            chest_.set_size(30, 9);
            chest_.set_position(1, 18);
            chest_.clear();
            chest_.add_line(1, 1, fmt::format(R"raw(A tribe of nomads greet you
and your army warmly. Their
   shaman, in awe of your
  prowess, teaches you the
secret of his tribe's magic.
Your maximum spell capacity
     is increased by {}.)raw",
                                              capacity));
            set_state(GameState::ChestMessage);

            state.max_spells += capacity;
        }
        else {    // spell
            int amount = (rand() % (state.continent + 1)) + 1;
            int spell = rand() % 14;

            chest_.set_size(30, 9);
            chest_.set_position(1, 18);
            chest_.clear();
            chest_.add_line(1, 1, fmt::format(R"raw(     You have captured a
  mischievous imp which has
    been terrorizing the
   region. In exchange for
  its release, you receive:
  
      {} {} spell{}.)raw",
                                              amount,
                                              kSpellNames[spell],
                                              amount == 1 ? "" : "s"));
            set_state(GameState::ChestMessage);

            state.spells[spell] += amount;
        }
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

void Game::chest_confirm()
{
    if (chest_.get_selection() != -1) {
        auto &state = scene_switcher_->state();
        if (chest_.get_selection() == 0) {
            state.gold += chest_gold_;
        }
        else {
            state.leadership += chest_leadership_;
        }
        chest_gold_ = 0;
        chest_leadership_ = 0;
    }

    set_state(GameState::Unpaused);
}
