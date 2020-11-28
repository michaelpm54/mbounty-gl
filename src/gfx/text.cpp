#include "gfx/text.hpp"

#include <spdlog/spdlog.h>

#include "gfx/font.hpp"
#include "gfx/texture.hpp"

namespace bty {

Text::~Text()
{
    if (vao_ != GL_NONE) {
        glDeleteVertexArrays(1, &vao_);
    }
}

void Text::create(int x, int y, const std::string &string, const Font &font)
{
    font_ = &font;
    set_string(string);
    set_position({x * 8.0f, y * 8.0f});
}

void Text::set_string(const std::string &string)
{
    string_ = string;

    if (!font_) {
        spdlog::warn("Text::set_string: no font");
        return;
    }

    num_vertices_ = 6 * string.size();

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 tex_coord;
    };

    std::vector<Vertex> vertices(num_vertices_);

    float x = 0;
    float y = 0;

    for (int i = 0, s = string.size(); i < s; i++) {
        switch (string[i]) {
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

        uint16_t char_code = (string[i] - 32) & 0xFF;
        const auto texture_coords = font_->get_texture_coordinates(char_code);

        auto *vertex = &vertices.data()[i * 6];

        *vertex++ = { { x,   y   }, texture_coords[0] };
        *vertex++ = { { x+8, y   }, texture_coords[1] };
        *vertex++ = { { x,   y+8 }, texture_coords[2] };
        *vertex++ = { { x+8, y   }, texture_coords[3] };
        *vertex++ = { { x+8, y+8 }, texture_coords[4] };
        *vertex++ = { { x,   y+8 }, texture_coords[5] };

        x += 8;
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

const Font *Text::get_font() const
{
    return font_;
}

GLuint Text::get_vao() const{
    return vao_;
}

GLuint Text::get_num_vertices() const {
    return num_vertices_;
}

}
