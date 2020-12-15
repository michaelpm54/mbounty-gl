#ifndef BTY_GAME_VIEW_CONTINENT_HPP_
#define BTY_GAME_VIEW_CONTINENT_HPP_

#include "engine/scene.hpp"
#include "engine/textbox.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
class SceneStack;
}    // namespace bty

struct Variables;

class ViewContinent : public bty::Scene {
public:
    ViewContinent(bty::SceneStack &ss, bty::Assets &assets);
    ~ViewContinent();
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void update_info(Variables &v, bool have_map, bool force_show = false);
    void set_color(bty::BoxColor color);

private:
    void gen_texture();

private:
    Variables *v;
    bty::SceneStack &ss;
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
    bool have_map {false};
};

#endif    // BTY_GAME_VIEW_CONTINENT_HPP_
