#ifndef BTY_GAME_GAME_HPP_
#define BTY_GAME_GAME_HPP_

#include <array>
#include <functional>
#include <queue>
#include <random>
#include <vector>

#include "engine/dialog-def.hpp"
#include "engine/dialog.hpp"
#include "engine/scene-stack.hpp"
#include "engine/textbox.hpp"
#include "game/dir-flags.hpp"
#include "game/entity.hpp"
#include "game/hero.hpp"
#include "game/hud.hpp"
#include "game/ingame.hpp"
#include "game/intro.hpp"
#include "game/kings-castle.hpp"
#include "game/map.hpp"
#include "game/mob.hpp"
#include "game/shop-info.hpp"
#include "game/shop.hpp"
#include "game/town.hpp"
#include "game/view-army.hpp"
#include "game/view-character.hpp"
#include "game/view-continent.hpp"
#include "game/view-contract.hpp"
#include "game/view-puzzle.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Gfx;
}    // namespace bty

struct GLFWwindow;

class Game {
public:
    Game(GLFWwindow *window, bty::Assets &assets);
    void draw(bty::Gfx &gfx);
    void key(int key, int action);
    void update(float dt);

private:
    void intro_pop(int ret);
    void ingame_pop(int ret);

private:
    GLFWwindow *window_ {nullptr};

    Intro intro;
    Ingame ingame;

    int hero_id {0};
    int difficulty {0};

    bty::SceneStack ss;
    bty::DialogStack ds;

    Hud hud_;

    glm::mat4 camera_ {1.0f};
};

#endif    // BTY_GAME_GAME_HPP_
