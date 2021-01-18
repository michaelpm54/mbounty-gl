#ifndef BTY_GAME_DEFEAT_HPP_
#define BTY_GAME_DEFEAT_HPP_

#include "engine/component.hpp"
#include "engine/textbox.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Engine;
}    // namespace bty

class Defeat : public Component {
public:
    Defeat(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    bty::Sprite _spImage;
    bty::TextBox _message;
    bty::Text *_btName;
    bool _pressedEnterOnce {false};
};

#endif    // BTY_GAME_DEFEAT_HPP_
