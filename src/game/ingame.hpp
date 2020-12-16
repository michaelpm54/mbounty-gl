#ifndef BTY_GAME_INGAME_HPP_
#define BTY_GAME_INGAME_HPP_

#include "engine/dialog-stack.hpp"
#include "engine/dialog.hpp"
#include "engine/scene.hpp"
#include "engine/textbox.hpp"
#include "game/battle.hpp"
#include "game/defeat.hpp"
#include "game/dir-flags.hpp"
#include "game/garrison.hpp"
#include "game/gen-variables.hpp"
#include "game/hero.hpp"
#include "game/kings-castle.hpp"
#include "game/map.hpp"
#include "game/mob.hpp"
#include "game/shop-info.hpp"
#include "game/shop.hpp"
#include "game/town.hpp"
#include "game/variables.hpp"
#include "game/victory.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "game/view-continent.hpp"
#include "game/view-contract.hpp"
#include "game/view-puzzle.hpp"
#include "game/wizard.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Gfx;
class DialogStack;
class SceneStack;
}    // namespace bty

class Hud;
struct GLFWwindow;
struct c2AABB;

class Ingame : public bty::Scene {
public:
    Ingame(GLFWwindow *window, bty::SceneStack &ss, bty::DialogStack &ds, bty::Assets &assets, Hud &hud);

    void update(float dt) override;
    void key(int key, int action) override;
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;

    void setup(int hero, int diff);

private:
    void defeat();
    void victory();
    void disgrace();
    void gen_tiles();
    void next_contract();
    void use_spell(int spell);
    void update_mobs(float dt);
    void draw_mobs(bty::Gfx &gfx);
    bool add_unit_to_army(int id, int count);

    /* Adventure spells */
    void spell_bridge();
    void spell_timestop();
    void spell_find_villain();
    void spell_instant_army();
    void spell_raise_control();
    void spell_tc_gate(bool town);

    /* Spell helpers */
    void town_gate_confirm(int opt);
    void castle_gate_confirm(int opt);
    void place_bridge_at(int x, int y, int continent, bool horizontal);

    /* Pause menu options */
    void pause();
    void dismiss();
    void use_magic();

    /* Pause menu helpers */
    void dismiss_slot(int slot);

    /* Clocks */
    void end_week_budget(bool search);
    void end_week_astrology(bool search);
    void update_day_clock(float dt);
    void update_timestop_clock(float dt);

    /* Movement */
    bool move_increment(c2AABB &box, float dx, float dy, Tile &center_tile, Tile &collided_tile);
    int get_move_input();
    void update_camera();
    void auto_move(float dt);
    void sail_to(int continent);
    void update_visited_tiles();
    void move_hero(int flags, float dt);
    void move_hero_to(int x, int y, int c);

    /* Collision */
    bool collide(const Tile &tile);
    void collide_sign(const Tile &tile);
    void collide_town(const Tile &tile);
    void collide_shop(const Tile &tile);
    void collide_chest(const Tile &tile);
    void collide_castle(const Tile &tile);
    void collide_artifact(const Tile &tile);
    void collide_teleport_cave(const Tile &tile);
    void wizard();

    void defeat_pop(int ret);
    void battle_pop(int ret);

    void search_area();
    void do_search();
    void search_fail();

    void end_week(bool search);

private:
    Variables v;
    GenVariables gen;

    GLFWwindow *window;
    bty::SceneStack &ss;
    bty::DialogStack &ds;
    Hud &hud;
    Map map;
    Hero hero;
    glm::mat4 ui_cam;
    glm::mat4 map_cam;
    DirFlags move_flags;
    std::array<const bty::Texture *, 25> unit_textures;

    /* Pause */
    ViewArmy view_army;
    ViewCharacter view_char;
    ViewContinent view_continent;
    ViewContract view_contract;
    ViewPuzzle view_puzzle;

    /* Clocks */
    float day_clock;
    float timestop_timer;

    /* Collision */
    KingsCastle kings_castle;
    Shop shop;
    Town town;
    Wizard s_wizard;

    Defeat s_defeat;
    Battle s_battle;
    Garrison s_garrison;
    Victory s_victory;

    int battle_mob {-1};
    int garrison_castle_id {-1};

    int sceptre_continent {-1};
    int sceptre_x {-1};
    int sceptre_y {-1};

    int temp_continent = -1;

    bty::Sprite boat;

    int last_water_x {-1};
    int last_water_y {-1};

    bty::Rect cr;
    bty::Text tile_text;
    Tile last_tile {-1, -1, -1};
    Tile last_event_tile {-1, -1, -1};

    bool debug {false};
};

#endif    // BTY_GAME_INGAME_HPP_
