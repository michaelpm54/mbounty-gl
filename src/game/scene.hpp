#ifndef BTY_GAME_SCENE_HPP_
#define BTY_GAME_SCENE_HPP_

#include <glm/mat4x4.hpp>

namespace bty {
class Assets;
class Gfx;
}    // namespace bty

class Scene {
public:
    virtual ~Scene() = default;
    virtual void key(int key, int action) = 0;
    virtual void draw(bty::Gfx &gfx, glm::mat4 &camera) = 0;
    virtual void update(float dt) = 0;
};

#endif    // BTY_GAME_SCENE_HPP_
