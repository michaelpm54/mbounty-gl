#ifndef BTY_GAME_GAME_HPP_
#define BTY_GAME_GAME_HPP_

#include <array>
#include <functional>
#include <queue>
#include <random>
#include <vector>

#include "game/dialog-def.hpp"
#include "game/dir-flags.hpp"
#include "game/entity.hpp"
#include "game/hero.hpp"
#include "game/hud.hpp"
#include "game/kings-castle.hpp"
#include "game/map.hpp"
#include "game/shop-info.hpp"
#include "game/shop.hpp"
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
        ViewArmy,
        ViewCharacter,
        ViewContinent,
        ViewContract,
        LoseGame,
        ViewPuzzle,
        Town,
        HudMessage,
        KingsCastle,
        Bridge,
        Shop,
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
    void end_week_astrology();
    void end_week_budget();
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
    void sail_to(int continent);
    void next_contract();
    void rent_boat();
    void view_contract();
    void buy_spell();
    void buy_siege();
    void kings_castle_option(int opt);
    void hud_messages(const std::vector<std::string> &messages);
    void draw_mobs(bty::Gfx &gfx);
    void view_army();
    void view_continent();
    bty::Dialog *show_dialog(const DialogDef &dialog);
    void castle_gate_confirm(int opt);
    void town_gate_confirm(int opt);
    void update_timestop(float dt);
    void update_day_clock(float dt);
    void update_mobs(float dt);
    void auto_move(float dt);
    void move_hero(float dt);
    void pause();
    void pause_confirm(int opt);

    void place_bridge_at(int x, int y, int continent, bool horizontal);
    void bridge_fail();

    void use_magic();
    void use_spell(int spell);

    /* Adventuring spells. */
    void spell_bridge();
    void spell_timestop();
    void spell_find_villain();
    void spell_tc_gate(bool town);
    void spell_instant_army();
    void spell_raise_control();

    void town_option(int opt);

    /* Event tiles. */
    void town(const Tile &tile);
    void castle(const Tile &tile);
    void artifact(const Tile &tile);
    void teleport_cave(const Tile &tile);
    void shop(const Tile &tile);
    void chest(const Tile &tile);

private:
    bty::Assets *assets_;
    bool paused_ {false};
    std::default_random_engine rng_ {};

    bty::SceneSwitcher *scene_switcher_;
    GameState state_ {GameState::Unpaused};
    GameState last_state_ {GameState::Unpaused};
    bool loaded_ {false};
    glm::mat4 camera_ {1.0f};

    Hud hud_;
    Map map_;

    glm::vec3 camera_pos_ {0};
    glm::mat4 game_camera_ {1};

    uint8_t move_flags_ {DIR_FLAG_NONE};

    Hero hero_;

    ViewArmy view_army_;
    ViewCharacter view_character_;
    ViewContinent view_continent_;
    ViewContract view_contract_;
    ViewPuzzle view_puzzle_;

    bool view_continent_fog_ {true};

    float clock_ {0};
    int days_passed_this_week {0};
    int weeks_passed_ {0};

    bty::TextBox lose_msg_;
    bty::Sprite lose_pic_;
    bty::Text *lose_msg_name_;
    int lose_state_ {0};

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

    bty::TextBox bridge_prompt_;

    std::queue<std::string> hud_message_queue_;

    Shop shop_;
    int shop_index_ {-1};

    /* RNG */
    std::array<std::array<Mob, 40>, 4> mobs_;
    std::array<std::vector<ShopInfo>, 4> shops_;
    std::array<glm::ivec2, 3> sail_maps_;
    std::array<glm::ivec2, 4> local_maps_;
    std::array<std::array<glm::ivec2, 2>, 4> teleport_caves_;
    std::array<std::vector<const Mob *>, 4> friendlies_;

    bool timestop_ {false};
    float timestop_timer_ {0};
    int timestop_left_ {0};

    bool visited_towns_[26] {false};
    bool visited_castles_[26] {false};

    std::vector<std::pair<std::shared_ptr<bty::Dialog>, DialogDef>> dialogs_;
};

#endif    // BTY_GAME_GAME_HPP_
