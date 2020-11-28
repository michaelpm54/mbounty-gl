#include "gfx/gfx.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/type_ptr.hpp>

#include "gfx/rect.hpp"
#include "gfx/shader.hpp"
#include "gfx/sprite.hpp"

namespace bty {

void gfx_set_initial_gl_state(Gfx *gfx);
void gfx_load_shaders(Gfx *gfx);
void gfx_get_uniform_locations(Gfx *gfx);
void gfx_create_geometry(Gfx *gfx);

Gfx *gfx_init()
{
    Gfx *gfx = new Gfx();

    gfx_set_initial_gl_state(gfx);
    gfx_load_shaders(gfx);
    gfx_get_uniform_locations(gfx);
    gfx_create_geometry(gfx);

    return gfx;
}

void gfx_free(Gfx *gfx)
{
    glDeleteProgram(gfx->sprite_shader);
    glDeleteProgram(gfx->rect_shader);
    glDeleteVertexArrays(1, &gfx->quad_vao);
    delete gfx;
}

void gfx_clear(Gfx *gfx)
{
    (void)gfx;
    glClear(GL_COLOR_BUFFER_BIT);
}

void gfx_draw_sprite(Gfx *gfx, Sprite &sprite, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(gfx->sprite_shader, gfx->locations[Locations::SpriteTransform], 1, GL_FALSE, glm::value_ptr(sprite.get_transform()));
    glProgramUniformMatrix4fv(gfx->sprite_shader, gfx->locations[Locations::SpriteCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(gfx->sprite_shader, gfx->locations[Locations::SpriteTexture], 0);

    glUseProgram(gfx->sprite_shader);
    glBindVertexArray(gfx->quad_vao);
    if (sprite.get_texture())
        glBindTextureUnit(0, sprite.get_texture()->handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void gfx_draw_rect(Gfx *gfx, Rect &rect, glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(gfx->rect_shader, gfx->locations[Locations::RectTransform], 1, GL_FALSE, glm::value_ptr(rect.get_transform()));
    glProgramUniformMatrix4fv(gfx->rect_shader, gfx->locations[Locations::RectCamera], 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform4fv(gfx->rect_shader, gfx->locations[Locations::RectColor], 1, glm::value_ptr(rect.get_color()));

    glUseProgram(gfx->rect_shader);
    glBindVertexArray(gfx->quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void gfx_get_uniform_locations(Gfx *gfx)
{
    gfx->locations[Locations::SpriteTransform] = glGetUniformLocation(gfx->sprite_shader, "transform");
    gfx->locations[Locations::SpriteCamera] = glGetUniformLocation(gfx->sprite_shader, "camera");
    gfx->locations[Locations::SpriteTexture] = glGetUniformLocation(gfx->sprite_shader, "image");
    gfx->locations[Locations::RectTransform] = glGetUniformLocation(gfx->rect_shader, "transform");
    gfx->locations[Locations::RectCamera] = glGetUniformLocation(gfx->rect_shader, "camera");
    gfx->locations[Locations::RectColor] = glGetUniformLocation(gfx->rect_shader, "fill_color");

    for (int i = 0; i < Locations::Count; i++) {
        if (gfx->locations[i] == -1) {
            spdlog::warn("Uniform {} not found", i);
        }
    }
}

void gfx_load_shaders(Gfx *gfx) {
    gfx->sprite_shader = load_shader("data/shaders/sprite.glsl.vert", "data/shaders/sprite.glsl.frag");
    if (gfx->sprite_shader == GL_NONE) {
        spdlog::warn("gfx_load_shaders: Failed to load sprite shader");
    }

    gfx->rect_shader = load_shader("data/shaders/rect.glsl.vert", "data/shaders/rect.glsl.frag");
    if (gfx->rect_shader == GL_NONE) {
        spdlog::warn("gfx_load_shaders: Failed to load rect shader");
    }
}

void gfx_set_initial_gl_state(Gfx *gfx) {
    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.3f, 0.6f, 0.3f, 1.0f);
}

void gfx_create_geometry(Gfx *gfx) {
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

    glCreateVertexArrays(1, &gfx->quad_vao);
    glBindVertexArray(gfx->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(GL_NONE);

    glDeleteBuffers(1, &quad_vbo);
}

}    // namespace bty
