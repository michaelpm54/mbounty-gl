#ifndef BTY_GFX_RECT_HPP_
#define BTY_GFX_RECT_HPP_

#include <glm/vec4.hpp>

#include "assets.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Rect : public Transformable {
public:
    void set_color(const glm::vec4 &color);
    const glm::vec4 &get_color() const;

private:
    glm::vec4 color_{0.0f};
};

}    // namespace bty

#endif    // BTY_GFX_RECT_HPP_