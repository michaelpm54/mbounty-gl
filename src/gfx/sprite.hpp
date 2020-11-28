#ifndef BTY_GFX_SPRITE_HPP_
#define BTY_GFX_SPRITE_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include "assets.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Sprite : public Transformable {
public:
    void set_texture(const Texture *texture);
    const Texture *get_texture() const;

private:
    const Texture *texture_ {nullptr};
};

}    // namespace bty

#endif    // BTY_GFX_SPRITE_HPP_