#ifndef BTY_GAME_GAME_HPP_
#define BTY_GAME_GAME_HPP_

#include "scene.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "gfx/dialog.hpp"

#include "game/hud.hpp"
#include "game/map.hpp"
#include "game/move-flags.hpp"
#include "game/entity.hpp"
#include "game/hero.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "game/view-continent.hpp"
#include "game/view-contract.hpp"

namespace bty {
class Assets;
struct Gfx;
class SceneSwitcher;
}    // namespace bty


class Game : public bty::Scene {
public:
    Game(bty::SceneSwitcher &scene_switcher);
    bool load(bty::Assets &assets) override;
    void draw(bty::Gfx &gfx) override;
    void key(int key, int scancode, int action, int mods) override;
    bool loaded() override;
    void update(float dt) override;

private:
    void update_camera();
    void collide(Tile &tile);
    void add_unit_to_army(int id, int count);
    void update_visited_tiles();
    void update_spells();
    void update_week_passed_cards();
    void sort_army();

private:
    enum class GameState {
        Unpaused,
        Paused,
        ViewArmy,
        ViewCharacter,
        ViewContinent,
        UseMagic,
        ViewContract,
        LoseGame,
        WeekPassed,
    };

    enum WeekPassedCard {
        Astrology,
        Budget,
    };
    
    bty::SceneSwitcher *scene_switcher_;
    GameState state_{GameState::Unpaused};
    bool loaded_ {false};
    bty::Font font_;
    glm::mat4 camera_ {1.0f};

    Hud hud_;
    Map map_;
    bty::Dialog pause_menu_;

    glm::vec3 camera_pos_{0};
    glm::mat4 game_camera_{1};
    glm::mat4 zoom_{0};

    uint8_t move_flags_{MOVE_FLAGS_NONE};

    Hero hero_;

    ViewArmy view_army_;
    ViewCharacter view_character_;
    ViewContinent view_continent_;
    bty::Dialog use_magic_;
    ViewContract view_contract_;

    bty::Text *magic_spells_[14]{nullptr};

    bool view_continent_fog_{true};

    float clock_{0};
    int days_passed_this_week{0};
    int weeks_passed_{0};

    WeekPassedCard week_passed_card_{WeekPassedCard::Astrology};
    bty::TextBox astrology_;
    bty::TextBox budget_;

    bool boat_rented_{false};
};

#endif    // BTY_GAME_GAME_HPP_
