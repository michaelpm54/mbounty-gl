#ifndef BTY_GFX_FONT_HPP_
#define BTY_GFX_FONT_HPP_

#include <array>
#include <vector>

#include <glm/vec2.hpp>

#include "gfx/texture.hpp"

namespace bty {

class Font {
public:
    void load_from_texture(const Texture *texture, glm::vec2 glyph_size);
    glm::vec2 get_glyph_size() const;
    glm::vec2 get_advance() const;
    const Texture *get_texture() const;
    std::array<glm::vec2, 6> get_texture_coordinates(uint16_t code) const;
    glm::vec2 get_uv(uint16_t code) const;

private:
    const Texture *texture_{nullptr};
    int columns_{0};
    int rows_{0};
    glm::vec2 glyph_size_{0.0f};
    glm::vec2 advance_{0.0f};
    std::vector<std::array<glm::vec2, 6>> texture_coordinates_;
};

}

#endif // BTY_GFX_FONT_HPP_
