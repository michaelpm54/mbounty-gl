#include "gfx/transformable.hpp"

#include <glm/gtx/transform.hpp>

namespace bty {

void Transformable::setPosition(float x, float y)
{
    _position = {x, y, 0.0f};
    _dirty = true;
}

void Transformable::setPosition(const glm::vec2 &position)
{
    _position = {position.x, position.y, 0.0f};
    _dirty = true;
}

void Transformable::move(float dx, float dy)
{
    _position.x += dx;
    _position.y += dy;
    _dirty = true;
}

void Transformable::move(glm::vec2 d)
{
    _position.x += d.x;
    _position.x += d.y;
    _dirty = true;
}

glm::vec2 Transformable::getPosition() const
{
    return _position;
}

glm::mat4 &Transformable::getTransform()
{
    if (_dirty) {
        _transform = glm::translate(_position) * glm::scale(_scale);
        _dirty = false;
    }

    return _transform;
}

void Transformable::setSize(float x, float y)
{
    _scale = {x, y, 1.0f};
    _dirty = true;
}

void Transformable::setSize(const glm::vec2 &size)
{
    _scale = {size.x, size.y, 1.0f};
    _dirty = true;
}

glm::vec2 Transformable::getSize() const
{
    return _scale;
}

}    // namespace bty
