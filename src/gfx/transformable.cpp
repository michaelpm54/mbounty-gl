#include "gfx/transformable.hpp"

#include <glm/gtx/transform.hpp>

namespace bty {

void Transformable::set_position(const glm::vec2 &position)
{
    position_ = {position.x, position.y, 0.0f};
    dirty_ = true;
}

void Transformable::move(float dx, float dy)
{
    position_.x += dx;
    position_.y += dy;
}

void Transformable::move(glm::vec2 d)
{
    position_.x += d.x;
    position_.x += d.y;
}

glm::vec2 Transformable::get_position() const
{
    return position_;
}

glm::mat4 &Transformable::get_transform()
{
    if (dirty_) {
        transform_ = glm::translate(position_) * glm::scale(scale_);
        dirty_ = false;
    }

    return transform_;
}

void Transformable::set_size(float x, float y)
{
    scale_ = {x, y, 1.0f};
    dirty_ = true;
}

}    // namespace bty
