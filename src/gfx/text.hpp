#ifndef BTY_GFX_TEXT_HPP_
#define BTY_GFX_TEXT_HPP_

#include <string>

#include "gfx/texture.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Text : public Transformable {
public:
    ~Text();
    void create(int x, int y, const std::string &string, const Texture *texture);
    void set_texture(const Texture *texture);
    void set_string(const std::string &string);
    GLuint get_vao() const;
    GLuint get_num_vertices() const;
    const Texture *get_texture() const;

private:
    GLuint vao_{GL_NONE};
    GLuint num_vertices_{0};
    const Texture *texture_{nullptr};
    std::string string_;
};

}

#endif // BTY_GFX_TEXT_HPP_
