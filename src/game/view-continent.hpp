#ifndef BTY_GAME_VIEW_CONTINENT_HPP_
#define BTY_GAME_VIEW_CONTINENT_HPP_

#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
}

struct SharedState;

class ViewContinent {
public:
    void load(bty::Assets &assets, bty::BoxColor color, const bty::Font &font, const std::array<const bty::Texture *, 8> &border_textures);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(const SharedState &state);
    void update(float dt);

private:
    bty::TextBox box_;
};

#endif // BTY_GAME_VIEW_CONTINENT_HPP_
