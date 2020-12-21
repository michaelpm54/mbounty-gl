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
    AidMagicAttack,
    AidTryMove,
    AidMove,
    AidFly,
    AidWait,
    AidPass,
    AidRetaliate,
    AidTryShoot,
};

struct Action {
    ActionId id;
    glm::ivec2 from;
    glm::ivec2 to;
    std::string fmtstr;
};

class Battle : public bty::Scene {
public:
    Battle(bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, ViewArmy &view_army, ViewCharacter &view_character, GameControls &game_controls, GameOptions &game_options, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

    void show(std::array<int, 5> &enemy_army, std::array<int, 5> &enemy_counts, bool siege, int castle_id);

private:
    enum class BattleState {
        Grounded,
        Waiting,
        Flying,
        Attack,
        PauseToDisplayDamage,
        Retaliation,
        Menu,
        GiveUp,
        ViewArmy,
        ViewCharacter,
        Delay,
        IsFrozen,
        Pass,
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

    void move_unit_to(int team, int unit, int x, int y);
    void confirm();
    void magic_confirm();
    void move_cursor(int dir);
    void status_wait(const Unit &unit);
    void status_fly(const Unit &unit);
    void status_attack(const Unit &unit);
    void status_retaliation(const Unit &unit);
    void reset_moves();
    int attack(int from_team, int from_unit, int to_team, int to_unit, bool shoot, bool magic, bool retaliation);
    std::tuple<int, bool> get_unit(int x, int y) const;
    int damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation);
    void clear_dead_units();
    void update_counts();
    void use_spell(int spell);
    void teleport();
    void clone();
    void freeze();
    void resurrect();
    void set_cursor_position(int x, int y);
    bool check_end();
    void victory();
    bool any_enemy_around() const;
    bool any_enemy_around(int team, int unit) const;

    /* Pause menu */
    void pause();
    void view_army();
    void use_magic();
    void give_up();

    /* Pause menu helpers */
    void menu_confirm(int opt);
    void give_up_confirm(int opt);

    std::string get_name() const;

    int check_waiting() const;
    int get_next_unit() const;
    void next_team();
    void set_move_state();
    UnitState &get_unit();
    void delay_then(std::function<void()> callback);
    void on_move();
    void set_status(const std::string &msg, bool wait_for_enter = false);
    void show_hit_marker(int x, int y);
    void hide_hit_marker();

    enum Cursor {
        Fly,
        Move,
        Melee,
        Shoot,
        Magic,
        None,
    };

    void set_cursor_mode(Cursor cursor);
    void do_action(Action action);

    void afn_try_move(Action action);
    void afn_move(Action action);
    void afn_attack(Action action);
    void afn_retaliate(Action action);
    void afn_wait(Action action);
    void afn_pass(Action action);
    void afn_try_shoot(Action action);

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
    int cx_ {0};
    int cy_ {0};
    glm::ivec2 active_ {0, 0};
    std::array<std::array<int, 5>, 2> armies_;
    std::array<std::array<int, 5>, 2> moves_left_;
    std::array<std::array<int, 5>, 2> waits_used_;
    std::array<std::array<bool, 5>, 2> flown_this_turn_;
    int cursor_distance_x_ {0};
    int cursor_distance_y_ {0};
    const bty::Texture *move_;
    const bty::Texture *melee_;
    const bty::Texture *shoot_;
    const bty::Texture *magic_;
    bty::Sprite hit_marker_;

    int last_attacking_team_ {-1};
    int last_attacking_unit_ {-1};
    int last_attacked_team_ {-1};
    int last_attacked_unit_ {-1};
    int last_kills_ {0};

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

    bool status_changed {false};

    std::function<void()> delay_callback {nullptr};
    bool draw_hit_marker {false};

    const bty::Texture *current_friendly;
    const bty::Texture *current_enemy;
    Cursor cursor_mode {Cursor::Move};

    bool in_delay {false};
    bool cursor_constrained {false};
};

#endif    // BTY_GAME_BATTLE_HPP_
