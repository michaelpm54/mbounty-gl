#ifndef BTY_GAME_BATTLE_HPP_
#define BTY_GAME_BATTLE_HPP_

#include <array>

#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "engine/textbox.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Gfx;
class DialogStack;
class SceneStack;
}    // namespace bty

struct GameOptions;
class GameControls;
struct Variables;
struct GenVariables;
struct Unit;
class ViewArmy;
class ViewCharacter;
class Hud;

enum ActionId {
    AidMeleeAttack,
    AidShootAttack,
    AidTryMove,
    AidMove,
    AidFly,
    AidWait,
    AidPass,
    AidRetaliate,
    AidTryShoot,
    AidSpellClone,
    AidSpellTeleport,
    AidSpellFreeze,
    AidSpellResurrect,
    AidSpellFireball,
    AidSpellLightning,
    AidSpellTurnUndead,
};

struct Action {
    ActionId id;
    glm::ivec2 from;
    glm::ivec2 to;
    std::string fmtstr;
    bool next_unit {true};
};

class Battle : public bty::Scene {
public:
    Battle(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, ViewArmy &view_army, ViewCharacter &view_character, GameControls &game_controls, GameOptions &game_options, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

    void show(std::array<int, 5> &enemy_army, std::array<int, 5> &enemy_counts, bool siege, int castle_id);

private:
    enum class Cursor {
        Fly,
        Move,
        Melee,
        Shoot,
        Magic,
        None,
    };

    struct UnitState {
        int id;
        int start_count;
        int turn_count;
        int count;
        int hp;
        int injury;
        int ammo;
        int moves;
        int waits;
        int x;
        int y;
        bool out_of_control;
        bool frozen;
        bool flying;
        bool retaliated;
    };

    void board_move_unit_to(int team, int unit, int x, int y);
    bool board_any_enemy_around() const;
    bool board_any_enemy_around(int team, int unit) const;
    void board_clear_dead_units();
    std::tuple<int, bool> board_get_unit_at(int x, int y) const;
    bool board_tile_blocked(int x, int y) const;
    bool board_blocked() const;
    bool board_blocked(int team, int unit) const;
    glm::ivec2 board_get_adjacent_tile(int player_unit) const;

    void ui_move_cursor_dir(int dir);
    void ui_show_hit_marker(int x, int y);
    void ui_hide_hit_marker();
    void ui_set_cursor_position(int x, int y);
    void ui_update_counts();
    void ui_confirm();
    void ui_confirm_spell();
    void ui_confirm_menu(int opt);
    void ui_confirm_give_up(int opt);
    void ui_set_status(const std::string &msg, bool wait_for_enter = false);
    void ui_set_cursor_mode(Cursor cursor);
    void ui_update_state();
    void ui_update_cursor();
    void ui_update_status();
    void ui_update_current_unit();
    void ui_update_count(int team, int unit);

    void pause_show();
    void pause_view_army();
    void pause_use_magic();
    void pause_give_up();

    void battle_reset_moves();
    int battle_attack(int from_team, int from_unit, int to_team, int to_unit, bool shoot, bool magic, bool retaliation, int magic_damage);
    int battle_damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation);
    bool battle_check_end();
    void battle_victory();
    void battle_defeat();
    std::string battle_get_name() const;
    int battle_get_next_unit() const;
    void battle_switch_team();
    void battle_set_move_state();
    UnitState &battle_get_unit();
    const UnitState &battle_get_unit() const;
    void battle_delay_then(std::function<void()> callback);
    void battle_on_move();
    void battle_do_action(Action action);
    void battle_use_spell(int spell);
    int battle_get_ranged_unit(int *team = nullptr) const;
    int battle_get_lowest_hp_unit(int *team = nullptr) const;
    int battle_enemy_team() const;
    bool battle_not_user() const;
    bool battle_can_shoot() const;

    void afn_try_move(Action action);
    void afn_move(Action action);
    void afn_melee_attack(Action action);
    void afn_shoot_attack(Action action);
    void afn_magic_attack(Action action);
    void afn_retaliate(Action action);
    void afn_wait(Action action);
    void afn_pass(Action action);
    void afn_try_shoot(Action action);

    void afn_spell_clone(Action action);
    void afn_spell_teleport(Action action);
    void afn_spell_freeze(Action action);
    void afn_spell_resurrect(Action action);
    void afn_spell_fireball(Action action);
    void afn_spell_lightning(Action action);
    void afn_spell_turn_undead(Action action);

    void ai_make_action();

private:
    bty::SceneStack &ss;
    bty::DialogStack &ds;
    Variables &v;
    GenVariables &gen;
    ViewArmy &s_view_army;
    ViewCharacter &s_view_character;
    GameControls &s_controls;
    glm::mat4 camera_ {1.0f};
    bty::Sprite bg_;
    bty::Sprite frame_;
    bty::Rect bar_;
    bty::Sprite cursor_;
    bty::Sprite current_;
    std::array<std::array<bty::Text, 5>, 2> counts_;
    std::array<std::array<bty::Sprite, 5>, 2> sprites_;
    std::array<const bty::Texture *, 25> unit_textures_;
    std::array<const bty::Texture *, 3> obstacle_textures;
    int cx_ {0};
    int cy_ {0};
    glm::ivec2 active_ {0, 0};
    std::array<std::array<int, 5>, 2> armies_;
    int cursor_distance_x_ {0};
    int cursor_distance_y_ {0};
    const bty::Texture *move_;
    const bty::Texture *melee_;
    const bty::Texture *shoot_;
    const bty::Texture *magic_;
    bty::Sprite hit_marker_;
    std::array<std::array<UnitState, 5>, 2> unit_states_;
    int using_spell_ {-1};
    bool used_spell_this_turn_ {false};
    bool selecting_teleport_location_ {false};
    int teleport_target_ {-1};
    int teleport_team_ {-1};
    float delay_timer_ {0};
    bool was_shooting_ {false};
    bool do_retaliate {false};
    int delay_ {1};
    float delay_duration_ {1.2f};
    bool siege {false};
    std::array<int, 5> *enemy_army;
    std::array<int, 5> *enemy_counts;
    const bty::Texture *encounter_bg;
    const bty::Texture *siege_bg;
    std::array<int, 30> terrain;
    int castle_id {-1};
    GameOptions &game_options;
    Hud &hud;
    std::function<void()> delay_callback {nullptr};
    bool draw_hit_marker {false};
    const bty::Texture *current_friendly;
    const bty::Texture *current_enemy;
    const bty::Texture *current_ooc;
    Cursor cursor_mode {Cursor::Move};
    bool in_delay {false};
    bool cursor_constrained {false};
    std::array<bty::Rect, 8> cost_squares;
    std::array<bty::Sprite, 6> terrain_sprites;
    bool is_end {false};
    bty::Font board_font;
};

#endif    // BTY_GAME_BATTLE_HPP_
