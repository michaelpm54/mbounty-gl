#ifndef BTY_GAME_VIEW_CHARACTER_HPP_
#define BTY_GAME_VIEW_CHARACTER_HPP_

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

class ViewCharacter {
public:
    void load(bty::Assets &assets, bty::BoxColor color, int hero_id);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(const SharedState &state);
    void set_color(bty::BoxColor color);

private:
    bty::Sprite frame_;
    bty::Sprite portrait_;
    bty::Rect rect_;
    bty::Text info_[11];
    const bty::Texture *portraits_[4];
    bty::Sprite map_sprites_[4];
    int found_maps_{0};
};

#endif // BTY_GAME_VIEW_CHARACTER_HPP_
