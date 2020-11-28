#include "gfx/text.hpp"

#include <spdlog/spdlog.h>

#include "gfx/texture.hpp"

namespace bty {

Text::~Text()
{
    if (vao_ != GL_NONE) {
        glDeleteVertexArrays(1, &vao_);
    }
}

void Text::create(int x, int y, const std::string &string, const Texture *texture)
{
    set_texture(texture);
    set_string(string);
    set_position({x * 8.0f, y * 8.0f});
}

void Text::set_texture(const Texture *texture)
{
    if (!texture) {
        spdlog::warn("Text::set_texture: nullptr");
        return;
    }

    texture_ = texture;

    if (!string_.empty())
        set_string(string_);
}

void Text::set_string(const std::string &string)
{
    string_ = string;

    if (!texture_)
        return;

    num_vertices_ = 6 * string.size();

    std::vector<glm::vec4> vertices(num_vertices_);

    float glyph_size_x = texture_->width / 8.0f;
    float glyph_size_y = texture_->height / 8.0f;

    float adv_x = 1.0f / glyph_size_x;
    float adv_y = 1.0f / glyph_size_y;

    float x = 0;
    float y = 0;

    for (int i = 0, s = string.size(); i < s; i++) {
        uint16_t char_code = string[i] - 32;

        int glyph_x = char_code % 16;
        int glyph_y = char_code / 16;

        float u = adv_x * glyph_x;
        float v = adv_y * glyph_y;

        auto *vertex = &vertices.data()[i * 6];

        *vertex++ = { x,   y,   u,       v       };
        *vertex++ = { x+8, y,   u+adv_x, v       };
        *vertex++ = { x,   y+8, u,       v+adv_y };
        *vertex++ = { x+8, y,   u+adv_x, v       };
        *vertex++ = { x+8, y+8, u+adv_x, v+adv_y };
        *vertex++ = { x,   y+8, u,       v+adv_y };

        switch (string[i]) {
            case '\n':
                y += 8;
                break;
            case '\t':
                x += 16;
                break;
            default:
                x += 8;
                break;
        }
    }

    if (vao_ != GL_NONE) {
        glDeleteVertexArrays(1, &vao_);
    }

    glCreateVertexArrays(1, &vao_);

    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const void*)(sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(GL_NONE);

    glDeleteBuffers(1, &vbo);
}

const Texture *Text::get_texture() const
{
    return texture_;
}

GLuint Text::get_vao() const{
    return vao_;
}

GLuint Text::get_num_vertices() const {
    return num_vertices_;
}

}
