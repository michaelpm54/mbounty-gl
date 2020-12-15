#ifndef BTY_GAME_VIEW_ARMY_HPP_
#define BTY_GAME_VIEW_ARMY_HPP_

#include <array>

#include "engine/scene.hpp"
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

class ViewArmy : public bty::Scene {
public:
    ViewArmy(bty::SceneStack &ss, bty::Assets &assets);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void set_color(bty::BoxColor color);
    void update_info(int *army, int *counts, int *morales, int diff);

private:
    bty::SceneStack &ss;
    bty::Sprite frame_;
    std::array<const bty::Texture *, 25> unit_textures_;
    std::array<bty::Rect, 5> rects_;
    std::array<bty::Sprite, 5> units_;
    std::array<std::array<bty::Text, 7>, 5> info_;
    int num_units_ {0};
};

#endif    // BTY_GAME_VIEW_ARMY_HPP_
