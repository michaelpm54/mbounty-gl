#include "gfx/sprite.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtx/transform.hpp>

namespace bty {

void Sprite::set_position(const glm::vec2 &position)
{
    position_ = {position.x, position.y, 0.0f};
    dirty_ = true;
}

void Sprite::set_texture(const Texture *texture)
{
    if (!texture)
        spdlog::warn("Sprite::set_texture: nullptr");
    else
        set_scale(texture->width, texture->height);

    texture_ = texture;
}

const Texture *Sprite::get_texture() const
{
    return texture_;
}

glm::mat4 &Sprite::get_transform()
{
    if (dirty_) {
        transform_ = glm::translate(position_) * glm::scale(scale_);
        dirty_ = false;
    }

    return transform_;
}

void Sprite::set_scale(float x, float y)
{
    scale_ = {x, y, 1.0f};
    dirty_ = true;
}

void Sprite::set_fill_color(const glm::vec4 &color)
{
    fill_color_ = color;
}

void Sprite::set_outline_color(const glm::vec4 &color)
{
    outline_color_ = color;
}

void Sprite::set_outline_thickness(int n)
{
    outline_thickness_ = n;
}

}    // namespace bty
