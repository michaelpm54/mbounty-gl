#ifndef BTY_GFX_TRANSFORMABLE_HPP_
#define BTY_GFX_TRANSFORMABLE_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace bty {

class Transformable {
public:
    virtual ~Transformable() = default;

    void move(glm::vec2 d);
    void move(float dx, float dy);

    void setPosition(float x, float y);
    void setPosition(const glm::vec2 &position);
    glm::vec2 getPosition() const;
    void setSize(float x, float y);
    void setSize(const glm::vec2 &size);
    glm::vec2 getSize() const;
    glm::mat4 &getTransform();

protected:
    glm::vec3 _position {0.0f};
    glm::vec3 _scale {1.0f};

private:
    glm::mat4 _transform {1.0f};
    bool _dirty {false};
};

}    // namespace bty

#endif    // BTY_GFX_TRANSFORMABLE_HPP_
