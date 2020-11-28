#include "gfx/sprite.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtx/transform.hpp>

namespace bty {

void Sprite::set_texture(const Texture *texture)
{
    if (!texture)
        spdlog::warn("Sprite::set_texture: nullptr");
    else
        set_size(texture->width, texture->height);

    texture_ = texture;
}

const Texture *Sprite::get_texture() const
{
    return texture_;
}

}    // namespace bty
