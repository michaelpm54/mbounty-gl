#ifndef BTY_ENGINE_SCENE_STACK_HPP_
#define BTY_ENGINE_SCENE_STACK_HPP_

#include <functional>
#include <glm/mat4x4.hpp>
#include <queue>

namespace bty {

class Gfx;
class Scene;

class SceneStack {
public:
    void key(int key, int action);
    void draw(Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    void push(Scene *scene, std::function<void(int)> callback);
    void pop(int ret);
    int size() const;
    const Scene *get() const;

private:
    std::vector<Scene *> stack_;
    std::vector<std::function<void(int)>> callbacks_;
};

}    // namespace bty

#endif    // BTY_ENGINE_SCENE_STACK_HPP_
