#ifndef BTY_GFX_TEXT_HPP_
#define BTY_GFX_TEXT_HPP_

#include <string>

#include "gfx/texture.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Font;

class Text : public Transformable {
public:
    ~Text();
    void create(int x, int y, const std::string &string, const Font &font);
    void set_string(const std::string &string);
    GLuint get_vao() const;
    GLuint get_num_vertices() const;
    const Font *get_font() const;

private:
    const Font *font_{nullptr};
    GLuint vao_{GL_NONE};
    GLuint num_vertices_{0};
    std::string string_;
};

}

#endif // BTY_GFX_TEXT_HPP_
