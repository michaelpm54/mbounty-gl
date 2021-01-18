#include "game/view-army.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"

ViewArmy::ViewArmy(bty::Engine &engine)
    : _engine(engine)
{
}

void ViewArmy::load()
{
    _engine.getGUI().getHUD().setBlankFrame();

    auto &textures {Textures::instance()};

    for (int i = 0; i < 25; i++) {
        _texUnits[i] = textures.get(fmt::format("units/{}.png", i), {2, 2});
    }

    _spFrame.setTexture(textures.get("frame/army.png"));
    _spFrame.setPosition(0, 16);

    const auto &font = textures.getFont();

    for (int i = 0; i < 5; i++) {
        _fillRects[i].setColor(bty::BoxColor::Intro);
        _fillRects[i].setSize(253, 32);
        _fillRects[i].setPosition(59, 24.0f + i * 40);
        _spUnits[i].setPosition(8, 24.0f + i * 40);

        for (int j = 0; j < 7; j++) {
            _btInfo[i][j].setFont(font);
        }

        float x = 64;
        float y = 32.0f + i * 40;
        _btInfo[i][0].setPosition(x, y);
        _btInfo[i][1].setPosition(x + 120, y);
        _btInfo[i][2].setPosition(x, y + 8);
        _btInfo[i][3].setPosition(x + 56, y + 8);
        _btInfo[i][4].setPosition(x + 120, y + 8);
        _btInfo[i][5].setPosition(x, y + 16);
        _btInfo[i][6].setPosition(x + 120, y + 16);
    }

    for (int i = 0; i < 5; i++) {
        _fillRects[i].setColor(bty::getBoxColor(State::difficulty));
    }

    _armySize = 0;

    for (int i = 0; i < 5; i++) {
        if (State::army[i] != -1) {
            _armySize++;
        }
    }

    for (int i = 0; i < _armySize; i++) {
        int unit_id = State::army[i];
        const auto &unit = kUnits[unit_id];
        if (unit_id != -1) {
            int hp = State::counts[i] * unit.hp;
            int minDmg = State::counts[i] * unit.meleeMinDmg;
            int maxDmg = State::counts[i] * unit.meleeMaxDmg;
            int weeklyCost = State::counts[i] * unit.weeklyCost;

            static const std::array<std::string, 4> kMoraleStrings = {{
                "Morale: Norm",
                "Morale: Low",
                "Morale: High",
                "Out of Control",
            }};

            assert(unit_id < 25);
            _spUnits[i].setTexture(_texUnits[unit_id]);
            _btInfo[i][0].setString(fmt::format("{} {}", State::counts[i], unit.namePlural));
            _btInfo[i][1].setString(fmt::format("HitPts: {}", hp));
            _btInfo[i][2].setString(fmt::format("SL: {}", unit.skillLevel));
            _btInfo[i][3].setString(fmt::format("MV: {}", unit.initialMoves));
            _btInfo[i][4].setString(fmt::format("Damage: {}-{}", minDmg, maxDmg));
            _btInfo[i][5].setString(fmt::format(kMoraleStrings[State::morales[i]]));
            _btInfo[i][6].setString(fmt::format("G-Cost: {}", weeklyCost));
        }
    }
}

void ViewArmy::render()
{
    GFX::instance().drawSprite(_spFrame);
    for (int i = 0; i < 5; i++) {
        GFX::instance().drawRect(_fillRects[i]);
    }
    for (int i = 0; i < _armySize; i++) {
        GFX::instance().drawSprite(_spUnits[i]);
        for (int j = 0; j < 7; j++) {
            GFX::instance().drawText(_btInfo[i][j]);
        }
    }
}

void ViewArmy::update(float dt)
{
    for (int i = 0; i < _armySize; i++) {
        _spUnits[i].update(dt);
    }
}

bool ViewArmy::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool ViewArmy::handleKey(Key key)
{
    switch (key) {
        case Key::Backspace:
            [[fallthrough]];
        case Key::Enter:
            SceneMan::instance().back();
            break;
        default:
            return false;
    }
    return true;
}
