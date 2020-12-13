#ifndef BTY_GAME_VIEW_CONTINENT_HPP_
#define BTY_GAME_VIEW_CONTINENT_HPP_

#include "game/scene.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
}    // namespace bty

class SceneStack;
struct Variables;

class ViewContinent : public Scene {
public:
    ViewContinent(SceneStack &ss, bty::Assets &assets);
    ~ViewContinent();
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void update_info(Variables &v);
    void set_color(bty::BoxColor color);

private:
    Variables *v;
    SceneStack &ss;
    bty::TextBox box_;
    bty::Text *continent_ {nullptr};
    bty::Text *coordinates_ {nullptr};
    bty::Sprite map_;
    bty::Texture map_texture_;
    float dot_timer_ {0.0f};
    float dot_alpha_ {0};
    int x_ {0};
    int y_ {0};
    bool fog {true};
};

#endif    // BTY_GAME_VIEW_CONTINENT_HPP_
