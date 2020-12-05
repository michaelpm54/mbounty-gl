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
    };

    void move_unit_to(int team, int unit, int x, int y);
    void confirm();
    void move_confirm();
    void move_cursor(int dir);
    void status();
    void status_move(const Unit &unit);
    void status_wait(const Unit &unit);
    void next_unit();
    void update_cursor();
    void update_current();
    void reset_moves();
    void reset_waits();
    void set_state(BattleState state);

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
    std::array<int, 2> army_sizes_;
    std::array<std::array<bty::Sprite, 6>, 2> sprites_;
    std::array<const bty::Texture *, 25> unit_textures_;
    int cx_ {0};
    int cy_ {0};
    bty::Text status_;
    glm::ivec2 active_ {0, 0};
    float wait_timer_ {0};
    std::array<std::array<int, 6>, 2> armies_;
    std::array<std::array<int, 6>, 2> army_counts_;
    std::array<std::array<int, 6>, 2> moves_left_;
    std::array<std::array<int, 6>, 2> waits_used_;
};

#endif    // BTY_INTRO_BATTLE_HPP_
