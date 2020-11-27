#ifndef BTY_GFX_GFX_HPP_
#define BTY_GFX_GFX_HPP_

#include <glm/mat4x4.hpp>

#include "gfx/gl.hpp"

namespace bty {

enum Locations {
    SpriteTransform,
    SpriteCamera,
    SpriteTexture,
    Count,
};

class Sprite;

struct Gfx {
    GLuint sprite_shader;
    GLuint sprite_vao;
    GLint locations[Locations::Count];
};

Gfx *gfx_init();
void gfx_free(Gfx *gfx);
void gfx_clear(Gfx *gfx);
void gfx_draw_sprite(Gfx *gfx, Sprite &sprite, glm::mat4 &camera);

}    // namespace bty

#endif    // BTY_GFX_GFX_HPP_
