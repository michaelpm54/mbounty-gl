#include "game/wizard.hpp"

#include <spdlog/spdlog.h>

#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/hud.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

inline constexpr const char *const kWizardGreeting {
    R"raw(   The venerable Archmage
Aurange, will teach you the
secrets of spell casting for
   only 5000 gold pieces.)raw",
};

inline constexpr const char *const kWizardScold {
    R"raw(  The sign said 5000 gold!
 Why waste my valuable time
when you know you don't have
the required amount of gold?

    Begone until you do!)raw",
};

Wizard::Wizard(bty::Engine &engine)
    : _engine(engine)
{
}

void Wizard::load()
{
    _spUnit.setPosition(64, 104);
    _spUnit.setTexture(Textures::instance().get("units/6.png", {2, 2}));
    _spBg.setPosition(8, 24);
    _spBg.setTexture(Textures::instance().get("bg/cave.png"));
    _dlgWizard.create(1, 18, 30, 9);
    _dlgWizard.addString(1, 1, kWizardGreeting);
    _dlgWizard.addOption(13, 6, "Accept");
    _dlgWizard.addOption(13, 7, "Decline");
    _dlgWizard.bind(Key::Enter, std::bind(&Wizard::handleDialogOption, this, std::placeholders::_1));
    _dlgWizard.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        SceneMan::instance().back();
    });
    _dlgScold.create(1, 18, 30, 9);
    _dlgScold.addString(1, 1, kWizardScold);
    _dlgScold.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
    });
    _dlgScold.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
    });
}

void Wizard::handleDialogOption(int opt)
{
    if (opt == 0) {
        if (State::gold >= 5000) {
            State::gold -= 5000;
            State::magic = true;
            _engine.getGUI().getHUD().setGold(State::gold);
            _engine.getGUI().getHUD().setMagic(true);
            _engine.getGUI().popDialog();
            _engine.acceptWizardOffer();
        }
        else {
            _engine.getGUI().pushDialog(_dlgScold);
        }
    }
    else {
        _engine.getGUI().popDialog();
        SceneMan::instance().back();
    }
}

void Wizard::render()
{
    GFX::instance().drawSprite(_spBg);
    GFX::instance().drawSprite(_spUnit);
}

void Wizard::enter()
{
    auto color {bty::getBoxColor(State::difficulty)};
    _dlgWizard.setColor(color);
    _dlgScold.setColor(color);
    _engine.getGUI().pushDialog(_dlgWizard);
}

void Wizard::update(float dt)
{
    _spUnit.update(dt);
}

bool Wizard::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool Wizard::handleKey(Key key)
{
    switch (key) {
        case Key::Backspace:
            _engine.getGUI().popDialog();
            SceneMan::instance().back();
            break;
        default:
            return false;
    }
    return true;
}
