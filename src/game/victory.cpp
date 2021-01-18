#include "game/victory.hpp"

#include <fmt/format.h>

#include "data/hero.hpp"
#include "engine/engine.hpp"
#include "game/hud.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

static constexpr const char *const kMessage {
    "Congratulations,\n"
    "{}!  You\n"
    "have recovered the\n"
    "Sceptre of Order\n"
    "from the clutches\n"
    "of the evil Master\n"
    "Villains. As a\n"
    "reward for saving\n"
    "himself and the\n"
    "four continents\n"
    "from ruin, King\n"
    "Maximus and his\n"
    "subjects reward\n"
    "you with a large\n"
    "parcel of land,\n"
    "a rank of nobility\n"
    "and a medal\n"
    "announcing your\n"
    "\n"
    "Final Score: {:>5}",
};

Victory::Victory(bty::Engine &engine)
    : _engine(engine)
{
}

void Victory::load()
{
    auto &textures {Textures::instance()};

    _spBg.setPosition(8, 24);
    _spBg.setTexture(textures.get("battle/encounter.png"));

    _spImage.setPosition(8, 24);
    _spImage.setTexture(textures.get("bg/king-massive-smile.png"));

    _spHero.setPosition(20.0f + 4 * 48.0f, 24.0f + 6 * 40.0f);
    _spHero.setTexture(textures.get("hero/walk-moving.png", {4, 1}));
    _spHero.setFlip(true);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int index = j * 5 + i;
            float x = 16.0f + i * 48.0f;
            float y = 24.0f + j * 40.0f;
            if (i == 4) {
                x += 48.0f;
                _spUnits[index].setFlip(true);
            }
            _spUnits[index].setPosition(x, y);
            _spUnits[index].setTexture(textures.get(fmt::format("units/{}.png", index), {2, 2}));
        }
    }

    _message.create(19, 3, 20, 24);
    _btName = _message.addString(1, 2);
}

void Victory::enter()
{
    _message.setColor(bty::getBoxColor(State::difficulty));
    _pressedEnterOnce = false;
    _btName->setString(fmt::format(kMessage, kShortHeroNames[State::hero], State::score));
}

void Victory::render()
{
    if (_inParade) {
        GFX::instance().drawSprite(_spBg);
        for (int i = 0; i < 25; i++) {
            GFX::instance().drawSprite(_spUnits[i]);
        }
        GFX::instance().drawSprite(_spHero);
    }
    else {
        GFX::instance().drawSprite(_spImage);
        _message.render();
    }
}

void Victory::update(float dt)
{
    if (_inParade) {
        for (int i = 0; i < 25; i++) {
            _spUnits[i].update(dt);
        }
        _spHero.move(0.0f, -70.0f * dt);
        _spHero.update(dt);

        if (_spHero.getPosition().y <= -140.0f) {
            _inParade = false;
        }
    }
}

bool Victory::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool Victory::handleKey(Key key)
{
    if (_inParade) {
        return false;
    }

    switch (key) {
        case Key::Enter:
            if (!_pressedEnterOnce) {
                _pressedEnterOnce = true;
                _engine.getGUI().getHUD().setTitle("      Press Enter to play again.");
            }
            else {
                _engine.getGUI().hideHUD();
                SceneMan::instance().setScene("intro");
            }
            break;
        default:
            return false;
    }

    return true;
}
