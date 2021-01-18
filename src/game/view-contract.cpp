#include "game/view-contract.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "data/castles.hpp"
#include "data/villains.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"

ViewContract::ViewContract(bty::Engine &engine, bty::Sprite *contract_sprite)
    : _engine(engine)
    , _spVillain(contract_sprite)
{
}

void ViewContract::load()
{
    _box.create(1, 3, 30, 24);
    _btNoContract.create(6, 14, "You have no contract.");
    _btInfo.create(2, 9, "");
}

void ViewContract::render()
{
    _box.render();
    if (_noContract) {
        GFX::instance().drawText(_btNoContract);
    }
    else if (_spVillain) {
        auto pos = _spVillain->getPosition();
        _spVillain->setPosition(104, 40);
        GFX::instance().drawSprite(*_spVillain);
        _spVillain->setPosition(pos);
        GFX::instance().drawText(_btInfo);
    }
}

void ViewContract::enter()
{
    _box.setColor(bty::getBoxColor(State::difficulty));

    if (State::contract == 17) {
        _noContract = true;
        return;
    }
    else {
        _noContract = false;
    }

    int castle = -1;

    if (State::contract < 17 && State::villains_found[State::contract]) {
        for (int i = 0; i < 26; i++) {
            if (State::castle_occupants[i] == State::contract) {
                castle = i;
                break;
            }
        }
    }

    std::string castleName = castle == -1 ? "Unknown" : kCastleInfo[castle].name;

    std::string reward = std::to_string(kVillainRewards[State::contract]);
    int n = static_cast<int>(reward.length()) - 3;
    while (n > 0) {
        reward.insert(n, ",");
        n -= 3;
    }

    _btInfo.setString(fmt::format(
        R"raw(
Name: {}
Alias: {}
Reward: {} gold
Last seen: {}
Castle: {}
Distinguishing Features: {}
Crimes: {}
            )raw",
        kVillains[State::contract][0],
        kVillains[State::contract][1],
        reward,
        kContinentNames[kVillainContinents[State::contract]],
        castleName,
        kVillains[State::contract][2],
        kVillains[State::contract][3]));
}

bool ViewContract::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool ViewContract::handleKey(Key key)
{
    switch (key) {
        case Key::Enter:
            [[fallthrough]];
        case Key::Backspace:
            SceneMan::instance().back();
            break;
        default:
            return false;
    }
    return true;
}
