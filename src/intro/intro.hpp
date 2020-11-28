#ifndef BTY_INTRO_INTRO_HPP_
#define BTY_INTRO_INTRO_HPP_

#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "scene/scene.hpp"
#include "gfx/textbox.hpp"

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
    bty::Sprite bg_;
    bty::TextBox name_box_;
    bty::TextBox diff_box_;
    bty::TextBox help_box_;
    bty::Text name_text_;
    glm::mat4 camera_ {1.0f};
};

#endif    // BTY_INTRO_INTRO_HPP_
