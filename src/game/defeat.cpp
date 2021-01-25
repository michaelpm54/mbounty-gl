#include "game/defeat.hpp"

#include "data/hero.hpp"
#include "engine/engine.hpp"
#include "game/hud.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

static constexpr const char *const kMessage {
    "Oh, {}\n"
    "you have failed to\n"
    "recover the\n"
    "Sceptre of Order\n"
    "in time to save\n"
    "the land! Beloved\n"
    "King Maximus has\n"
    "died and the Demon\n"
    "King Urthrax\n"
    "Killspite rules in\n"
    "his place.  The\n"
    "Four Continents\n"
    "lay in ruin about\n"
    "you, its people\n"
    "doomed to a life\n"
    "of misery and\n"
    "oppression because\n"
    "you could not find\n"
    "the Sceptre."
    "\n"
    "Final Score: {:>5}",
};

Defeat::Defeat(bty::Engine &engine)
    : _engine(engine)
{
}

void Defeat::load()
{
    _spImage.setTexture(Textures::instance().get("bg/king-dead.png"));
    _spImage.setPosition(168, 24);
    _message.create(1, 3, 20, 24);
    _btName = _message.addString(1, 2);
}

void Defeat::enter()
{
    _pressedEnterOnce = false;
    _engine.getGUI().getHUD().setBlankFrame();
    _message.setColor(bty::getBoxColor(State::difficulty));
    _btName->setString(fmt::format(kMessage, kShortHeroNames[State::hero], State::score));
}

void Defeat::render()
{
    GFX::instance().drawSprite(_spImage);
    _message.render();
}

bool Defeat::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool Defeat::handleKey(Key key)
{
    switch (key) {
        case Key::Enter:
            if (!_pressedEnterOnce) {
                _pressedEnterOnce = true;
                _engine.getGUI().getHUD().setTitle("      Press Enter to play again.");
            }
            else {
                _engine.getGUI().hideHUD();
                SceneMan::instance().setScene("intro", true);
            }
            break;
        default:
            return false;
    }
    return true;
}
