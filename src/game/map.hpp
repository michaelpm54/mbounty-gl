#ifndef BTY_GAME_MAP_HPP_
#define BTY_GAME_MAP_HPP_

#include <array>
#include <glm/mat4x4.hpp>
#include <vector>

#include "gfx/gl.hpp"

namespace bty {
struct Texture;
}    // namespace bty

struct Tile {
    int tx;
    int ty;
    int id;
};

class Map {
public:
    ~Map();
    void setContinent(int continent);
    void load();
    void draw(const glm::mat4 &camera);
    void update(float dt);
    Tile getTile(int tx, int ty, int continent) const;
    Tile getTile(float x, float y, int continent) const;
    Tile getTile(glm::vec2 pos, int continent) const;
    Tile getTile(glm::ivec2 coord, int continent) const;
    unsigned char *getTiles(int continent);
    void createGeometry();
    void reset();
    void setTile(const Tile &tile, int continent, int id);

private:
    int _continent {0};
    int _numVerts {0};
    GLuint _vbos[4] {GL_NONE};
    GLuint _vaos[4] {GL_NONE};
    GLuint _shader {GL_NONE};
    GLint _viewLoc {-1};
    GLint _texLoc {-1};
    const bty::Texture *_texTilesets[10] {nullptr};
    float _tilesetAnimTimer {0};
    int _curTilesetIndex {0};
    std::array<std::vector<unsigned char>, 4> _tiles;
    std::array<std::vector<unsigned char>, 4> _readOnlyTiles;
};

#endif    // BTY_GAME_MAP_HPP_
