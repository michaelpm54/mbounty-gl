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
    SpriteFlip,
    SpriteSingleTextureTransform,
    SpriteSingleTextureCamera,
    SpriteSingleTextureTexture,
    SpriteSingleTextureFlip,
    SpriteSingleTextureRepeat,
    SpriteSingleTextureSize,
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
    void create_quad_vao();

private:
    GLuint sprite_shader_ {GL_NONE};
    GLuint sprite_single_texture_shader_ {GL_NONE};
    GLuint rect_shader_ {GL_NONE};
    GLuint text_shader_ {GL_NONE};
    GLuint quad_vao_ {GL_NONE};
    GLuint quad_vbo_ {GL_NONE};
    GLint locations_[Locations::Count];
};

}    // namespace bty

#endif    // BTY_GFX_GFX_HPP_
