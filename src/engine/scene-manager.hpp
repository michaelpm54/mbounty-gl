#ifndef BTY_ENGINE_SCENE_MANAGER_HPP_
#define BTY_ENGINE_SCENE_MANAGER_HPP_

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "gfx/rect.hpp"
#include "engine/events.hpp"
#include "engine/singleton.hpp"

class Component;

namespace bty {

class Gfx;
struct Window;

class SceneManager {
private:
	enum class TransitionState
	{
		TransitionOut,
		TransitionIn,
		Pause,
		None,
	};

    struct Transition
    {
        TransitionState state{TransitionState::None};
        float progress;
        std::string next;
        bty::Rect fadeRect;
		std::function<void()> onTransitionIn{nullptr};
    };

public:
    void init(bty::Window *window, const std::vector<std::pair<std::string, Component *>> &sceneList);
    void deinit();

    bool handleEvent(Event event);
    void render();
    void update(float dt);
    void back();
    void renderLate();
    void setScene(std::string name, bool transition = false, std::function<void()> onTransitionIn = nullptr);
    Component *getLastScene();
    std::string getLastSceneName() const;
    Component *getScene(std::string name);

private:
    void startTransitionOut();
    void startTransitionIn();
    void updateTransitionOut(float dt);
	void updateTransitionIn(float dt);

    std::unordered_map<std::string, Component *> _sceneMap;
    Component *_curScene {nullptr};
    std::string _curSceneName {"none"};
    std::string _lastSceneName {"none"};
    Transition _transition;
};

}    // namespace bty

using SceneMan = bty::SingletonProvider<bty::SceneManager>;

#endif    // BTY_ENGINE_SCENE_MANAGER_HPP_
