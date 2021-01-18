#include "game/game-controls.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "game/game-options.hpp"
#include "game/state.hpp"

GameControls::GameControls(bty::Engine &engine)
    : _engine(engine)
{
}

void GameControls::load()
{
    _dlg.create(6, 10, 20, 9);
    _dlg.addString(4, 1, "Game Control");
    _dlg.addString(4, 2, "____________");
    _optMusic = _dlg.addOption(4, 4);
    _optSound = _dlg.addOption(4, 5);
    _optDelay = _dlg.addOption(4, 6);
    updateOptions();
}

bool GameControls::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool GameControls::handleKey(Key key)
{
    auto &options {_engine.getGameOptions()};
    switch (key) {
        case Key::Up:
            _dlg.prev();
            break;
        case Key::Down:
            _dlg.next();
            break;
        case Key::Left:
            if (_dlg.getSelection() == 2) {
                options.combat_delay = (options.combat_delay - 1 + 10) % 10;
            }
            break;
        case Key::Right:
            if (_dlg.getSelection() == 2) {
                options.combat_delay = (options.combat_delay + 1) % 10;
            }
            break;
        case Key::Enter:
            switch (_dlg.getSelection()) {
                case 0:
                    options.music = !options.music;
                    break;
                case 1:
                    options.sound = !options.sound;
                    break;
                case 2:
                    options.combat_delay = (options.combat_delay + 1) % 10;
                    break;
                default:
                    break;
            }
            break;
        case Key::Backspace:
            SceneMan::instance().back();
            break;
        default:
            return false;
    }

    updateOptions();

    return true;
}

void GameControls::updateOptions()
{
    auto &options {_engine.getGameOptions()};
    _optMusic->setString(fmt::format("Music {}", options.music ? "on" : "off"));
    _optSound->setString(fmt::format("Sound {}", options.sound ? "on" : "off"));
    _optDelay->setString(fmt::format("Delay {}", options.combat_delay));
}

void GameControls::update(float dt)
{
    SceneMan::instance().getLastScene()->update(dt);
    _dlg.update(dt);
}

void GameControls::render()
{
    SceneMan::instance().getLastScene()->render();
    auto view {glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f)};
    GFX::instance().setView(view);
    _dlg.render();
}

void GameControls::enter()
{
    _dlg.setColor(bty::getBoxColor(State::difficulty));
    _dlg.setCellPosition(SceneMan::instance().getLastSceneName() == "battle" ? 10 : 6, 10);
}
