#ifndef BTY_GFX_FONT_HPP_
#define BTY_GFX_FONT_HPP_

#include <array>
#include <glm/vec2.hpp>
#include <vector>

#include "gfx/texture.hpp"

namespace bty {

class Font {
public:
    void loadFromTexture(const Texture *texture, glm::vec2 glyphSize);
    glm::vec2 getGlyphSize() const;
    glm::vec2 getAdvance() const;
    const Texture *getTexture() const;
    std::array<glm::vec2, 6> getTexCoords(uint16_t code) const;
    glm::vec2 getUV(uint16_t code) const;

private:
    const Texture *_texture {nullptr};
    int _columns {0};
    int _rows {0};
    glm::vec2 _glyphSize {0.0f};
    glm::vec2 _advance {0.0f};
    std::vector<std::array<glm::vec2, 6>> _texCoords;
};

}    // namespace bty

#endif    // BTY_GFX_FONT_HPP_
