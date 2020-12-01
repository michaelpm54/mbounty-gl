#ifndef GAME_HUD_HPP_
#define GAME_HUD_HPP_

#include <vector>

#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

#include <glm/mat4x4.hpp>

namespace bty {
    class Assets;
    class Font;
    class Gfx;
    struct Texture;
}

struct SharedState;

class Hud {
public:
    void load(bty::Assets &assets, bty::Font &font, SharedState &state);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    void update_state();

    /* Temporarily set the text in the box. Intended to be cleared ASAP. */
    /* Useful for an alert or message. */
    void set_title(const std::string &msg);

private:
    SharedState *state_{nullptr};
    const bty::Font *font_{nullptr};
    bty::Sprite frame_;
    bty::Rect top_bar_;
    bty::Text name_;
    bty::Text days_;
    bty::Sprite contract_;
    bty::Sprite siege_;
    bty::Sprite magic_;
    bty::Sprite puzzle_;
    bty::Sprite money_;
    std::vector<const bty::Texture *> contract_textures_;

    const bty::Texture *siege_no{nullptr};
    const bty::Texture *siege_yes{nullptr};

    const bty::Texture *magic_no{nullptr};
    const bty::Texture *magic_yes{nullptr};
};

#endif // GAME_HUD_HPP_
