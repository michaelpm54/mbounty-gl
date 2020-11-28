#include "gfx/font.hpp"

#include <spdlog/spdlog.h>

namespace bty {

void Font::load_from_texture(const Texture *texture, glm::vec2 glyph_size)
{
    if (!texture)
        spdlog::warn("Font::load_from_texture: nullptr");

    columns_ = texture->width / glyph_size.x;
    rows_ = texture->height / glyph_size.y;
    texture_ = texture;
    glyph_size_ = glyph_size;
    advance_ = {1.0f / columns_, 1.0f / rows_};

    texture_coordinates_.resize(columns_ * rows_);

    for (int i = 0, max = rows_ * columns_; i < max; i++) {
        int x = i % columns_;
        int y = i / columns_;

        glm::vec2 *uv = &texture_coordinates_[i][0];

            *uv++ = { advance_.x * x,     advance_.y * y     };
            *uv++ = { advance_.x * (x+1), advance_.y * y     };
            *uv++ = { advance_.x * x,     advance_.y * (y+1) };
            *uv++ = { advance_.x * (x+1), advance_.y * y     };
            *uv++ = { advance_.x * (x+1), advance_.y * (y+1) };
            *uv++ = { advance_.x * x,     advance_.y * (y+1) };
    }
}

std::array<glm::vec2, 6> Font::get_texture_coordinates(uint16_t code) const
{
    if (code >= texture_coordinates_.size()) {
        spdlog::warn("Failed to get texture coordinate for char {:04x}", code + 32);
        return {};
    }
    return texture_coordinates_[code];
}

glm::vec2 Font::get_glyph_size() const
{
    return glyph_size_;
}

glm::vec2 Font::get_advance() const
{
    return advance_;
}

const Texture *Font::get_texture() const
{
    return texture_;
}

glm::vec2 Font::get_uv(uint16_t code) const
{
    return { advance_.x * (code % columns_), advance_.y * (code / columns_) };
}

}
