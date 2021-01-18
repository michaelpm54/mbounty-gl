#ifndef BTY_GAME_INTRO_HPP_
#define BTY_GAME_INTRO_HPP_

#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "engine/textbox.hpp"
#include "game/ingame.hpp"
#include "gfx/font.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Gfx;
class Engine;
}    // namespace bty

class Intro : public Component {
public:
    Intro(bty::Engine &engine);

    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;
    void load() override;
    void enter() override;
    void render() override;

private:
    bty::Engine &_engine;
    bty::Sprite _spBg;
    bty::TextBox _nameBox;
    bty::TextBox _helpBox;
    bty::Text *_btName;
    bty::Text *_btHelp;
    bty::Dialog _dlgDifficulty;
    bool _pickedHero {false};
};

#endif    // BTY_INTRO_INTRO_HPP_
