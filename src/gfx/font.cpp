#include "gfx/font.hpp"

#include <spdlog/spdlog.h>

namespace bty {

void Font::loadFromTexture(const Texture *texture, glm::vec2 glyphSize)
{
    if (!texture) {
        spdlog::warn("Font::loadFromTexture: nullptr");
        return;
    }

    _columns = texture->width / static_cast<int>(glyphSize.x);
    _rows = texture->height / static_cast<int>(glyphSize.y);
    _texture = texture;
    _glyphSize = glyphSize;
    _advance = {1.0f / _columns, 1.0f / _rows};

    _texCoords.resize(_columns * _rows);

    for (int i = 0, max = _rows * _columns; i < max; i++) {
        int x = i % _columns;
        int y = i / _columns;

        glm::vec2 *uv = &_texCoords[i][0];

        *uv++ = {_advance.x * x, _advance.y * y};
        *uv++ = {_advance.x * (x + 1), _advance.y * y};
        *uv++ = {_advance.x * x, _advance.y * (y + 1)};
        *uv++ = {_advance.x * (x + 1), _advance.y * y};
        *uv++ = {_advance.x * (x + 1), _advance.y * (y + 1)};
        *uv++ = {_advance.x * x, _advance.y * (y + 1)};
    }
}

std::array<glm::vec2, 6> Font::getTexCoords(uint16_t code) const
{
    if (code >= _texCoords.size()) {
        spdlog::warn("Failed to get texture coordinate for char {:04x}", code + 32);
        return {};
    }
    return _texCoords[code];
}

glm::vec2 Font::getGlyphSize() const
{
    return _glyphSize;
}

glm::vec2 Font::getAdvance() const
{
    return _advance;
}

const Texture *Font::getTexture() const
{
    return _texture;
}

glm::vec2 Font::getUV(uint16_t code) const
{
    return {_advance.x * (code % _columns), _advance.y * (code / _columns)};
}

}    // namespace bty
