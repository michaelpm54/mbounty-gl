#include "engine/scene-manager.hpp"

#include <spdlog/spdlog.h>
#include <glm/gtc/matrix_transform.hpp>
#include "gfx/gfx.hpp"
#include "engine/component.hpp"
#include "window/window.hpp"

namespace bty {

void SceneManager::init(bty::Window *window, const std::vector<std::pair<std::string, Component *>> &sceneList)
{
    for (auto &[name, component] : sceneList) {
        assert(name != "none");
        _sceneMap[name] = component;
    }
	_transition.fadeRect.setSize(static_cast<float>(window_width(window)), static_cast<float>(window_height(window)));
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
	if (_transition.state == TransitionState::None && _curScene) {
		return _curScene->handleEvent(event);
	}
	return false;
}

void SceneManager::renderLate()
{
    if (_curScene) {
        _curScene->renderLate();
    }

	if (_transition.state != TransitionState::None) {
		GFX::instance().setView(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f));
		GFX::instance().drawRect(_transition.fadeRect);
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
	switch (_transition.state) {
		case TransitionState::None:
			if (_curScene) {
	        	_curScene->update(dt);
    		}
			break;
		case TransitionState::TransitionOut:
			updateTransitionOut(dt);
			break;
		case TransitionState::Pause:
			if (_curScene) {
				_curScene->unload();
			}
			_lastSceneName = _curSceneName;
			_curSceneName = _transition.next;
			_curScene = _sceneMap[_transition.next];
			_curScene->load();
			_curScene->enter();
			_transition.state = TransitionState::TransitionIn;
			if (_transition.onTransitionIn) {
				_transition.onTransitionIn();
				_transition.onTransitionIn = nullptr;
			}
			break;
		case TransitionState::TransitionIn:
			updateTransitionIn(dt);
			break;
		default:
			break;
	}
}

void SceneManager::updateTransitionOut(float dt)
{
	_transition.progress += dt * 2;
	if (_transition.progress >= 1.0f) {
		_transition.fadeRect.setColor({0.0f, 0.0f, 0.0f, 1.0f});
		_transition.progress = 0.0f;
		_transition.state = TransitionState::Pause;
	}
	else {
		float alpha = _transition.progress;
		_transition.fadeRect.setColor({0.0f, 0.0f, 0.0f, alpha});
	}
}

void SceneManager::updateTransitionIn(float dt)
{
	_transition.progress += dt * 2;
	if (_transition.progress >= 1.0f) {
		_transition.state = TransitionState::None;
	}
	else {
		float alpha = 1.0f - _transition.progress;
		_transition.fadeRect.setColor({0.0f, 0.0f, 0.0f, alpha});
	}
}

void SceneManager::setScene(std::string name, bool transition, std::function<void()> onTransitionIn)
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
			if (transition) {
	    		_transition.state = TransitionState::TransitionOut;
				_transition.progress = 0.0f;
				_transition.next = name;
				_transition.onTransitionIn = onTransitionIn;
			}
			else {
				if (_curScene) {
					_curScene->unload();
				}
				_lastSceneName = _curSceneName;
				_curSceneName = name;
				_curScene = _sceneMap[name];
				_curScene->load();
				_curScene->enter();
			}
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
