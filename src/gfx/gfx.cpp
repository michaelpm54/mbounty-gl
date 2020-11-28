#include "gfx/gfx.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/type_ptr.hpp>

#include "gfx/font.hpp"
#include "gfx/rect.hpp"
#include "gfx/shader.hpp"
#include "gfx/text.hpp"
#include "gfx/sprite.hpp"

namespace bty {

Gfx::Gfx()
{
    set_initial_gl_state();
    load_shaders();
    get_uniform_locations();
    create_geometry();
}

Gfx::~Gfx()
{
    glDeleteProgram(sprite_shader_);
    glDeleteProgram(rect_shader_);
    glDeleteProgram(text_shader_);
    glDeleteVertexArrays(1, &quad_vao_);
}

void Gfx::clear()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void Gfx::draw_sprite(Sprite &sprite, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(sprite_shader_, locations_[Locations::SpriteTransform], 1, GL_FALSE, glm::value_ptr(sprite.get_transform()));
    glProgramUniformMatrix4fv(sprite_shader_, locations_[Locations::SpriteCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(sprite_shader_, locations_[Locations::SpriteTexture], 0);
    glProgramUniform1i(sprite_shader_, locations_[Locations::SpriteFrame], sprite.get_frame());

    glUseProgram(sprite_shader_);
    glBindVertexArray(quad_vao_);
    if (sprite.get_texture())
        glBindTextureUnit(0, sprite.get_texture()->handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Gfx::draw_rect(Rect &rect, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(rect_shader_, locations_[Locations::RectTransform], 1, GL_FALSE, glm::value_ptr(rect.get_transform()));
    glProgramUniformMatrix4fv(rect_shader_, locations_[Locations::RectCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform4fv(rect_shader_, locations_[Locations::RectColor], 1, glm::value_ptr(rect.get_color()));

    glUseProgram(rect_shader_);
    glBindVertexArray(quad_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Gfx::draw_text(Text &text, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(text_shader_, locations_[Locations::TextTransform], 1, GL_FALSE, glm::value_ptr(text.get_transform()));
    glProgramUniformMatrix4fv(text_shader_, locations_[Locations::TextCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(text_shader_, locations_[Locations::TextTexture], 0);

    glUseProgram(text_shader_);
    glBindVertexArray(text.get_vao());
    if (text.get_font() && text.get_font()->get_texture())
        glBindTextureUnit(0, text.get_font()->get_texture()->handle);
    glDrawArrays(GL_TRIANGLES, 0, text.get_num_vertices());
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Gfx::get_uniform_locations()
{
    locations_[Locations::SpriteTransform] = glGetUniformLocation(sprite_shader_, "transform");
    locations_[Locations::SpriteCamera] = glGetUniformLocation(sprite_shader_, "camera");
    locations_[Locations::SpriteTexture] = glGetUniformLocation(sprite_shader_, "image");
    locations_[Locations::SpriteFrame] = glGetUniformLocation(sprite_shader_, "frame");
    locations_[Locations::RectTransform] = glGetUniformLocation(rect_shader_, "transform");
    locations_[Locations::RectCamera] = glGetUniformLocation(rect_shader_, "camera");
    locations_[Locations::RectColor] = glGetUniformLocation(rect_shader_, "fill_color");
    locations_[Locations::TextTransform] = glGetUniformLocation(text_shader_, "transform");
    locations_[Locations::TextCamera] = glGetUniformLocation(text_shader_, "camera");
    locations_[Locations::TextTexture] = glGetUniformLocation(text_shader_, "image");

    for (int i = 0; i < Locations::Count; i++) {
        if (locations_[i] == -1) {
            spdlog::warn("Uniform {} not found", i);
        }
    }
}

void Gfx::load_shaders() {
    sprite_shader_ = load_shader("data/shaders/sprite.glsl.vert", "data/shaders/sprite.glsl.frag");
    if (sprite_shader_ == GL_NONE) {
        spdlog::warn("Gfx::load_shaders: Failed to load sprite shader");
    }

    rect_shader_ = load_shader("data/shaders/rect.glsl.vert", "data/shaders/rect.glsl.frag");
    if (rect_shader_ == GL_NONE) {
        spdlog::warn("Gfx::load_shaders: Failed to load rect shader");
    }
    
    text_shader_ = load_shader("data/shaders/text.glsl.vert", "data/shaders/text.glsl.frag");
    if (text_shader_ == GL_NONE) {
        spdlog::warn("Gfx::load_shaders: Failed to load text shader");
    }
}

void Gfx::set_initial_gl_state() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.3f, 0.6f, 0.3f, 1.0f);
}

void Gfx::create_geometry() {
    GLuint quad_vbo;
    glGenBuffers(1, &quad_vbo);

    /* clang-format off */
    GLfloat quad_vertices[] = {
        /* xy */ 0.0f, 0.0f,
        /* xy */ 1.0f, 0.0f,
        /* xy */ 0.0f, 1.0f,

        /* xy */ 1.0f, 0.0f,
        /* xy */ 1.0f, 1.0f,
        /* xy */ 0.0f, 1.0f,
    };
    /* clang-format on */

    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, 48, quad_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glCreateVertexArrays(1, &quad_vao_);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(GL_NONE);

    glDeleteBuffers(1, &quad_vbo);
}

}    // namespace bty
