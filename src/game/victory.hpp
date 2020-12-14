#ifndef BTY_GAME_VICTORY_HPP_
#define BTY_GAME_VICTORY_HPP_

#include "game/scene.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
}    // namespace bty

struct Variables;
class SceneStack;
class DialogStack;
class Hud;

class Victory : public Scene {
public:
    Victory(SceneStack &ss, DialogStack &ds, bty::Assets &assets, Variables &v, Hud &hud);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void view();

private:
    SceneStack &ss;
    DialogStack &ds;
    Variables &v;
    Hud &hud;
    bty::Sprite bg;
    bty::Sprite unit_sprites[25];
    bty::Sprite hero;
    bty::Sprite king;
    bool parade {true};
};

#endif    // BTY_GAME_VICTORY_HPP_
