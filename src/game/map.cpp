#include "game/map.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/type_ptr.hpp>

#include "assets.hpp"
#include "gfx/shader.hpp"
#include "gfx/texture.hpp"

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

Map::~Map()
{
    glDeleteVertexArrays(4, vaos_);
    glDeleteBuffers(4, vbos_);
    glDeleteProgram(program_);
}

void Map::load(bty::Assets &assets)
{
    for (int i = 0; i < 10; i++) {
        tilesets_[i] = assets.get_texture(fmt::format("tilesets/tileset{}.png", i));
    }

    num_vertices_ = 4096 * 6;

    static constexpr const char *const kContinentNames[4] = {
        "maps/genesis/continentia.bin",
        "maps/genesis/forestria.bin",
        "maps/genesis/archipelia.bin",
        "maps/genesis/saharia.bin",
    };

    glCreateBuffers(4, vbos_);
    glCreateVertexArrays(4, vaos_);

    for (int i = 0; i < 4; i++) {
        const std::string &file_path = fmt::format("{}/{}", assets.get_base_path(), kContinentNames[i]);

        tiles_[i].resize(4096);
        read_only_tiles_[i].resize(4096);
        FILE *map_stream = fopen(file_path.c_str(), "rb");
        if (!map_stream) {
            spdlog::error("Failed to load map: {}", file_path);
            num_vertices_ = 0;
            return;
        }
        fread(tiles_[i].data(), 1, 4096, map_stream);
        fclose(map_stream);
        std::copy(tiles_[i].begin(), tiles_[i].end(), read_only_tiles_[i].begin());

        glNamedBufferStorage(vbos_[i], 4096 * 6 * sizeof(GLfloat) * 4, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindVertexArray(vaos_[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos_[i]);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const void *)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(GL_NONE);
    }

    const auto &base_path = assets.get_base_path();

    program_ = bty::load_shader(fmt::format("{}/shaders/map.glsl.vert", base_path), fmt::format("{}/shaders/map.glsl.frag", base_path));
    if (program_ == GL_NONE) {
        spdlog::warn("Map: Failed to load shader");
    }
    else {
        camera_loc_ = glGetUniformLocation(program_, "camera");
        texture_loc_ = glGetUniformLocation(program_, "image");
    }
}

void Map::draw(glm::mat4 &camera, int continent)
{
    glProgramUniformMatrix4fv(program_, camera_loc_, 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(program_, texture_loc_, 0);

    glUseProgram(program_);
    glBindVertexArray(vaos_[continent]);
    glBindTextureUnit(0, tilesets_[tileset_index_]->handle);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices_);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Map::update(float dt)
{
    tileset_anim_timer_ += dt;
    if (tileset_anim_timer_ >= 0.18f) {
        tileset_anim_timer_ = 0;
        tileset_index_ = (tileset_index_ + 1) % 10;
    }
}

Tile Map::get_tile(int tx, int ty, int continent) const
{
    if (tx < 0 || tx > 63 || ty < 0 || ty > 63) {
        return {-1, -1, -1};
    }

    return {tx, ty, tiles_[continent][ty * 64 + tx]};
}

Tile Map::get_tile(float x, float y, int continent) const
{
    int tx = x / 48.0f;
    int ty = y / 40.0f;

    return get_tile(tx, ty, continent);
}

Tile Map::get_tile(glm::vec2 pos, int continent) const
{
    return get_tile(pos.x, pos.y, continent);
}

Tile Map::get_tile(glm::ivec2 coord, int continent) const
{
    return get_tile(coord.x, coord.y, continent);
}

unsigned char *Map::get_data(int continent)
{
    return tiles_[continent].data();
}

void Map::create_geometry()
{
    for (int continent = 0; continent < 4; continent++) {
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

                int tile_id = tiles_[continent][j * 64 + i];
                int tile_x = tile_id % 16;
                int tile_y = tile_id / 16;

                float ua = tile_x * tex_adv_x + px_offset_x;
                float ub = (tile_x + 1) * tex_adv_x - px_offset_x;
                float va = tile_y * tex_adv_y + px_offset_y;
                float vb = (tile_y + 1) * tex_adv_y - px_offset_y;

                *vtx++ = {{l, t}, {ua, va}};
                *vtx++ = {{r, t}, {ub, va}};
                *vtx++ = {{l, b}, {ua, vb}};
                *vtx++ = {{r, t}, {ub, va}};
                *vtx++ = {{r, b}, {ub, vb}};
                *vtx++ = {{l, b}, {ua, vb}};
            }
        }

        glNamedBufferSubData(vbos_[continent], 0, 4096 * 6 * sizeof(GLfloat) * 4, vertices.data());
    }
}

void Map::reset()
{
    for (int i = 0; i < 4; i++) {
        std::copy(read_only_tiles_[i].begin(), read_only_tiles_[i].end(), tiles_[i].begin());
    }
}

void Map::set_tile(const Tile &tile, int continent, int id)
{
    tiles_[continent][tile.tx + tile.ty * 64] = id;

    float tex_adv_x = 1.0f / (tilesets_[0]->width / 50.0f);
    float tex_adv_y = 1.0f / (tilesets_[0]->height / 42.0f);

    float px_offset_x = 1.0f / tilesets_[0]->width;
    float px_offset_y = 1.0f / tilesets_[0]->height;

    Vertex vertices[6];

    Vertex *vtx {&vertices[0]};

    float l = tile.tx * 48.0f;
    float t = tile.ty * 40.0f;
    float r = (tile.tx + 1) * 48.0f;
    float b = (tile.ty + 1) * 40.0f;

    int tile_x = id % 16;
    int tile_y = id / 16;

    float ua = tile_x * tex_adv_x + px_offset_x;
    float ub = (tile_x + 1) * tex_adv_x - px_offset_x;
    float va = tile_y * tex_adv_y + px_offset_y;
    float vb = (tile_y + 1) * tex_adv_y - px_offset_y;

    *vtx++ = {{l, t}, {ua, va}};
    *vtx++ = {{r, t}, {ub, va}};
    *vtx++ = {{l, b}, {ua, vb}};
    *vtx++ = {{r, t}, {ub, va}};
    *vtx++ = {{r, b}, {ub, vb}};
    *vtx++ = {{l, b}, {ua, vb}};

    auto size = 6 * sizeof(Vertex);
    auto offset = (tile.ty + tile.tx * 64) * size;

    glNamedBufferSubData(vbos_[continent], offset, size, vertices);
}
