#ifndef BTY_GAME_VIEW_CONTRACT_HPP_
#define BTY_GAME_VIEW_CONTRACT_HPP_

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

class ViewContract {
public:
    void load(bty::Assets &assets, bty::BoxColor color);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(int contract, bool known_location);
    void update(float dt);

private:
    bty::TextBox box_;
    bty::Text no_contract_text_;
    bool no_contract_{false};
};

#endif // BTY_GAME_VIEW_CONTRACT_HPP_
