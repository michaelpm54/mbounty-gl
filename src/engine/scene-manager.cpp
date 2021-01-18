#include "engine/scene-manager.hpp"

#include <spdlog/spdlog.h>

#include "engine/component.hpp"

namespace bty {

void SceneManager::init(const std::vector<std::pair<std::string, Component *>> &sceneList)
{
    for (auto &[name, component] : sceneList) {
        assert(name != "none");
        _sceneMap[name] = component;
    }
}

void SceneManager::deinit()
{
    for (auto &[_, component] : _sceneMap) {
        component->unload();
        delete component;
    }
    _curScene = nullptr;
}

void SceneManager::back()
{
    setScene(_lastSceneName);
}

bool SceneManager::handleEvent(Event event)
{
    if (_curScene) {
        return _curScene->handleEvent(event);
    }
    return false;
}

void SceneManager::renderLate()
{
    if (_curScene) {
        _curScene->renderLate();
    }
}

void SceneManager::render()
{
    if (_curScene) {
        if (_curScene->isOverlay()) {
            if (_lastSceneName != "none") {
                _sceneMap[_lastSceneName]->render();
            }
        }
        _curScene->render();
    }
}

void SceneManager::update(float dt)
{
    if (_curScene) {
        _curScene->update(dt);
    }
}

void SceneManager::setScene(std::string name)
{
    if (!_sceneMap.contains(name)) {
        spdlog::warn("SceneManager: no component by name '{}'", name);
    }
    else {
        if (_curScene == _sceneMap[name]) {
            spdlog::warn("SceneManager: component is already '{}'", name);
        }
        else {
            spdlog::debug("SceneManager: setScene(\"{}\")", name);
            if (_curScene) {
                _curScene->unload();
            }
            _curScene = _sceneMap[name];
            _lastSceneName = _curSceneName;
            _curSceneName = name;
            _curScene->load();
            _curScene->enter();
        }
    }
}

Component *SceneManager::getLastScene()
{
    if (_lastSceneName != "none" && _sceneMap[_lastSceneName] != _curScene) {
        return _sceneMap[_lastSceneName];
    }
    return nullptr;
}

std::string SceneManager::getLastSceneName() const
{
    return _lastSceneName;
}

Component *SceneManager::getScene(std::string name)
{
    if (_sceneMap.contains(name)) {
        return _sceneMap[name];
    }
    return nullptr;
}

}    // namespace bty
