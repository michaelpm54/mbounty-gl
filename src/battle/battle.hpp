#ifndef BTY_BATTLE_BATTLE_HPP_
#define BTY_BATTLE_BATTLE_HPP_

#include <array>

#include "gfx/dialog.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "scene.hpp"

namespace bty {
class Assets;
class Gfx;
class SceneSwitcher;
}    // namespace bty

struct Unit;

class Battle : public bty::Scene {
public:
    Battle(bty::SceneSwitcher &scene_switcher);
    bool load(bty::Assets &assets) override;
    void draw(bty::Gfx &gfx) override;
    void key(int key, int scancode, int action, int mods) override;
    bool loaded() override;
    void update(float dt) override;
    void enter(bool reset) override;

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
    };

    struct UnitState {
        int start_count;
        int turn_count;
        int count;
        int hp;
        int injury;
        int ammo;
        bool out_of_control;
    };

    void move_unit_to(int team, int unit, int x, int y);
    void confirm();
    void move_confirm();
    void move_cursor(int dir);
    void status();
    void status_move(const Unit &unit);
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
    int get_unit(int x, int y, bool &enemy) const;
    void update_unit_info();
    void damage(int from_team, int from_unit, int to_team, int to_unit, bool is_ranged, bool is_external, int external_damage, bool retaliation);
    void clear_dead_units();
    void update_counts();

private:
    bool loaded_ {false};
    bty::SceneSwitcher *scene_switcher_;
    BattleState state_ {BattleState::Moving};
    BattleState last_state_ {BattleState::Moving};
    glm::mat4 camera_ {1.0f};
    bty::Sprite bg_;
    bty::Sprite frame_;
    bty::Rect bar_;
    bty::Sprite cursor_;
    bty::Sprite current_;
    std::array<std::array<glm::ivec2, 6>, 2> positions_;
    std::array<std::array<bty::Text, 6>, 2> counts_;
    std::array<std::array<bty::Sprite, 6>, 2> sprites_;
    std::array<const bty::Texture *, 25> unit_textures_;
    int cx_ {0};
    int cy_ {0};
    bty::Text status_;
    glm::ivec2 active_ {0, 0};
    float wait_timer_ {0};
    std::array<std::array<int, 6>, 2> armies_;
    std::array<std::array<int, 6>, 2> moves_left_;
    std::array<std::array<int, 6>, 2> waits_used_;
    std::array<std::array<bool, 6>, 2> flown_this_turn_;
    std::array<std::array<bool, 6>, 2> retaliated_this_turn_;
    int cursor_distance_x_ {0};
    int cursor_distance_y_ {0};
    const bty::Texture *move_;
    const bty::Texture *melee_;
    const bty::Texture *shoot_;
    const bty::Texture *magic_;
    bty::Sprite hit_marker_;
    float damage_timer_ {0};

    int last_attacking_team_ {-1};
    int last_attacking_unit_ {-1};
    int last_attacked_team_ {-1};
    int last_attacked_unit_ {-1};
    int last_kills_ {0};

    std::array<std::array<UnitState, 6>, 2> unit_states_;

    bty::Dialog menu_;
};

#endif    // BTY_INTRO_BATTLE_HPP_
