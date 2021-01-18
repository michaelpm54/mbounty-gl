#ifndef BTY_GFX_RECT_HPP_
#define BTY_GFX_RECT_HPP_

#include <glm/vec4.hpp>

#include "data/color.hpp"
#include "engine/texture-cache.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Rect : public Transformable {
public:
    Rect() = default;
    Rect(const glm::vec4 &color, const glm::vec2 &size, const glm::vec2 &position);
    void setColor(const glm::vec4 &color);
    void setColor(BoxColor color);
    const glm::vec4 &getColor() const;

private:
    glm::vec4 _color {0.0f};
};

}    // namespace bty

#endif    // BTY_GFX_RECT_HPP_