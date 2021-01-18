#ifndef BTY_GAME_VIEW_CONTINENT_HPP_
#define BTY_GAME_VIEW_CONTINENT_HPP_

#include "engine/component.hpp"
#include "engine/textbox.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

struct Variables;

class ViewContinent : public Component {
public:
    ViewContinent(bty::Engine &engine);
    void load() override;
    void unload() override;
    void render() override;
    void update(float dt) override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;
    void enter() override;

private:
    void genTexture();

private:
    bty::Engine &_engine;
    bty::TextBox _box;
    bty::Text *_btContinent {nullptr};
    bty::Text *_btCoordinates {nullptr};
    bty::Sprite _spMap;
    bty::Texture _texMap;
    glm::mat4 _view;
    float _dotTimer {0.0f};
    float _dotAlpha {0};
    bool _fogEnabled {true};
    bool _haveThisMap {false};
};

#endif    // BTY_GAME_VIEW_CONTINENT_HPP_
