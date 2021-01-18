#include "game/map.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/type_ptr.hpp>

#include "engine/texture-cache.hpp"
#include "gfx/shader.hpp"
#include "gfx/texture.hpp"

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

Map::~Map()
{
    glDeleteVertexArrays(4, _vaos);
    glDeleteBuffers(4, _vbos);
    glDeleteProgram(_shader);
}

void Map::load()
{
    for (int i = 0; i < 10; i++) {
        _texTilesets[i] = Textures::instance().get(fmt::format("tilesets/tileset{}.png", i));
    }

    _numVerts = 4096 * 6;

    static constexpr const char *const kContinentNames[4] = {
        "maps/continentia.bin",
        "maps/forestria.bin",
        "maps/archipelia.bin",
        "maps/saharia.bin",
    };

    glCreateBuffers(4, _vbos);
    glCreateVertexArrays(4, _vaos);

    auto &textures {Textures::instance()};

    for (int i = 0; i < 4; i++) {
        const std::string &file_path = fmt::format("{}/{}", textures.getBasePath(), kContinentNames[i]);

        _tiles[i].resize(4096);
        _readOnlyTiles[i].resize(4096);
        FILE *mapStream = fopen(file_path.c_str(), "rb");
        if (!mapStream) {
            spdlog::error("Failed to load map: {}", file_path);
            _numVerts = 0;
            return;
        }
        fread(_tiles[i].data(), 1, 4096, mapStream);
        fclose(mapStream);
        std::copy(_tiles[i].begin(), _tiles[i].end(), _readOnlyTiles[i].begin());

        glNamedBufferStorage(_vbos[i], 4096 * 6 * sizeof(GLfloat) * 4, nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindVertexArray(_vaos[i]);
        glBindBuffer(GL_ARRAY_BUFFER, _vbos[i]);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, nullptr);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (const void *)(2 * sizeof(GLfloat)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindVertexArray(GL_NONE);
    }

    const auto &basePath = textures.getBasePath();

    _shader = bty::loadShader(fmt::format("{}/shaders/map.glsl.vert", basePath), fmt::format("{}/shaders/map.glsl.frag", basePath));
    if (_shader == GL_NONE) {
        spdlog::warn("Map: Failed to load shader");
    }
    else {
        _viewLoc = glGetUniformLocation(_shader, "camera");
        _texLoc = glGetUniformLocation(_shader, "image");
    }
}

void Map::draw(const glm::mat4 &camera)
{
    glProgramUniformMatrix4fv(_shader, _viewLoc, 1, GL_FALSE, glm::value_ptr(camera));
    glProgramUniform1i(_shader, _texLoc, 0);

    glUseProgram(_shader);
    glBindVertexArray(_vaos[_continent]);
    glBindTextureUnit(0, _texTilesets[_curTilesetIndex]->handle);
    glDrawArrays(GL_TRIANGLES, 0, _numVerts);
    glBindVertexArray(GL_NONE);
    glUseProgram(GL_NONE);
}

void Map::update(float dt)
{
    _tilesetAnimTimer += dt;
    if (_tilesetAnimTimer >= 0.18f) {
        _tilesetAnimTimer = 0;
        _curTilesetIndex = (_curTilesetIndex + 1) % 10;
    }
}

Tile Map::getTile(int tx, int ty, int continent) const
{
    if (tx < 0 || tx > 63 || ty < 0 || ty > 63) {
        return {-1, -1, -1};
    }

    return {tx, ty, _tiles[continent][ty * 64 + tx]};
}

Tile Map::getTile(float x, float y, int continent) const
{
    int tx = static_cast<int>(floor(x)) / 48;
    int ty = static_cast<int>(floor(y)) / 40;

    return getTile(tx, ty, continent);
}

Tile Map::getTile(glm::vec2 pos, int continent) const
{
    return getTile(pos.x, pos.y, continent);
}

Tile Map::getTile(glm::ivec2 coord, int continent) const
{
    return getTile(coord.x, coord.y, continent);
}

unsigned char *Map::getTiles(int continent)
{
    return _tiles[continent].data();
}

void Map::createGeometry()
{
    for (int continent = 0; continent < 4; continent++) {
        float texAdvX = 1.0f / (_texTilesets[0]->width / 50.0f);
        float texAdvY = 1.0f / (_texTilesets[0]->height / 42.0f);

        float pxOfsX = 1.0f / _texTilesets[0]->width;
        float pxOfsY = 1.0f / _texTilesets[0]->height;

        std::vector<Vertex> vertices(4096 * 6);

        auto *vtx = &vertices[0];

        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
                float l = i * 48.0f;
                float t = j * 40.0f;
                float r = (i + 1) * 48.0f;
                float b = (j + 1) * 40.0f;

                int tileId = _tiles[continent][j * 64 + i];
                int tileX = tileId % 16;
                int tileY = tileId / 16;

                float ua = tileX * texAdvX + pxOfsX;
                float ub = (tileX + 1) * texAdvX - pxOfsX;
                float va = tileY * texAdvY + pxOfsY;
                float vb = (tileY + 1) * texAdvY - pxOfsY;

                *vtx++ = {{l, t}, {ua, va}};
                *vtx++ = {{r, t}, {ub, va}};
                *vtx++ = {{l, b}, {ua, vb}};
                *vtx++ = {{r, t}, {ub, va}};
                *vtx++ = {{r, b}, {ub, vb}};
                *vtx++ = {{l, b}, {ua, vb}};
            }
        }

        glNamedBufferSubData(_vbos[continent], 0, 4096 * 6 * sizeof(GLfloat) * 4, vertices.data());
    }
}

void Map::reset()
{
    for (int i = 0; i < 4; i++) {
        std::copy(_readOnlyTiles[i].begin(), _readOnlyTiles[i].end(), _tiles[i].begin());
    }
}

void Map::setTile(const Tile &tile, int continent, int id)
{
    if (tile.tx < 0 || tile.tx > 63 || tile.ty < 0 || tile.ty > 63) {
        return;
    }

    _tiles[continent][tile.tx + tile.ty * 64] = id;

    float texAdvX = 1.0f / (_texTilesets[0]->width / 50.0f);
    float texAdvY = 1.0f / (_texTilesets[0]->height / 42.0f);

    float pxOfsX = 1.0f / _texTilesets[0]->width;
    float pxOfsY = 1.0f / _texTilesets[0]->height;

    Vertex vertices[6];

    Vertex *vtx {&vertices[0]};

    float l = tile.tx * 48.0f;
    float t = tile.ty * 40.0f;
    float r = (tile.tx + 1) * 48.0f;
    float b = (tile.ty + 1) * 40.0f;

    int tileX = id % 16;
    int tileY = id / 16;

    float ua = tileX * texAdvX + pxOfsX;
    float ub = (tileX + 1) * texAdvX - pxOfsX;
    float va = tileY * texAdvY + pxOfsY;
    float vb = (tileY + 1) * texAdvY - pxOfsY;

    *vtx++ = {{l, t}, {ua, va}};
    *vtx++ = {{r, t}, {ub, va}};
    *vtx++ = {{l, b}, {ua, vb}};
    *vtx++ = {{r, t}, {ub, va}};
    *vtx++ = {{r, b}, {ub, vb}};
    *vtx++ = {{l, b}, {ua, vb}};

    auto size = 6 * sizeof(Vertex);
    auto offset = (tile.ty + tile.tx * 64) * size;

    glNamedBufferSubData(_vbos[continent], offset, size, vertices);
}

void Map::setContinent(int continent)
{
    _continent = continent;
}
