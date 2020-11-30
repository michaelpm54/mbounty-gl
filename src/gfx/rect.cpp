#include "gfx/rect.hpp"

namespace bty {

void Rect::set_color(const glm::vec4 &color)
{
    color_ = color;
}

const glm::vec4 &Rect::get_color() const{
    return color_;
}

void Rect::set_color(BoxColor color) {
    set_color(bty::get_color(color));
}

}
