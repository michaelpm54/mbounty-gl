#ifndef BTY_GAME_VICTORY_HPP_
#define BTY_GAME_VICTORY_HPP_

#include "engine/component.hpp"
#include "engine/textbox.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

class Victory : public Component {
public:
    Victory(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    void update(float dt) override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    bty::Sprite _spBg;
    bty::Sprite _spUnits[25];
    bty::Sprite _spHero;
    bty::Sprite _spImage;
    bty::TextBox _message;
    bty::Text *_btName;
    bool _inParade {true};
    bool _pressedEnterOnce {false};
};

#endif    // BTY_GAME_VICTORY_HPP_
