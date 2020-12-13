#ifndef BTY_GAME_BATTLE_HPP_
#define BTY_GAME_BATTLE_HPP_

#include <array>

#include "game/scene.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "gfx/dialog.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"

namespace bty {
class Assets;
class Gfx;
}    // namespace bty

struct Variables;
struct GenVariables;
struct Unit;
class DialogStack;
class SceneStack;
class ViewArmy;
class ViewCharacter;

class Battle : public Scene {
public:
    Battle(SceneStack &ss, DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, ViewArmy &view_army, ViewCharacter &view_character);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void key(int key, int action) override;
    void update(float dt) override;

    void show(std::array<int, 5> &enemy_army, std::array<int, 5> &enemy_counts, bool siege);

private:
    enum class BattleState {
        Moving,
        Waiting,
        Flying,
        Attack,
        PauseToDisplayDamage,
        Retaliation,
        Menu,
        Shooting,
        Magic,
        GiveUp,
        ViewArmy,
        ViewCharacter,
        TemporaryMessage,
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
        bool out_of_control;
        bool frozen;
    };

    void move_unit_to(int team, int unit, int x, int y);
    void confirm();
    void move_confirm();
    void shoot_confirm();
    void magic_confirm();
    void move_cursor(int dir);
    void status();
    void status_wait(const Unit &unit);
    void status_fly(const Unit &unit);
    void status_attack(const Unit &unit);
    void status_retaliation(const Unit &unit);
    void next_unit();
    void update_cursor();
    void update_current();
    void reset_moves();
    void reset_waits();
    void set_state(BattleState state);
    void land();
    void attack(int from_team, int from_unit, int to_team, int to_unit);
    std::tuple<int, bool> get_unit(int x, int y) const;
    void update_unit_info();
    void damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation);
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

    /* Pause menu */
    void pause();
    void view_army();
    void use_magic();
    void controls(int selection = 0);
    void give_up();

    /* Pause menu helpers */
    void menu_confirm(int opt);
    void give_up_confirm(int opt);

private:
    SceneStack &ss;
    DialogStack &ds;
    Variables &v;
    GenVariables &gen;
    ViewArmy &s_view_army;
    ViewCharacter &s_view_character;
    BattleState state_ {BattleState::Moving};
    BattleState last_state_ {BattleState::Moving};
    glm::mat4 camera_ {1.0f};
    bty::Sprite bg_;
    bty::Sprite frame_;
    bty::Rect bar_;
    bty::Sprite cursor_;
    bty::Sprite current_;
    std::array<std::array<glm::ivec2, 5>, 2> positions_;
    std::array<std::array<bty::Text, 5>, 2> counts_;
    std::array<std::array<bty::Sprite, 5>, 2> sprites_;
    std::array<const bty::Texture *, 25> unit_textures_;
    int cx_ {0};
    int cy_ {0};
    bty::Text status_;
    glm::ivec2 active_ {0, 0};
    std::array<std::array<int, 5>, 2> armies_;
    std::array<std::array<int, 5>, 2> moves_left_;
    std::array<std::array<int, 5>, 2> waits_used_;
    std::array<std::array<bool, 5>, 2> flown_this_turn_;
    std::array<std::array<bool, 5>, 2> retaliated_this_turn_;
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

    BattleState state_before_menu_ {BattleState::Moving};

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
};

#endif    // BTY_GAME_BATTLE_HPP_
