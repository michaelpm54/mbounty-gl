#ifndef BTY_GAME_VIEW_ARMY_HPP_
#define BTY_GAME_VIEW_ARMY_HPP_

#include <array>

#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
}

struct SharedState;

class ViewArmy {
public:
    void load(bty::Assets &assets, bty::BoxColor color);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(const SharedState &state);
    void update(float dt);
    void set_color(bty::BoxColor color);

private:
    bty::Sprite frame_;
    std::array<const bty::Texture *, 25> unit_textures_;
    std::array<bty::Rect, 5> rects_;
    std::array<bty::Sprite, 5> units_;
    std::array<std::array<bty::Text, 7>, 5> info_;
    int num_units_{0};
};

#endif // BTY_GAME_VIEW_ARMY_HPP_
