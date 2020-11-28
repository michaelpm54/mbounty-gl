#ifndef BTY_INTRO_INTRO_HPP_
#define BTY_INTRO_INTRO_HPP_

#include "scene/scene.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "gfx/dialog.hpp"

namespace bty {
class Assets;
struct Gfx;
}    // namespace bty

class Intro : public bty::Scene {
public:
    bool load(bty::Assets &assets) override;
    void draw(bty::Gfx &gfx) override;
    void key(int key, int scancode, int action, int mods) override;
    bool loaded() override;

private:
    bool loaded_ {false};
    bty::Font font_;
    bty::Sprite bg_;
    bty::TextBox name_box_;
    bty::Dialog diff_box_;
    bty::TextBox help_box_;
    glm::mat4 camera_ {1.0f};
};

#endif    // BTY_INTRO_INTRO_HPP_
