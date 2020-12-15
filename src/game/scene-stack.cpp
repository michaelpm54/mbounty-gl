#include "game/scene-stack.hpp"

#include <spdlog/spdlog.h>

#include "game/scene.hpp"

SceneStack::SceneStack(bty::Assets &assets)
    : assets_(assets)
{
}

void SceneStack::push(Scene *scene, std::function<void(int)> callback)
{
    if (stack_.size() && stack_.back() == scene) {
        return;
    }
    callbacks_.push_back(callback);
    stack_.push_back(scene);
}

void SceneStack::pop(int ret)
{
    if (stack_.empty()) {
        spdlog::warn("SceneStack::pop: stack empty");
        return;
    }

    stack_.pop_back();

    auto callback = callbacks_.back();
    callbacks_.pop_back();

    if (callback) {
        callback(ret);
    }
}

void SceneStack::key(int key, int action)
{
    if (stack_.empty()) {
        return;
    }

    stack_.back()->key(key, action);
}

void SceneStack::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    if (stack_.empty()) {
        return;
    }

    for (auto &scene : stack_) {
        scene->draw(gfx, camera);
    }
}

void SceneStack::update(float dt)
{
    if (stack_.empty()) {
        return;
    }

    for (auto *scene : stack_) {
        scene->update(dt);
    }
}

bty::Assets &SceneStack::get_assets()
{
    return assets_;
}

int SceneStack::size() const
{
    return static_cast<int>(stack_.size());
}

const Scene *SceneStack::get() const
{
    return stack_.size() ? stack_.back() : nullptr;
}