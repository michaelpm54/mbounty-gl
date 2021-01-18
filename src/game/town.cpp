#include "game/town.hpp"

#include <spdlog/spdlog.h>

#include "data/castles.hpp"
#include "data/spells.hpp"
#include "data/towns.hpp"
#include "data/villains.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/hud.hpp"
#include "game/map.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

Town::Town(bty::Engine &engine)
    : _engine(engine)
{
}

void Town::load()
{
    _spUnit.setPosition(64, 104);
    _spBg.setTexture(Textures::instance().get("bg/town.png"));
    _spBg.setPosition(8, 24);
    for (int i = 0; i < 25; i++) {
        _texUnits[i] = Textures::instance().get(fmt::format("units/{}.png", i), {2, 2});
    }

    _dlgMain.create(1, 18, 30, 9);
    _btTownName = _dlgMain.addString(1, 1);
    _btGP = _dlgMain.addString(22, 2);
    _optGetContract = _dlgMain.addOption(3, 3, "Get contract");
    _optRentBoat = _dlgMain.addOption(3, 4);
    _optGatherInfo = _dlgMain.addOption(3, 5, "Gather information");
    _optBuySpell = _dlgMain.addOption(3, 6);
    _optBuySiege = _dlgMain.addOption(3, 7);
    _dlgMain.bind(Key::Enter, std::bind(&Town::handleSelection, this, std::placeholders::_1));
    _dlgMain.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        SceneMan::instance().setScene("ingame");
    });

    _dlgOccupier.create(1, 18, 30, 9);
    _dlgOccupier.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
    });
    _btOccupierName = _dlgOccupier.addString(1, 1);
    _btOccupierArmy = _dlgOccupier.addString(1, 3);
}

void Town::render()
{
    GFX::instance().drawSprite(_spBg);
    GFX::instance().drawSprite(_spUnit);
    _dlgMain.render();
}

void Town::setTown(TownGen *info)
{
    _info = info;
}

void Town::enter()
{
    auto color {bty::getBoxColor(State::difficulty)};
    _dlgMain.setColor(color);
    _dlgOccupier.setColor(color);

    bool hasAnchor = State::artifacts_found[ArtiAnchorOfAdmirality];
    int boatCost = hasAnchor ? 100 : 500;

    _btTownName->setString(fmt::format("Town of {}", kTownInfo[_info->id].name));
    _btGP->setString(fmt::format("GP={}", bty::numberK(State::gold)));
    _optRentBoat->setString(State::boat_rented ? "Cancel boat rental" : fmt::format("Rent boat ({} week)", boatCost));
    _optBuySpell->setString(fmt::format("{} spell ({})", kSpellNames[_info->spell], kSpellCosts[_info->spell]));
    _optBuySiege->setString(State::siege ? "" : "Buy siege weapons (1000)");

    int num_captured = 0;
    for (int i = 0; i < 17; i++) {
        if (State::villains_captured[i]) {
            num_captured++;
        }
    }

    _optGetContract->setVisible(num_captured < 17);
    _optBuySiege->setVisible(!State::siege);

    _spUnit.setTexture(_texUnits[_info->unit]);

    _engine.getGUI().getHUD().setGold(State::gold);
    _engine.getGUI().pushDialog(_dlgMain);
}

void Town::update(float dt)
{
    _spUnit.update(dt);
}

void Town::optBuySpell()
{
    if (State::known_spells < State::max_spells) {
        if (State::gold >= kSpellCosts[_info->spell]) {
            State::gold -= kSpellCosts[_info->spell];
            State::spells[_info->spell]++;
            State::known_spells++;
            _engine.getGUI().getHUD().setGold(State::gold);
            int remaining = State::max_spells - State::known_spells;
            _engine.getGUI().getHUD().setTitle(fmt::format("     You can learn {} more spell{}.", remaining, remaining != 1 ? "s" : ""));
        }
        else {
            _engine.getGUI().getHUD().setError("    You do not have enough gold!");
        }
    }
    else {
        _engine.getGUI().getHUD().setError("   You can not learn anymore spells!", [this]() {
            _engine.getGUI().getHUD().setHero(State::hero, State::rank);
            _engine.getGUI().getHUD().setDays(State::days);
        });
    }
}

void Town::optBuySiege()
{
    if (State::gold < 3000) {
        _engine.getGUI().getHUD().setError("    You do not have enough gold!");
    }
    else {
        State::gold -= 3000;
        State::siege = true;
        _engine.getGUI().getHUD().setGold(State::gold);
        _btGP->setString(fmt::format("GP={}", bty::numberK(State::gold)));
        _engine.getGUI().getHUD().setSiege(true);
    }
    _dlgMain.prev();
    _optBuySiege->setVisible(!State::siege);
}

void Town::optGatherInfo()
{
    std::string army;
    for (int i = 0; i < 5; i++) {
        army += fmt::format(" {} {}\n", bty::unitDescriptor(State::castle_counts[_info->id][i]), kUnits[State::castle_armies[_info->id][i]].namePlural);
    }

    int villainId = State::castle_occupants[_info->id];
    std::string occupier;
    switch (villainId) {
        case -1:
            occupier = fmt::format("Castle {}\nis under your rule.", kCastleInfo[_info->id].name);
            break;
        case 0x7F:
            occupier = fmt::format("Castle {}\nis under no one's rule.", kCastleInfo[_info->id].name);
            break;
        default:
            occupier = fmt::format("Castle {} is under\n{}'s rule.", kCastleInfo[_info->id].name, kVillains[villainId][0]);
            break;
    }

    _btOccupierName->setString(occupier);
    _btOccupierArmy->setString(army);

    if (villainId != 0x7F && villainId != -1) {
        State::villains_found[villainId] = true;
    }

    _engine.getGUI().pushDialog(_dlgOccupier);
}

void Town::optGetContract()
{
    std::array<int, 5> availableContracts;
    std::fill(availableContracts.begin(), availableContracts.end(), -1);

    /* Gather the first five uncaught villains. */
    int found = 0;
    for (int i = 0; i < 17; i++) {
        if (!State::villains_captured[i]) {
            availableContracts[found++] = i;
            if (found == 5) {
                break;
            }
        }
    }

    /* Find the current contract amongst the first five uncaught villains. */
    int curIndexAmongstAvailable = -1;
    for (int i = 0; i < 5; i++) {
        if (availableContracts[i] == State::contract) {
            curIndexAmongstAvailable = i;
            break;
        }
    }

    /* If we don't have a contract, use the first available one, else use the next one. */
    if (curIndexAmongstAvailable == -1) {
        State::contract = availableContracts[0];
    }
    else {
        State::contract = availableContracts[(curIndexAmongstAvailable + 1) % 5];
    }

    _engine.getGUI().popDialog();
    _engine.getGUI().getHUD().setContract(State::contract);
    SceneMan::instance().setScene("viewcontract");
}

void Town::optRentBoat()
{
    if (!State::boat_rented) {
        State::boat_x = kTownBoatX[_info->id];
        State::boat_y = 63 - kTownBoatY[_info->id];
        State::boat_c = State::continent;
        _engine.setBoatPosition(State::boat_x * 48.0f + 8.0f, State::boat_y * 40.0f + 8.0f);
    }
    else {
        State::boat_x = -1;
        State::boat_y = -1;
        State::boat_c = -1;
    }

    State::boat_rented = !State::boat_rented;

    bool hasAnchor = State::artifacts_found[ArtiAnchorOfAdmirality];
    int boatCost = hasAnchor ? 100 : 500;
    if (State::boat_rented) {
        State::gold -= boatCost;
        _engine.getGUI().getHUD().setGold(State::gold);
    }
    _btGP->setString(fmt::format("GP={}", bty::numberK(State::gold)));
    _optRentBoat->setString(State::boat_rented ? "Cancel boat rental" : fmt::format("Rent boat ({} week)", boatCost));
}

void Town::handleSelection(int opt)
{
    switch (opt) {
        case 0:
            optGetContract();
            break;
        case 1:
            optRentBoat();
            break;
        case 2:
            optGatherInfo();
            break;
        case 3:
            optBuySpell();
            break;
        case 4:
            optBuySiege();
            break;
        default:
            break;
    }
}
