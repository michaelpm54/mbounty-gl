#ifndef BTY_ENGINE_SCENE_HPP_
#define BTY_ENGINE_SCENE_HPP_

#include <glm/mat4x4.hpp>

namespace bty {

class Gfx;

class Scene {
public:
    virtual ~Scene() = default;
    virtual void key(int key, int action) = 0;
    virtual void draw(Gfx &gfx, glm::mat4 &camera) = 0;
    virtual void update(float dt) = 0;
};

}    // namespace bty

#endif    // BTY_ENGINE_SCENE_HPP_
