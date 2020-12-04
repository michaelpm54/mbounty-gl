#ifndef BTY_GFX_RECT_HPP_
#define BTY_GFX_RECT_HPP_

#include <glm/vec4.hpp>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Rect : public Transformable {
public:
    Rect() = default;
    Rect(const glm::vec4 &color, const glm::vec2 &size, const glm::vec2 &position);
    void set_color(const glm::vec4 &color);
    void set_color(BoxColor color);
    const glm::vec4 &get_color() const;

private:
    glm::vec4 color_ {0.0f};
};

}    // namespace bty

#endif    // BTY_GFX_RECT_HPP_