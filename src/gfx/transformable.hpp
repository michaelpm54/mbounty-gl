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

    void set_position(float x, float y);
    void set_position(const glm::vec2 &position);
    glm::vec2 get_position() const;
    void set_size(float x, float y);
    void set_size(const glm::vec2 &size);
    glm::vec2 get_size() const;
    glm::mat4 &get_transform();

protected:
    glm::vec3 position_ {0.0f};
    glm::vec3 scale_ {1.0f};

private:
    glm::mat4 transform_ {1.0f};
    bool dirty_ {false};
};

}    // namespace bty

#endif    // BTY_GFX_TRANSFORMABLE_HPP_
