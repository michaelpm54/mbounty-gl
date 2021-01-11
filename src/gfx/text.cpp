#include "gfx/text.hpp"

#include <spdlog/spdlog.h>

#include "engine/texture-cache.hpp"
#include "gfx/font.hpp"
#include "gfx/texture.hpp"

namespace bty {

Text::~Text()
{
    if (vao_ != GL_NONE) {
        glDeleteVertexArrays(1, &vao_);
    }
    if (vbo_ != GL_NONE) {
        glDeleteBuffers(1, &vbo_);
    }
}

Text::Text(Text &&other)
    : Transformable(other)
{
    string_ = other.string_;
    num_vertices_ = other.num_vertices_;
    _font = other._font;

    /* Move constructor to prevent automatic destruction
        of the OpenGL resources. */
    vao_ = other.vao_;
    vbo_ = other.vbo_;
    other.vao_ = GL_NONE;
    other.vbo_ = GL_NONE;
}

Text::Text()
{
    glCreateVertexArrays(1, &vao_);
}

void Text::create(int x, int y, const std::string &string)
{
    _font = &Textures::instance().get_font();
    set_string(string);
    set_position({x * 8.0f, y * 8.0f});
}

void Text::set_string(const std::string &string)
{
    if (string_ == string) {
        return;
    }

    bool same = string_ == string;
    string_ = string;

    if (!same) {
        update_vbo();
    }
}

GLuint Text::get_vao() const
{
    return vao_;
}

GLuint Text::get_num_vertices() const
{
    return num_vertices_;
}

void Text::set_font(const Font &font)
{
    _font = &font;
}

const Font *Text::get_font() const
{
    return _font;
}

const std::string Text::get_string() const
{
    return string_;
}

void Text::update_vbo()
{
    assert(_font);

    num_vertices_ = 6 * static_cast<GLuint>(string_.size());

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 tex_coord;
    };

    std::vector<Vertex> vertices(num_vertices_);

    float x = 0;
    float y = 0;

    for (size_t i = 0u, s = string_.size(); i < s; i++) {
        switch (string_[i]) {
            case '\n':
                x = 0;
                y += 8;
                continue;
            case '\t':
                x += 16;
                continue;
            default:
                break;
        }

        uint16_t char_code = (string_[i] - 32) & 0xFF;
        const auto texture_coords = _font->get_texture_coordinates(char_code);

        auto *vertex = &vertices.data()[i * 6];

        *vertex++ = {{x, y}, texture_coords[0]};
        *vertex++ = {{x + 8, y}, texture_coords[1]};
        *vertex++ = {{x, y + 8}, texture_coords[2]};
        *vertex++ = {{x + 8, y}, texture_coords[3]};
        *vertex++ = {{x + 8, y + 8}, texture_coords[4]};
        *vertex++ = {{x, y + 8}, texture_coords[5]};

        x += 8;
    }

    if (vbo_ != GL_NONE) {
        glDeleteBuffers(1, &vbo_);
    }

    glGenBuffers(1, &vbo_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const void *)(sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(GL_NONE);
}

}    // namespace bty
