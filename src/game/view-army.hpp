#ifndef BTY_GAME_VIEW_ARMY_HPP_
#define BTY_GAME_VIEW_ARMY_HPP_

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

private:
    const bty::Texture *unit_textures_[25]{nullptr};
    bty::Sprite frame_;
    bty::Rect rects_[5];
    bty::Sprite units_[5];
    bty::Text info_[5][7];
    int num_units_{0};
};

#endif // BTY_GAME_VIEW_ARMY_HPP_
