#include "game/shop.hpp"

#include <spdlog/spdlog.h>

#include "data/shop.hpp"
#include "data/tiles.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/hud.hpp"
#include "game/shop-info.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

enum Dwellings {
    Cave,
    Forest,
    Dungeon,
    Plains,
};

static constexpr const char *const kShopNames[] = {
    "Cave",
    "Forest",
    "Dungeon",
    "Plains",
};

Shop::Shop(bty::Engine &engine)
    : _engine(engine)
{
}

void Shop::load()
{
    _box.create(1, 18, 30, 9);
    _box.addString(1, 1);     // Shop name
    _box.addString(1, 2);     // underline
    _box.addString(1, 4);     // available
    _box.addString(1, 5);     // cost
    _box.addString(22, 5);    // gp
    _box.addString(1, 6);     // you may recuit up to
    _box.addString(1, 7, "Recruit how many?");
    _box.addString(25, 7);    // current

    _spUnit.setPosition(64, 104);
    _spBg.setPosition(8, 24);
    for (int i = 0; i < 25; i++) {
        _texUnits[i] = Textures::instance().get(fmt::format("units/{}.png", i), {2, 2});
    }

    static constexpr const char *const kShopImages[] = {
        "bg/cave.png",
        "bg/forest.png",
        "bg/dungeon.png",
        "bg/plains.png",
    };

    for (int i = 0; i < 4; i++) {
        _texDwellings[i] = Textures::instance().get(kShopImages[i]);
    }
}

void Shop::render()
{
    GFX::instance().drawSprite(_spBg);
    GFX::instance().drawSprite(_spUnit);
    _box.render();
}

void Shop::setShop(ShopInfo &info)
{
    assert(info.unit != -1);
    _info = &info;
}

void Shop::enter()
{
    _box.setColor(bty::getBoxColor(State::difficulty));

    _recruitInput.clear();

    _spUnit.setTexture(_texUnits[_info->unit]);

    static constexpr int kTileToTypeMapping[] = {
        Tile_ShopCave,
        Tile_ShopTree,
        Tile_ShopDungeon,
        Tile_ShopWagon,
    };

    int shopType = 0;
    for (int i = 0; i < 4; i++) {
        if (kShopTileForUnit[_info->unit] == kTileToTypeMapping[i]) {
            shopType = i;
            break;
        }
    }

    const auto &unit = kUnits[_info->unit];

    int alreadyHave = 0;
    for (int i = 0; i < 5; i++) {
        if (State::army[i] == _info->unit) {
            alreadyHave = State::counts[i];
            break;
        }
    }

    int recruitable = (State::leadership / unit.hp) - alreadyHave;
    recruitable = std::min(State::gold / unit.recruitCost, recruitable);

    const std::string shopName = kShopNames[shopType];
    int numSpaces = static_cast<int>(floor(14.0f - (shopName.size() / 2)));
    std::string spaces = std::string(numSpaces, ' ');

    _box.set_line(0, spaces + shopName);
    _box.set_line(1, spaces + std::string(shopName.size(), '_'));
    _box.set_line(2, fmt::format("{} {} are available.", _info->count, kUnits[_info->unit].namePlural));
    _box.set_line(3, fmt::format("Cost = {} each.", kUnits[_info->unit].recruitCost));
    _box.set_line(4, fmt::format("GP={}", bty::numberK(State::gold)));
    _box.set_line(5, fmt::format("You may recruit up to {}.", recruitable));
    _recruitInput.setMax(recruitable);

    _spBg.setTexture(_texDwellings[shopType]);
}

void Shop::update(float dt)
{
    _spUnit.update(dt);
    _recruitInput.update(dt);
    _box.set_line(7, fmt::format("{:>3}", _recruitInput.getCurrentAmount()));
}

bool Shop::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKeyDown(event.key);
    }
    else if (event.id == EventId::KeyUp) {
        return handleKeyUp(event.key);
    }
    return false;
}

bool Shop::handleKeyDown(Key key)
{
    switch (key) {
        case Key::Backspace:
            SceneMan::instance().back();
            break;
        case Key::Enter:
            confirm();
            break;
        case Key::Left:
            break;
        case Key::Right:
            break;
        case Key::Up:
            break;
        case Key::Down:
            break;
        default:
            return false;
    }
    _recruitInput.handleKeyDown(key);
    return true;
}

bool Shop::handleKeyUp(Key key)
{
    _recruitInput.handleKeyUp(key);
    return true;
}

void Shop::confirm()
{
    if (_info->unit == -1 || !_info) {
        return;
    }

    int recruitCost = kUnits[_info->unit].recruitCost;
    int current = _recruitInput.getCurrentAmount();

    if (recruitCost > State::gold) {
        _engine.getGUI().getHUD().setError("     You do not have enough gold!");
    }
    else if (current > 0) {
        int cost = current * recruitCost;
        State::gold -= cost;
        _engine.getGUI().getHUD().setGold(State::gold);
        _info->count -= current;

        /* Add the amount to the existing one. */
        int alreadyHave = 0;
        for (int i = 0; i < 5; i++) {
            if (State::army[i] == _info->unit) {
                State::counts[i] += current;
                alreadyHave = State::counts[i];
                break;
            }
        }

        /* Find the first -1 unit and set it to the new one. */
        if (!alreadyHave) {
            for (int i = 0; i < 5; i++) {
                if (State::army[i] == -1) {
                    State::army[i] = _info->unit;
                    State::counts[i] = current;
                    alreadyHave = State::counts[i];
                    break;
                }
            }
        }

        /* Update the shop _info-> */
        const auto &unit = kUnits[_info->unit];

        int recruitable = (State::leadership / unit.hp) - alreadyHave;
        recruitable = std::min(State::gold / unit.recruitCost, recruitable);

        _box.set_line(2, fmt::format("{} {} are available.", _info->count, kUnits[_info->unit].namePlural));
        _box.set_line(4, fmt::format("GP={}", bty::numberK(State::gold)));
        _box.set_line(5, fmt::format("You may recruit up to {}.", recruitable));
        _recruitInput.setMax(recruitable);
    }
}
