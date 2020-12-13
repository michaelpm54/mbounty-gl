#ifndef BTY_GAME_INGAME_HPP_
#define BTY_GAME_INGAME_HPP_

#include "game/battle.hpp"
#include "game/defeat.hpp"
#include "game/dialog-stack.hpp"
#include "game/dir-flags.hpp"
#include "game/gen-variables.hpp"
#include "game/hero.hpp"
#include "game/kings-castle.hpp"
#include "game/map.hpp"
#include "game/mob.hpp"
#include "game/scene.hpp"
#include "game/shop-info.hpp"
#include "game/shop.hpp"
#include "game/town.hpp"
#include "game/variables.hpp"
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

namespace bty {
class Assets;
class Gfx;
}    // namespace bty

class Hud;
class DialogStack;
class SceneStack;
struct GLFWwindow;

class Ingame : public Scene {
public:
    void update(float dt) override;
    void setup(int hero, int diff);
    void key(int key, int action) override;
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    Ingame(GLFWwindow *window, SceneStack &ss, DialogStack &ds, bty::Assets &assets, Hud &hud);

private:
    void defeat();
    void victory();
    void disgrace();
    void gen_tiles();
    void next_contract();
    void use_spell(int spell);
    void update_mobs(float dt);
    void draw_mobs(bty::Gfx &gfx);
    void add_unit_to_army(int id, int count);

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
    void end_week_budget();
    void end_week_astrology();
    void update_day_clock(float dt);
    void update_timestop_clock(float dt);

    /* Movement */
    int get_move_input();
    void update_camera();
    void auto_move(float dt);
    void sail_to(int continent);
    void update_visited_tiles();
    void move_hero(int flags, float dt);
    void move_hero_to(int x, int y, int c);

    /* Collision */
    void collide(const Tile &tile);
    void collide_town(const Tile &tile);
    void collide_shop(const Tile &tile);
    void collide_chest(const Tile &tile);
    void collide_castle(const Tile &tile);
    void collide_artifact(const Tile &tile);
    void collide_teleport_cave(const Tile &tile);

    void defeat_pop(int ret);
    void battle_pop(int ret);

private:
    Variables v;
    GenVariables gen;

    GLFWwindow *window;
    SceneStack &ss;
    DialogStack &ds;
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

    Defeat s_defeat;
    Battle s_battle;
    int battle_mob {-1};
};

#endif    // BTY_INGAME_INGAME_HPP_
