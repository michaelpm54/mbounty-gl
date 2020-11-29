#ifndef BTY_GFX_TRANSFORMABLE_HPP_
#define BTY_GFX_TRANSFORMABLE_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

namespace bty {

class Transformable {
public:
    Transformable() = default;
    Transformable(Transformable &&other) = default;
    Transformable(const Transformable &other) = default;

    void set_position(const glm::vec2 &position);
    glm::vec2 get_position() const;
    void set_size(float x, float y);
    glm::mat4 &get_transform();

protected:
    glm::vec3 position_ {0.0f};
    glm::vec3 scale_ {1.0f};

private:
    glm::mat4 transform_ {1.0f};
    bool dirty_ {false};
};

}

#endif // BTY_GFX_TRANSFORMABLE_HPP_
