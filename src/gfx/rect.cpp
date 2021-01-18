#include "gfx/rect.hpp"

namespace bty {

void Rect::setColor(const glm::vec4 &color)
{
    _color = color;
}

const glm::vec4 &Rect::getColor() const
{
    return _color;
}

void Rect::setColor(BoxColor color)
{
    setColor(bty::getColor(color));
}

Rect::Rect(const glm::vec4 &color, const glm::vec2 &size, const glm::vec2 &position)
{
    setColor(color);
    setPosition(position);
    setSize(size);
}

}    // namespace bty
