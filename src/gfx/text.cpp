#include "gfx/text.hpp"

#include <spdlog/spdlog.h>

#include "engine/texture-cache.hpp"
#include "gfx/font.hpp"
#include "gfx/texture.hpp"

namespace bty {

Text::~Text()
{
    if (_vao != GL_NONE) {
        glDeleteVertexArrays(1, &_vao);
    }
    if (_vbo != GL_NONE) {
        glDeleteBuffers(1, &_vbo);
    }
}

Text::Text(Text &&other)
    : Transformable(other)
{
    _string = other._string;
    _numVerts = other._numVerts;
    _font = other._font;

    /* Move constructor to prevent automatic destruction
        of the OpenGL resources. */
    _vao = other._vao;
    _vbo = other._vbo;
    other._vao = GL_NONE;
    other._vbo = GL_NONE;
}

Text::Text()
{
    glCreateVertexArrays(1, &_vao);
}

void Text::create(int x, int y, const std::string &string)
{
    _font = &Textures::instance().getFont();
    setString(string);
    setPosition({x * 8.0f, y * 8.0f});
}

void Text::setString(const std::string &string)
{
    if (_string == string) {
        return;
    }

    bool same = _string == string;
    _string = string;

    if (!same) {
        updateVbo();
    }
}

GLuint Text::getVao() const
{
    return _vao;
}

GLuint Text::getNumVerts() const
{
    return _numVerts;
}

void Text::setFont(const Font &font)
{
    _font = &font;
}

const Font *Text::getFont() const
{
    return _font;
}

const std::string Text::getString() const
{
    return _string;
}

void Text::updateVbo()
{
    assert(_font);

    _numVerts = 6 * static_cast<GLuint>(_string.size());

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 texCoord;
    };

    std::vector<Vertex> vertices(_numVerts);

    float x = 0;
    float y = 0;

    for (size_t i = 0u, s = _string.size(); i < s; i++) {
        switch (_string[i]) {
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

        uint16_t char_code = (_string[i] - 32) & 0xFF;
        const auto texCoords = _font->getTexCoords(char_code);

        auto *vertex = &vertices.data()[i * 6];

        *vertex++ = {{x, y}, texCoords[0]};
        *vertex++ = {{x + 8, y}, texCoords[1]};
        *vertex++ = {{x, y + 8}, texCoords[2]};
        *vertex++ = {{x + 8, y}, texCoords[3]};
        *vertex++ = {{x + 8, y + 8}, texCoords[4]};
        *vertex++ = {{x, y + 8}, texCoords[5]};

        x += 8;
    }

    if (_vbo != GL_NONE) {
        glDeleteBuffers(1, &_vbo);
    }

    glGenBuffers(1, &_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const void *)(sizeof(GLfloat) * 2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(GL_NONE);
}

void Text::hide()
{
    _visible = false;
}

void Text::show()
{
    _visible = true;
}

bool Text::visible() const
{
    return _visible;
}

}    // namespace bty
