#ifndef BTY_GFX_SPRITE_HPP_
#define BTY_GFX_SPRITE_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/vec2.hpp>

#include "assets.hpp"

namespace bty {

class Sprite {
public:
    void set_position(const glm::vec2 &position);
    void set_texture(const Texture *texture);
    void set_scale(float x, float y);
    void set_fill_color(const glm::vec4 &color);
    void set_outline_color(const glm::vec4 &color);
    void set_outline_thickness(int n);
    glm::mat4 &get_transform();
    const Texture *get_texture() const;

private:
    glm::vec3 position_ {0.0f};
    glm::mat4 transform_ {1.0f};
    glm::vec3 scale_ {1.0f};
    bool dirty_ {false};
    const Texture *texture_ {nullptr};
    glm::vec4 outline_color_{0.0f};
    glm::vec4 fill_color_{0.0f};
    int outline_thickness_{0};
};

}    // namespace bty

#endif    // BTY_GFX_SPRITE_HPP_