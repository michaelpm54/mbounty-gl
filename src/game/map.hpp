#ifndef BTY_GAME_MAP_HPP_
#define BTY_GAME_MAP_HPP_

#include "gfx/gl.hpp"

#include <glm/mat4x4.hpp>

namespace bty {
class Assets;
class Texture;
}

class Map {
public:
    ~Map();
    void load(bty::Assets &assets, unsigned char *data);
    void draw(glm::mat4 &camera);
    void update(float dt);

private:
    GLuint vao_{GL_NONE};
    GLuint program_{GL_NONE};
    GLint camera_loc_{-1};
    GLint texture_loc_{-1};
    const bty::Texture *tilesets_[10]{nullptr};
    float tileset_anim_timer_{0};
    int tileset_index_{0};
};

#endif // BTY_GAME_MAP_HPP_
