#include "game/garrison.hpp"

#include <spdlog/spdlog.h>

#include "data/castles.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/hud.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

static constexpr int kKingsCastleUnits[5] = {
    Militias,
    Archers,
    Pikemen,
    Cavalries,
    Knights,
};

Garrison::Garrison(bty::Engine &engine)
    : _engine(engine)
{
}

void Garrison::load()
{
    _dlgMain.create(1, 18, 30, 9, false);
    _dlgMain.addOption(21, 3, "Garrison");
    _dlgMain.addOption(21, 4, "Remove");
    _dlgMain.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        SceneMan::instance().back();
    });
    _dlgMain.bind(Key::Enter, [this](int opt) {
        if (opt == 0) {
            _engine.getGUI().pushDialog(_dlgGarrison);
        }
        else if (opt == 1) {
            _dlgRemove.next();
            _dlgRemove.prev();

            if (!_dlgRemove.get_options()[_dlgRemove.getSelection()].enabled()) {
                _engine.getGUI().getHUD().setError("    There are no armies to remove!", [this]() {
                    _engine.getGUI().popDialog();
                });
            }

            _engine.getGUI().pushDialog(_dlgRemove);
        }
    });
    _dlgMain.bind(Key::Up, [this](int) {
        _dlgMain.prev();
        int opt = _dlgMain.getSelection();
        if (opt == 0) {
            showArmyUnits();
        }
        else if (opt == 1) {
            showCastleUnits();
        }
    });
    _dlgMain.bind(Key::Down, [this](int) {
        _dlgMain.next();
        int opt = _dlgMain.getSelection();
        if (opt == 0) {
            showArmyUnits();
        }
        else if (opt == 1) {
            showCastleUnits();
        }
    });

    _dlgGarrison.create(1, 18, 30, 9);
    _dlgRemove.create(1, 18, 30, 9);
    for (int i = 0; i < 5; i++) {
        _optGarrisonUnits[i] = _dlgGarrison.addOption(3, 3 + i);
        _optRemoveUnits[i] = _dlgRemove.addOption(3, 3 + i);
    }
    _dlgGarrison.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
    });
    _dlgGarrison.addString(21, 4, " Select\narmy  to\ngarrison");
    _dlgGarrison.bind(Key::Enter, std::bind(&Garrison::tryGarrisonUnit, this, std::placeholders::_1));

    _dlgRemove.addString(21, 4, " Select\narmy  to\n remove");
    _dlgRemove.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
    });
    _dlgRemove.bind(Key::Enter, std::bind(&Garrison::tryRemoveUnit, this, std::placeholders::_1));

    _btCastleName.create(2, 19, "");
    _btGP.create(23, 19, "");

    for (int i = 0; i < 5; i++) {
        _btUnits[i].create(4, 21 + i, "");
    }

    _spBg.setTexture(Textures::instance().get("bg/castle.png"));
    _spBg.setPosition(8, 24);

    _spUnit.setPosition(64, 104);

    for (int i = 0; i < 5; i++) {
        _texUnits[i] = Textures::instance().get(fmt::format("units/{}.png", kKingsCastleUnits[i]), {2, 2});
    }
}

void Garrison::render()
{
    GFX::instance().drawSprite(_spBg);
    GFX::instance().drawSprite(_spUnit);
}

void Garrison::setCastle(int castleId)
{
    _castleId = castleId;
}

void Garrison::renderLate()
{
    GFX::instance().drawText(_btCastleName);
    GFX::instance().drawText(_btGP);
    for (int i = 0; i < 5; i++) {
        GFX::instance().drawText(_btUnits[i]);
    }
}

void Garrison::enter()
{
    auto color {bty::getBoxColor(State::difficulty)};
    _dlgMain.setColor(color);
    _dlgGarrison.setColor(color);
    _dlgRemove.setColor(color);
    _spUnit.setTexture(_texUnits[rand() % 5]);
    _btCastleName.setString(fmt::format("Castle {}", kCastleInfo[_castleId].name));
    _btGP.setString(fmt::format("GP={}", bty::numberK(State::gold)));
    showArmyUnits();
    _engine.getGUI().pushDialog(_dlgMain);
}

void Garrison::update(float dt)
{
    _spUnit.update(dt);
}

void Garrison::showArmyUnits()
{
    for (int i = 0; i < 5; i++) {
        if (State::army[i] == -1) {
            _btUnits[i].setString("empty");
            _optGarrisonUnits[i]->disable();
        }
        else {
            auto cost = bty::numberK(kUnits[State::army[i]].weeklyCost * State::counts[i]);
            auto name = kUnits[State::army[i]].namePlural;
            std::string spaces(15 - cost.size() - name.size(), ' ');
            _btUnits[i].setString(fmt::format("{}{}{}", name, spaces, cost));
            _optGarrisonUnits[i]->enable();
        }
    }
}

void Garrison::showCastleUnits()
{
    for (int i = 0; i < 5; i++) {
        if (State::garrison_armies[_castleId][i] == -1) {
            _btUnits[i].setString("empty");
            _optRemoveUnits[i]->disable();
        }
        else {
            auto count = std::to_string(State::garrison_counts[_castleId][i]);
            auto name = kUnits[State::garrison_armies[_castleId][i]].namePlural;
            std::string spaces(15 - count.size() - name.size(), ' ');
            _btUnits[i].setString(fmt::format("{}{}{}", name, spaces, count));
            _optRemoveUnits[i]->enable();
        }
    }
}

void Garrison::tryGarrisonUnit(int index)
{
    int armySize = 0;
    for (int i = 0; i < 5; i++) {
        if (State::army[i] != -1) {
            armySize++;
        }
    }

    if (armySize == 1) {
        _engine.getGUI().getHUD().setError(" You may not garrison your last army!");
        return;
    }

    bool did_garrison {false};
    for (int i = 0; i < 5; i++) {
        if (State::garrison_armies[_castleId][i] == -1) {
            did_garrison = true;
            State::garrison_armies[_castleId][i] = State::army[index];
            State::garrison_counts[_castleId][i] = State::counts[index];
            // TODO: Gold error?
            State::gold -= kUnits[State::army[index]].weeklyCost * State::counts[index];
            State::army[index] = -1;
            State::counts[index] = 0;
            State::castle_occupants[_castleId] = -1;
            break;
        }
    }
    if (!did_garrison) {
        // TODO: Slots error?
    }
    bty::sortArmy(State::army, State::counts);
    showArmyUnits();
}

void Garrison::tryRemoveUnit(int index)
{
    int armySize = 0;
    for (int i = 0; i < 5; i++) {
        if (State::garrison_armies[_castleId][i] != -1) {
            armySize++;
        }
    }

    if (armySize == 0) {
        _engine.getGUI().getHUD().setError("    There are no armies to remove!");
        return;
    }

    bool did_remove {false};
    for (int i = 0; i < 5; i++) {
        if (State::army[i] == -1) {
            did_remove = true;
            State::army[i] = State::garrison_armies[_castleId][index];
            State::counts[i] = State::garrison_counts[_castleId][index];
            State::garrison_armies[_castleId][index] = -1;
            State::garrison_counts[_castleId][index] = 0;
            break;
        }
        else if (State::army[i] == State::garrison_armies[_castleId][i]) {
            did_remove = true;
            State::counts[i] += State::garrison_counts[_castleId][index];
            State::garrison_armies[_castleId][index] = -1;
            State::garrison_counts[_castleId][index] = 0;
            break;
        }
    }
    if (!did_remove) {
        // TODO: Slot error?
        _engine.getGUI().getHUD().setError("Failed to remove!");
    }
    else {
        bty::sortArmy(State::army, State::counts);
    }
    showCastleUnits();
}
