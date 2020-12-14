#ifndef BTY_GAME_TOWN_HPP_
#define BTY_GAME_TOWN_HPP_

#include "bounty.hpp"
#include "game/dialog-def.hpp"
#include "game/scene.hpp"
#include "gfx/dialog.hpp"
#include "gfx/sprite.hpp"

class SceneStack;
class DialogStack;
class Hud;
class ViewContract;
struct Variables;
struct GenVariables;
struct Tile;

namespace bty {
class Gfx;
struct Texture;
}    // namespace bty

class Town : public Scene {
public:
    Town(SceneStack &ss, DialogStack &ds, bty::Assets &assets, Variables &v, GenVariables &gen, Hud &hud, ViewContract &view_contract, bty::Sprite &boat);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void view(int town_id, const Tile &tile, int continent, int unit_id, int spell);

    void get_contract();
    void rent_boat();
    void gather_information();
    void buy_spell();
    void buy_siege();

private:
    SceneStack &ss;
    DialogStack &ds;
    Variables &v;
    GenVariables &gen;
    Hud &hud;
    ViewContract &view_contract;
    bty::Sprite &boat;
    bty::Sprite bg_;
    bty::Sprite unit_;
    const bty::Texture *unit_textures_[25];
    int current_info_contract_ {-1};
    int town_id {-1};

    bty::Dialog dialog;
    bty::Text *t_town_name;
    bty::Text *t_gp;
    bty::Option *t_get_contract;
    bty::Option *t_rent_boat;
    bty::Option *t_gather_information;
    bty::Option *t_buy_spell;
    bty::Option *t_buy_siege;
};

#endif    // BTY_GAME_TOWN_HPP_
