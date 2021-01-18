#include "gfx/gfx.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/type_ptr.hpp>

#include "gfx/font.hpp"
#include "gfx/rect.hpp"
#include "gfx/shader.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {

Gfx::Gfx()
{
    initGLState();
    loadShaders();
    getUniformLocations();
    createQuadVao();
}

Gfx::~Gfx()
{
    glDeleteProgram(_shdSpriteMulti);
    glDeleteProgram(_shdSpriteSingle);
    glDeleteProgram(_shdRect);
    glDeleteProgram(_shdText);
    glDeleteVertexArrays(1, &_quadVao);
    glDeleteBuffers(1, &_quadVbo);
}

void Gfx::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Gfx::drawSprite(Sprite &sprite, glm::mat4 &camera)
{
    const Texture *texture = sprite.getTexture();

    if (texture) {
        if (texture->framesX > 1 || texture->framesY > 1) {
            glProgramUniformMatrix4fv(_shdSpriteMulti, _locations[Locations::SpriteTransform], 1, GL_FALSE, glm::value_ptr(sprite.getTransform()));
            glProgramUniformMatrix4fv(_shdSpriteMulti, _locations[Locations::SpriteCamera], 1, GL_FALSE, glm::value_ptr(camera));
            glProgramUniform1i(_shdSpriteMulti, _locations[Locations::SpriteTexture], 0);
            glProgramUniform1i(_shdSpriteMulti, _locations[Locations::SpriteFrame], sprite.getFrame());
            glProgramUniform1i(_shdSpriteMulti, _locations[Locations::SpriteFlip], static_cast<int>(sprite.getFlip()));
            glUseProgram(_shdSpriteMulti);
            glBindTextureUnit(0, texture->handle);
        }
        else {
            glProgramUniformMatrix4fv(_shdSpriteSingle, _locations[Locations::SpriteSingleTextureTransform], 1, GL_FALSE, glm::value_ptr(sprite.getTransform()));
            glProgramUniformMatrix4fv(_shdSpriteSingle, _locations[Locations::SpriteSingleTextureCamera], 1, GL_FALSE, glm::value_ptr(camera));
            glProgramUniform1i(_shdSpriteSingle, _locations[Locations::SpriteSingleTextureTexture], 0);
            glProgramUniform1i(_shdSpriteSingle, _locations[Locations::SpriteSingleTextureFlip], static_cast<int>(sprite.getFlip()));
            glProgramUniform1i(_shdSpriteSingle, _locations[Locations::SpriteSingleTextureRepeat], static_cast<int>(sprite.getRepeat()));
            if (sprite.getRepeat()) {
                const auto size {sprite.getSize()};
                glm::vec2 scale = {size.x / texture->width, size.y / texture->height};
                glProgramUniform2fv(_shdSpriteSingle, _locations[Locations::SpriteSingleTextureSize], 1, glm::value_ptr(scale));
            }
            glUseProgram(_shdSpriteSingle);
            glBindTextureUnit(0, texture->handle);
        }
    }
    else {
        glBindTextureUnit(0, GL_NONE);
        return;
    }

    glBindVertexArray(_quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Gfx::drawRect(Rect &rect, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(_shdRect, _locations[Locations::RectTransform], 1, GL_FALSE, glm::value_ptr(rect.getTransform()));
    glProgramUniformMatrix4fv(_shdRect, _locations[Locations::RectCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform4fv(_shdRect, _locations[Locations::RectColor], 1, glm::value_ptr(rect.getColor()));

    glUseProgram(_shdRect);
    glBindVertexArray(_quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Gfx::drawText(Text &text, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(_shdText, _locations[Locations::TextTransform], 1, GL_FALSE, glm::value_ptr(text.getTransform()));
    glProgramUniformMatrix4fv(_shdText, _locations[Locations::TextCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(_shdText, _locations[Locations::TextTexture], 0);

    glUseProgram(_shdText);
    glBindVertexArray(text.getVao());
    if (text.getFont() && text.getFont()->getTexture())
        glBindTextureUnit(0, text.getFont()->getTexture()->handle);
    glDrawArrays(GL_TRIANGLES, 0, text.getNumVerts());
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Gfx::getUniformLocations()
{
    _locations[Locations::SpriteTransform] = glGetUniformLocation(_shdSpriteMulti, "transform");
    _locations[Locations::SpriteCamera] = glGetUniformLocation(_shdSpriteMulti, "camera");
    _locations[Locations::SpriteTexture] = glGetUniformLocation(_shdSpriteMulti, "image");
    _locations[Locations::SpriteFrame] = glGetUniformLocation(_shdSpriteMulti, "frame");
    _locations[Locations::SpriteFlip] = glGetUniformLocation(_shdSpriteMulti, "flip");
    _locations[Locations::SpriteSingleTextureTransform] = glGetUniformLocation(_shdSpriteSingle, "transform");
    _locations[Locations::SpriteSingleTextureCamera] = glGetUniformLocation(_shdSpriteSingle, "camera");
    _locations[Locations::SpriteSingleTextureTexture] = glGetUniformLocation(_shdSpriteSingle, "image");
    _locations[Locations::SpriteSingleTextureFlip] = glGetUniformLocation(_shdSpriteSingle, "flip");
    _locations[Locations::SpriteSingleTextureRepeat] = glGetUniformLocation(_shdSpriteSingle, "repeat");
    _locations[Locations::SpriteSingleTextureSize] = glGetUniformLocation(_shdSpriteSingle, "size");
    _locations[Locations::RectTransform] = glGetUniformLocation(_shdRect, "transform");
    _locations[Locations::RectCamera] = glGetUniformLocation(_shdRect, "camera");
    _locations[Locations::RectColor] = glGetUniformLocation(_shdRect, "fill_color");
    _locations[Locations::TextTransform] = glGetUniformLocation(_shdText, "transform");
    _locations[Locations::TextCamera] = glGetUniformLocation(_shdText, "camera");
    _locations[Locations::TextTexture] = glGetUniformLocation(_shdText, "image");

    for (int i = 0; i < Locations::Count; i++) {
        if (_locations[i] == -1) {
            spdlog::warn("Uniform {} not found", i);
        }
    }
}

void Gfx::loadShaders()
{
    const auto &base_path = Textures::instance().getBasePath();

    _shdSpriteMulti = loadShader(fmt::format("{}/shaders/sprite.glsl.vert", base_path), fmt::format("{}/shaders/sprite.glsl.frag", base_path));
    if (_shdSpriteMulti == GL_NONE) {
        spdlog::warn("Gfx::loadShaders: Failed to load sprite shader");
    }

    _shdSpriteSingle = loadShader(fmt::format("{}/shaders/sprite_single_texture.glsl.vert", base_path), fmt::format("{}/shaders/sprite_single_texture.glsl.frag", base_path));
    if (_shdSpriteSingle == GL_NONE) {
        spdlog::warn("Gfx::loadShaders: Failed to load _shdSpriteSingle");
    }

    _shdRect = loadShader(fmt::format("{}/shaders/rect.glsl.vert", base_path), fmt::format("{}/shaders/rect.glsl.frag", base_path));
    if (_shdRect == GL_NONE) {
        spdlog::warn("Gfx::loadShaders: Failed to load rect shader");
    }

    _shdText = loadShader(fmt::format("{}/shaders/text.glsl.vert", base_path), fmt::format("{}/shaders/text.glsl.frag", base_path));
    if (_shdText == GL_NONE) {
        spdlog::warn("Gfx::loadShaders: Failed to load text shader");
    }
}

void Gfx::initGLState()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 163 / 255.0f, 166 / 255.0f, 1.0f);
}

void Gfx::createQuadVao()
{
    if (_quadVbo != GL_NONE) {
        glDeleteBuffers(1, &_quadVbo);
    }

    glGenBuffers(1, &_quadVbo);

    /* clang-format off */
    GLfloat quadVerts[] = {
        /* xy */ 0.0f, 0.0f,
        /* xy */ 1.0f, 0.0f,
        /* xy */ 0.0f, 1.0f,

        /* xy */ 1.0f, 0.0f,
        /* xy */ 1.0f, 1.0f,
        /* xy */ 0.0f, 1.0f,
    };
    /* clang-format on */

    glBindBuffer(GL_ARRAY_BUFFER, _quadVbo);
    glBufferData(GL_ARRAY_BUFFER, 48, quadVerts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glCreateVertexArrays(1, &_quadVao);
    glBindVertexArray(_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, _quadVbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(GL_NONE);
}

void Gfx::setView(const glm::mat4 &mat)
{
    _view = mat;
}

void Gfx::drawSprite(Sprite &sprite)
{
    drawSprite(sprite, _view);
}

void Gfx::drawRect(Rect &rect)
{
    drawRect(rect, _view);
}

void Gfx::drawText(Text &text)
{
    if (text.visible()) {
        drawText(text, _view);
    }
}

}    // namespace bty
