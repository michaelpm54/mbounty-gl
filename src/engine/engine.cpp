#include "engine.hpp"

#include <spdlog/spdlog.h>

#include <chrono>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

#include "game/ingame.hpp"
#include "game/intro.hpp"
#include "game/save.hpp"
#include "game/use-magic.hpp"
#include "gfx/gfx.hpp"
#include "window/window.hpp"

namespace bty {

Engine::Engine(Window &window)
    : _inputLayer({.engine = this})
    , _window(&window)
    , _view(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
{
    window_init_callbacks(_window, &_inputLayer);
    _btFPSLabel.create(1, 3, "FPS: ");
    _btFPS.create(5, 3, "");
}

void Engine::run()
{
    using namespace std::chrono;

    auto &sceneMan {SceneMan::instance()};

    _ingame = new Ingame(*this);
    _battle = new Battle(*this);
    _garrison = new Garrison(*this);
    _shop = new Shop(*this);
    _town = new Town(*this);
    _saveManager = new SaveManager(*this);
    _saveManager->onLoad([this](const std::string &filename) {
        loadState(filename);
    });
    _saveManager->onSave([this](const std::string &filename) {
        saveState(filename);
    });

    sceneMan.init({
        {
            "intro",
            new Intro(*this),
        },
        {
            "ingame",
            _ingame,
        },
        {
            "battle",
            _battle,
        },
        {
            "victory",
            new Victory(*this),
        },
        {
            "defeat",
            new Defeat(*this),
        },
        {
            "viewarmy",
            new ViewArmy(*this),
        },
        {
            "viewchar",
            new ViewCharacter(*this),
        },
        {
            "viewcontinent",
            new ViewContinent(*this),
        },
        {
            "viewcontract",
            new ViewContract(*this, _gui.getHUD().getContractSprite()),
        },
        {
            "viewpuzzle",
            new ViewPuzzle(*this),
        },
        {
            "controls",
            new GameControls(*this),
        },
        {
            "wizard",
            new Wizard(*this),
        },
        {
            "kingscastle",
            new KingsCastle(*this),
        },
        {
            "garrison",
            _garrison,
        },
        {
            "shop",
            _shop,
        },
        {
            "town",
            _town,
        },
        {
            "usemagic",
            new UseMagic(*this),
        },
        {
            "save",
            _saveManager,
        },
    });

    sceneMan.setScene("intro");

    GFX::instance().setView(_view);

    auto curTime = steady_clock::now();
    int frameCount = 0;
    int frameRate = 0;

    while (_run) {
        auto lastTime = curTime;
        curTime = steady_clock::now();
        ++frameCount;
        if (time_point_cast<seconds>(curTime) != time_point_cast<seconds>(lastTime)) {
            frameRate = frameCount;
            frameCount = 0;
        }

        float dt = duration<float>(curTime - lastTime).count();

        window_events(_window);

        if (!_gui.update(dt)) {
            sceneMan.update(dt);
        }

        if (_gameOptions.debug) {
            _btFPS.setString(std::to_string(frameRate));
        }

        GFX::instance().clear();
        sceneMan.render();
        _gui.render();
        sceneMan.renderLate();

        if (_gameOptions.debug) {
            GFX::instance().drawText(_btFPSLabel);
            GFX::instance().drawText(_btFPS);
        }

        window_swap(_window);
    }

    SceneMan::instance().deinit();
}

void Engine::quit()
{
    _run = false;
}

GameOptions &Engine::getGameOptions()
{
    return _gameOptions;
}

void Engine::event(Event event)
{
    if (event.id == EventId::Quit) {
        quit();
        return;
    }
    else if (event.id == EventId::KeyDown) {
        if (event.key == Key::F1) {
            _gameOptions.debug = !_gameOptions.debug;
            return;
        }
        else if (event.key == Key::Q) {
            quit();
            return;
        }
    }

    if (!_gui.handleEvent(event)) {
        SceneMan::instance().handleEvent(event);
    }
}

GUI &Engine::getGUI()
{
    return _gui;
}

void Engine::startSiegeBattle(int castleId)
{
    SceneMan::instance().setScene("battle");
    _battle->startSiegeBattle(castleId);
}

void Engine::startEncounterBattle(int mobId)
{
    SceneMan::instance().setScene("battle");
    _battle->startEncounterBattle(mobId);
}

void Engine::winSiegeBattle(int castleId)
{
    SceneMan::instance().setScene("ingame");
    _ingame->winSiegeBattle(castleId);
}

void Engine::winEncounterBattle(int mobId)
{
    SceneMan::instance().setScene("ingame");
    _ingame->winEncounterBattle(mobId);
}

void Engine::acceptWizardOffer()
{
    SceneMan::instance().setScene("ingame");
    _ingame->acceptWizardOffer();
}

void Engine::openGarrison(int castleId)
{
    _garrison->setCastle(castleId);
    SceneMan::instance().setScene("garrison");
}

void Engine::openShop(ShopInfo &info)
{
    _shop->setShop(info);
    SceneMan::instance().setScene("shop");
}

void Engine::openTown(TownGen *info)
{
    _town->setTown(info);
    SceneMan::instance().setScene("town");
}

void Engine::setBoatPosition(float x, float y)
{
    _ingame->setBoatPosition(x, y);
}

void Engine::loseBattle()
{
    _ingame->disgrace();
    SceneMan::instance().setScene("ingame");
}

void Engine::loadState(const std::string &filename)
{
    const auto path = fmt::format("saves/{}", filename);

    spdlog::info("Loading from {}", path);

    std::ifstream f(path, std::ios::in | std::ios::binary);

    if (!f.good()) {
        spdlog::warn("Failed to open file '{}' for loading", path);
        return;
    }

    _ingame->loadState(f);
}

void Engine::saveState(const std::string &filename)
{
    const auto path = fmt::format("saves/{}", filename);

    spdlog::info("Saving to {}", path);

    std::ofstream f(path, std::ios::out | std::ios::binary | std::ios::trunc);

    if (!f.good()) {
        spdlog::warn("Failed to open file '{}' for saving", path);
        return;
    }

    _ingame->saveState(f);
}

void Engine::openSaveManager(bool toLoad)
{
    _saveManager->setMode(toLoad);
    SceneMan::instance().setScene("save");
}

}    // namespace bty
