#ifndef BTY_GAME_GAME_HPP_
#define BTY_GAME_GAME_HPP_

#include <array>
#include <queue>
#include <random>
#include <vector>

#include "game/entity.hpp"
#include "game/hero.hpp"
#include "game/hud.hpp"
#include "game/kings-castle.hpp"
#include "game/map.hpp"
#include "game/move-flags.hpp"
#include "game/town.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "game/view-continent.hpp"
#include "game/view-contract.hpp"
#include "game/view-puzzle.hpp"
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

class Game : public bty::Scene {
public:
    Game(bty::SceneSwitcher &scene_switcher);
    bool load(bty::Assets &assets) override;
    void draw(bty::Gfx &gfx) override;
    void key(int key, int scancode, int action, int mods) override;
    bool loaded() override;
    void update(float dt) override;
    void enter(bool reset) override;

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
        Disgrace,
        ViewPuzzle,
        Dismiss,
        ChestMap,
        SailNext,
        Town,
        HudMessage,
        KingsCastle,
        UntrainedInMagic,
        Bridge,
    };

    struct EventTile {
        int x;
        int y;
        int c;
    };

    struct Mob {
        bool dead {false};
        Entity entity;
        glm::ivec2 tile;
        std::array<int, 6> army {-1};
        std::array<int, 6> counts;
    };

    void update_camera();
    void collide(Tile &tile);
    void add_unit_to_army(int id, int count);
    void update_visited_tiles();
    void update_spells();
    void end_of_week(bool search);
    void sort_army(int *army, int *counts, int max);
    void disgrace();
    void lose_game();
    void setup_game();
    void view_puzzle();
    void dismiss();
    void dismiss_slot(int slot);
    void gen_tiles();
    void clear_movement();
    void set_state(GameState state);
    void find_map(const Tile &tile);
    void sail_next();
    void sail_to(int continent);
    void town(const Tile &tile);
    void town_option(int opt);
    void next_contract();
    void rent_boat();
    void view_contract();
    void buy_spell();
    void buy_siege();
    void kings_castle_option(int opt);
    void use_spell(int spell);
    void bridge(int direction);
    void bridge_fail();
    void hud_messages(const std::vector<std::string> &messages);
    void draw_mobs(bty::Gfx &gfx);
    void view_army();

private:
    enum WeekPassedCard {
        Astrology,
        Budget,
    };

    std::default_random_engine rng_ {};

    bty::SceneSwitcher *scene_switcher_;
    GameState state_ {GameState::Unpaused};
    GameState last_state_ {GameState::Unpaused};
    bool loaded_ {false};
    glm::mat4 camera_ {1.0f};

    Hud hud_;
    Map map_;
    bty::Dialog pause_menu_;

    glm::vec3 camera_pos_ {0};
    glm::mat4 game_camera_ {1};
    glm::mat4 zoom_ {0};

    uint8_t move_flags_ {MOVE_FLAGS_NONE};

    Hero hero_;

    ViewArmy view_army_;
    ViewCharacter view_character_;
    ViewContinent view_continent_;
    bty::Dialog use_magic_;
    ViewContract view_contract_;
    ViewPuzzle view_puzzle_;

    bty::Text *magic_spells_[14] {nullptr};

    bool view_continent_fog_ {true};

    float clock_ {0};
    int days_passed_this_week {0};
    int weeks_passed_ {0};

    WeekPassedCard week_passed_card_ {WeekPassedCard::Astrology};
    bty::TextBox astrology_;
    bty::TextBox budget_;

    bty::TextBox lose_msg_;
    bty::Sprite lose_pic_;
    bty::Text *lose_msg_name_;
    int lose_state_ {0};

    bty::Dialog dismiss_;

    bty::TextBox found_map_;
    bty::Text *found_map_continent_;

    bty::Dialog sail_dialog_;
    bool controls_locked_ {false};
    float control_lock_timer_ {0};
    glm::ivec2 auto_move_dir_ {0};

    Town town_;
    std::array<int, 26> town_units_;
    std::array<CastleOccupation, 26> castle_occupations_;
    std::array<int, 26> town_spells_;

    KingsCastle kings_castle_;

    std::array<const bty::Texture *, 25> unit_textures_;

    std::array<std::array<glm::ivec2, 40>, 4> mob_tile_;
    std::array<std::array<std::array<int, 6>, 40>, 4> mob_armies_;

    bty::TextBox untrained_in_magic_;
    bty::TextBox bridge_prompt_;

    std::queue<std::string> hud_message_queue_;

    bty::TextBox disgrace_;

    struct Shop {
        int x;
        int y;
        int unit;
        int count;
    };

    /* RNG */
    std::array<std::array<Mob, 40>, 4> mobs_;
    std::array<std::vector<Shop>, 4> shops_;
    std::array<glm::ivec2, 3> sail_maps_;
    std::array<glm::ivec2, 4> local_maps_;
};

#endif    // BTY_GAME_GAME_HPP_
