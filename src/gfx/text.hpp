#ifndef BTY_GFX_TEXT_HPP_
#define BTY_GFX_TEXT_HPP_

#include <string>

#include "gfx/texture.hpp"
#include "gfx/transformable.hpp"

namespace bty {

class Font;

class Text : public Transformable {
public:
    Text();
    virtual ~Text();
    Text(Text &&other);

    void create(int x, int y, const std::string &string);
    void setString(const std::string &string);
    const std::string getString() const;
    GLuint getVao() const;
    GLuint getNumVerts() const;
    void setFont(const Font &font);
    const Font *getFont() const;
    void hide();
    void show();
    bool visible() const;

private:
    void updateVbo();

private:
    GLuint _vbo {GL_NONE};
    GLuint _vao {GL_NONE};
    GLuint _numVerts {0};
    std::string _string {""};
    const Font *_font {nullptr};
    bool _visible {true};
};

}    // namespace bty

#endif    // BTY_GFX_TEXT_HPP_
