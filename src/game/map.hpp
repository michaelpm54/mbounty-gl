#ifndef BTY_GAME_MAP_HPP_
#define BTY_GAME_MAP_HPP_

#include <array>
#include <glm/mat4x4.hpp>
#include <vector>

#include "gfx/gl.hpp"

namespace bty {
class Assets;
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
    void load(bty::Assets &assets);
    void draw(glm::mat4 &camera, int continent);
    void update(float dt);
    Tile get_tile(int tx, int ty, int continent) const;
    Tile get_tile(float x, float y, int continent) const;
    Tile get_tile(glm::vec2 pos, int continent) const;
    Tile get_tile(glm::ivec2 coord, int continent) const;
    unsigned char *get_data(int continent);
    void create_geometry();
    void reset();
    void set_tile(const Tile &tile, int continent, int id);

private:
    int num_vertices_ {0};
    GLuint vbos_[4] {GL_NONE};
    GLuint vaos_[4] {GL_NONE};
    GLuint program_ {GL_NONE};
    GLint camera_loc_ {-1};
    GLint texture_loc_ {-1};
    const bty::Texture *tilesets_[10] {nullptr};
    float tileset_anim_timer_ {0};
    int tileset_index_ {0};
    std::array<std::vector<unsigned char>, 4> tiles_;
    std::array<std::vector<unsigned char>, 4> read_only_tiles_;
};

#endif    // BTY_GAME_MAP_HPP_
