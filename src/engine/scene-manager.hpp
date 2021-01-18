#ifndef BTY_ENGINE_SCENE_MANAGER_HPP_
#define BTY_ENGINE_SCENE_MANAGER_HPP_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "engine/events.hpp"
#include "engine/singleton.hpp"

class Component;

namespace bty {

class Gfx;

class SceneManager {
public:
    void init(const std::vector<std::pair<std::string, Component *>> &sceneList);
    void deinit();

    bool handleEvent(Event event);
    void render();
    void update(float dt);
    void back();
    void renderLate();
    void setScene(std::string name);
    Component *getLastScene();
    std::string getLastSceneName() const;
    Component *getScene(std::string name);

private:
    std::unordered_map<std::string, Component *> _sceneMap;
    Component *_curScene {nullptr};
    std::string _curSceneName {"none"};
    std::string _lastSceneName {"none"};
};

}    // namespace bty

using SceneMan = bty::SingletonProvider<bty::SceneManager>;

#endif    // BTY_ENGINE_SCENE_MANAGER_HPP_
