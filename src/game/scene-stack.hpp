#ifndef BTY_GAME_SCENE_STACK_HPP_
#define BTY_GAME_SCENE_STACK_HPP_

#include <functional>
#include <glm/mat4x4.hpp>
#include <queue>

namespace bty {
class Assets;
class Gfx;
}    // namespace bty

class Scene;

class SceneStack {
public:
    SceneStack(bty::Assets &assets);
    bty::Assets &get_assets();
    void key(int key, int action);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    void push(Scene *scene, std::function<void(int)> callback);
    void pop(int ret);
    int size() const;
    const Scene *get() const;

private:
    bty::Assets &assets_;
    std::vector<Scene *> stack_;
    std::vector<std::function<void(int)>> callbacks_;
};

#endif    // BTY_GAME_SCENE_STACK_HPP_
