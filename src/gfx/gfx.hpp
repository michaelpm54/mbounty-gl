#ifndef BTY_GFX_GFX_HPP_
#define BTY_GFX_GFX_HPP_

#include <glm/mat4x4.hpp>

#include "engine/singleton.hpp"
#include "gfx/gl.hpp"

namespace bty {

enum Locations {
    SpriteTransform,
    SpriteCamera,
    SpriteTexture,
    SpriteFrame,
    SpriteFlip,
    SpriteSingleTextureTransform,
    SpriteSingleTextureCamera,
    SpriteSingleTextureTexture,
    SpriteSingleTextureFlip,
    SpriteSingleTextureRepeat,
    SpriteSingleTextureSize,
    RectTransform,
    RectCamera,
    RectColor,
    TextTransform,
    TextCamera,
    TextTexture,
    Count,
};

class Rect;
class Sprite;
class Text;

class Gfx {
public:
    Gfx();
    ~Gfx();
    void clear();
    void setView(const glm::mat4 &mat);
    void drawSprite(Sprite &sprite);
    void drawRect(Rect &rect);
    void drawText(Text &text);
    void drawSprite(Sprite &sprite, glm::mat4 &camera);
    void drawRect(Rect &rect, glm::mat4 &camera);
    void drawText(Text &text, glm::mat4 &camera);

private:
    void initGLState();
    void loadShaders();
    void getUniformLocations();
    void createQuadVao();

private:
    GLuint _shdSpriteMulti {GL_NONE};
    GLuint _shdSpriteSingle {GL_NONE};
    GLuint _shdRect {GL_NONE};
    GLuint _shdText {GL_NONE};
    GLuint _quadVao {GL_NONE};
    GLuint _quadVbo {GL_NONE};
    GLint _locations[Locations::Count];
    glm::mat4 _view {1.0f};
};

}    // namespace bty

using GFX = bty::SingletonProvider<bty::Gfx>;

#endif    // BTY_GFX_GFX_HPP_
