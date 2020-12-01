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
    ViewContinent();
    ~ViewContinent();
    void load(bty::Assets &assets, bty::BoxColor color, const bty::Font &font, const std::array<const bty::Texture *, 8> &border_textures);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(int x, int y, int continent, const unsigned char * const map);
    void update(float dt);

private:
    bty::TextBox box_;
    bty::Text *continent_{nullptr};
    bty::Text *coordinates_{nullptr};
    bty::Sprite map_;
    bty::Texture map_texture_;
    float dot_timer_{0.0f};
    float dot_alpha_{0};
    int x_{0};
    int y_{0};
};

#endif // BTY_GAME_VIEW_CONTINENT_HPP_
