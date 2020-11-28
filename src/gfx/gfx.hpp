#ifndef BTY_GFX_GFX_HPP_
#define BTY_GFX_GFX_HPP_

#include <glm/mat4x4.hpp>

#include "gfx/gl.hpp"

namespace bty {

enum Locations {
    SpriteTransform,
    SpriteCamera,
    SpriteTexture,
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

struct Gfx {
    GLuint sprite_shader;
    GLuint rect_shader;
    GLuint text_shader;
    GLuint quad_vao;
    GLint locations[Locations::Count];
};

Gfx *gfx_init();
void gfx_free(Gfx *gfx);
void gfx_clear(Gfx *gfx);
void gfx_draw_sprite(Gfx *gfx, Sprite &sprite, glm::mat4 &camera);
void gfx_draw_rect(Gfx *gfx, Rect &rect, glm::mat4 &camera);
void gfx_draw_text(Gfx *gfx, Text &text, glm::mat4 &camera);

}    // namespace bty

#endif    // BTY_GFX_GFX_HPP_
