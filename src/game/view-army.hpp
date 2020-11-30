#ifndef BTY_GAME_VIEW_ARMY_HPP_
#define BTY_GAME_VIEW_ARMY_HPP_

#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Assets;
class Gfx;
struct Texture;
}

struct SharedState;

class ViewArmy {
public:
    void load(bty::Assets &assets, const glm::vec4 &color);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(const SharedState &state);
    void update(float dt);

private:
    const bty::Texture *unit_textures_[25]{nullptr};
    bty::Sprite frame_;
    bty::Rect rects_[5];
    bty::Sprite units_[5];
    int num_units_{0};
};

#endif // BTY_GAME_VIEW_ARMY_HPP_
