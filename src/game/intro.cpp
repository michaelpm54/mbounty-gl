#include "game/intro.hpp"

#include <spdlog/spdlog.h>

#include "data/color.hpp"
#include "data/hero.hpp"
#include "engine/engine.hpp"
#include "engine/texture-cache.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

Intro::Intro(bty::Engine &engine)
    : _engine(engine)
{
}

void Intro::load()
{
    State::hero = 0;
    State::difficulty = 1;

    _spBg.setTexture(Textures::instance().get("bg/intro.png"));

    _nameBox.create(7, 1, 27, 3);
    _btName = _nameBox.addString(2, 1, kHeroNames[0][0]);
    _helpBox.create(1, 24, 38, 3);
    _btHelp = _helpBox.addString(2, 1, "Select a character and press Enter");

    _dlgDifficulty.create(7, 10, 27, 8);
    _dlgDifficulty.addString(2, 1, "Difficulty   Days  Score");
    _dlgDifficulty.addOption(3, 3, "Easy         900   x.5");
    _dlgDifficulty.addOption(3, 4, "Normal       600    x1");
    _dlgDifficulty.addOption(3, 5, "Hard         400    x2");
    _dlgDifficulty.addOption(3, 6, "Impossible?  200    x4");
    _dlgDifficulty.bind(Key::Backspace, [this](int) {
        _pickedHero = false;
        _btHelp->setString("Select a character and press Enter");
        _engine.getGUI().popDialog();
        State::difficulty = 1;
    });
    _dlgDifficulty.bind(Key::Enter, [this](int opt) {
        State::difficulty = opt;
        _engine.getGUI().popDialog();
        SceneMan::instance().setScene("ingame");
    });
}

void Intro::enter()
{
    _pickedHero = false;
    State::hero = 0;
    State::difficulty = 1;
    _dlgDifficulty.setSelection(1);
    _btName->setString(kHeroNames[State::hero][0]);
    _btHelp->setString("Select a character and press Enter");
}

void Intro::render()
{
    GFX::instance().drawSprite(_spBg);
    _nameBox.render();
    _helpBox.render();
}

bool Intro::handleEvent(Event event)
{
    switch (event.id) {
        case EventId::KeyDown:
            return handleKey(event.key);
        default:
            break;
    }
    return false;
}

bool Intro::handleKey(Key key)
{
    bool handled = true;

    if (!_pickedHero) {
        switch (key) {
            case Key::Left:
                State::hero = (State::hero + 1) % 4;
                break;
            case Key::Right:
                State::hero = (State::hero - 1 + 4) % 4;
                break;
            case Key::Enter:
                _btHelp->setString("Select a difficulty and press Enter");
                _pickedHero = true;
                _dlgDifficulty.setSelection(State::difficulty);
                _engine.getGUI().pushDialog(_dlgDifficulty);
                break;
            default:
                handled = false;
                break;
        }
        _btName->setString(kHeroNames[State::hero][0]);
    }

    return handled;
}
