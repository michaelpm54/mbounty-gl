#include "game/map.hpp"

#include "assets.hpp"
#include "gfx/texture.hpp"
#include "gfx/shader.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <spdlog/spdlog.h>

Map::~Map()
{
    delete[] data_;
    glDeleteVertexArrays(1, &vao_);
    glDeleteProgram(program_);
}

void Map::load(bty::Assets &assets) {
    for (int i = 0; i < 10; i++) {
        tilesets_[i] = assets.get_texture(fmt::format("tilesets/tileset{}.png", i));
    }

    num_vertices_ = 4096 * 6;

    FILE *map_stream = fopen("data/maps/genesis/continentia.bin", "rb");
    if (!map_stream) {
        spdlog::error("Failed to load map");
        num_vertices_ = 0;
        return;
    }
    data_ = new unsigned char[4096];
    fread(data_, 1, 4096, map_stream);
    fclose(map_stream);

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv;
    };

    float tex_adv_x = 1.0f / (tilesets_[0]->width / 50.0f);
    float tex_adv_y = 1.0f / (tilesets_[0]->height / 42.0f);

    float px_offset_x = 1.0f / tilesets_[0]->width;
    float px_offset_y = 1.0f / tilesets_[0]->height;
    
    std::vector<Vertex> vertices(4096 * 6);

    auto *vtx = &vertices[0];

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            float l = i * 48.0f;
            float t = j * 40.0f;
            float r = (i + 1) * 48.0f;
            float b = (j + 1) * 40.0f;

            int tile_id = data_[j * 64 + i];
            int tile_x = tile_id % 16;
            int tile_y = tile_id / 16;

            float ua = tile_x * tex_adv_x + px_offset_x;
            float ub = (tile_x + 1) * tex_adv_x - px_offset_x;
            float va = tile_y * tex_adv_y + px_offset_y;
            float vb = (tile_y + 1) * tex_adv_y - px_offset_y;

            *vtx++ = { { l, t }, { ua, va } };
            *vtx++ = { { r, t }, { ub, va } };
            *vtx++ = { { l, b }, { ua, vb } };
            *vtx++ = { { r, t }, { ub, va } };
            *vtx++ = { { r, b }, { ub, vb } };
            *vtx++ = { { l, b }, { ua, vb } };
        }
    }

    GLuint map_vbo;
    glCreateBuffers(1, &map_vbo);
    glNamedBufferStorage(map_vbo, 4096 * 6 * sizeof(Vertex), vertices.data(), 0);

    glCreateVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, map_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, nullptr);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const void *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(GL_NONE);

    glDeleteBuffers(1, &map_vbo);

    program_ = bty::load_shader("data/shaders/map.glsl.vert", "data/shaders/map.glsl.frag");
    if (program_ == GL_NONE) {
        spdlog::warn("Map: Failed to load shader");
    }
    else {
        camera_loc_ = glGetUniformLocation(program_, "camera");
        texture_loc_ = glGetUniformLocation(program_, "image");
    }
}

void Map::draw(glm::mat4 &camera) {
    glProgramUniformMatrix4fv(program_, camera_loc_, 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(program_, texture_loc_, 0);

    glUseProgram(program_);
    glBindVertexArray(vao_);
    glBindTextureUnit(0, tilesets_[tileset_index_]->handle);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Map::update(float dt) {
    tileset_anim_timer_ += dt;
    if (tileset_anim_timer_ >= 0.18f) {
        tileset_anim_timer_ = 0;
        tileset_index_ = (tileset_index_ + 1) % 10;
    }
}

Tile Map::get_tile(int tx, int ty) const
{
    if (tx < 0 || tx > 63 || ty < 0 || ty > 63) {
        spdlog::warn("Map::get_tile: Tile index out of range: {}, {}", tx, ty);
        return {-1,-1,-1};
    }

    return {tx, ty, data_[ty * 64 + tx]};
}

Tile Map::get_tile(float x, float y) const
{
    int tx = x / 48.0f;
    int ty = y / 40.0f;

    return get_tile(tx, ty);
}

Tile Map::get_tile(glm::vec2 pos) const
{
    return get_tile(pos.x, pos.y);
}
