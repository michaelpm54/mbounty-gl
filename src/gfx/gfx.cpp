#include "gfx/gfx.hpp"

#include <glm/gtc/type_ptr.hpp>

#include "gfx/shader.hpp"
#include "gfx/sprite.hpp"

namespace bty {

Gfx *gfx_init()
{
    Gfx *gfx = new Gfx();

    // glEnable(GL_DEPTH_TEST);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.3f, 0.6f, 0.3f, 1.0f);

    gfx->sprite_shader = load_shader("data/shaders/sprite.glsl.vert", "data/shaders/sprite.glsl.frag");
    if (gfx->sprite_shader == GL_NONE) {
        delete gfx;
        return nullptr;
    }

    gfx->locations[Locations::SpriteTransform] = glGetUniformLocation(gfx->sprite_shader, "transform");
    gfx->locations[Locations::SpriteCamera] = glGetUniformLocation(gfx->sprite_shader, "camera");
    gfx->locations[Locations::SpriteTexture] = glGetUniformLocation(gfx->sprite_shader, "image");

    GLuint sprite_vbo;
    glGenBuffers(1, &sprite_vbo);

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

    glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
    glBufferData(GL_ARRAY_BUFFER, 48, quad_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glCreateVertexArrays(1, &gfx->sprite_vao);
    glBindVertexArray(gfx->sprite_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sprite_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(GL_NONE);

    // glDeleteBuffers(1, &sprite_vbo);

    return gfx;
}

void gfx_free(Gfx *gfx)
{
    glDeleteProgram(gfx->sprite_shader);
    glDeleteVertexArrays(1, &gfx->sprite_vao);
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
    glBindVertexArray(gfx->sprite_vao);
    if (sprite.get_texture())
        glBindTextureUnit(0, sprite.get_texture()->handle);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

}    // namespace bty
