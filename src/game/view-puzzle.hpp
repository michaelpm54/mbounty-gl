#ifndef BTY_GAME_VIEW_PUZZLE_HPP_
#define BTY_GAME_VIEW_PUZZLE_HPP_

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

class ViewPuzzle {
public:
    void load(bty::Assets &assets);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(const SharedState &state);
    void update(float dt);

private:
    const bty::Texture *textures_[25]{nullptr};
    bty::Sprite sprites_[25];
};

#endif // BTY_GAME_VIEW_PUZZLE_HPP_
