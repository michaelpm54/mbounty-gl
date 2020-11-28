#ifndef BTY_GFX_GFX_HPP_
#define BTY_GFX_GFX_HPP_

#include <glm/mat4x4.hpp>

#include "gfx/gl.hpp"

namespace bty {

enum Locations {
    SpriteTransform,
    SpriteCamera,
    SpriteTexture,
    SpriteFrame,
    RectTransform,
    RectCamera,
    RectColor,
    TextTransform,
    TextCamera,
    TextTexture,
    Count,
};

class Rect;
class Sprite;
class Text;

class Gfx {
public:
    Gfx();
    ~Gfx();
    void clear();
    void draw_sprite(Sprite &sprite, glm::mat4 &camera);
    void draw_rect(Rect &rect, glm::mat4 &camera);
    void draw_text(Text &text, glm::mat4 &camera);

private:
    void set_initial_gl_state();
    void load_shaders();
    void get_uniform_locations();
    void create_geometry();

private:
    GLuint sprite_shader_;
    GLuint rect_shader_;
    GLuint text_shader_;
    GLuint quad_vao_;
    GLint locations_[Locations::Count];
};

}    // namespace bty

#endif    // BTY_GFX_GFX_HPP_
