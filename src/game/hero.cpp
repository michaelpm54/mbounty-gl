#include "game/hero.hpp"

#include <spdlog/spdlog.h>

#include "data/tiles.hpp"
#include "engine/texture-cache.hpp"
#include "game/map.hpp"

void Hero::load()
{
    auto &textures {Textures::instance()};
    _texWalkMoving = textures.get("hero/walk-moving.png", {4, 1});
    _texWalkStationary = textures.get("hero/walk-stationary.png", {4, 1});
    _texBoatMoving = textures.get("hero/boat-moving.png", {4, 1});
    _texBoatStationary = textures.get("hero/boat-stationary.png", {2, 1});
    _texFlying = textures.get("hero/flying.png", {4, 1});
    setTexture(_texWalkStationary);
}

void Hero::updateTexture()
{
    switch (_mount) {
        case Mount::Walk:
            setTexture(_moving ? _texWalkMoving : _texWalkStationary);
            break;
        case Mount::Boat:
            setTexture(_moving ? _texBoatMoving : _texBoatStationary);
            break;
        case Mount::Fly:
            setTexture(_texFlying);
            break;
        default:
            break;
    }
}

void Hero::setMount(Mount mount)
{
    _mount = mount;
    updateTexture();
}

Mount Hero::getMount() const
{
    return _mount;
}

void Hero::setMoving(bool val)
{
    _moving = val;
    updateTexture();
}

bool Hero::canMove(int id, int x, int y, int c)
{
    switch (_mount) {
        case Mount::Fly:
            return true;
        case Mount::Boat:
            return id == Tile_BridgeHorizontal || id == Tile_BridgeVertical || id == Tile_WaterConnector || (id >= Tile_WaterIRT && id <= Tile_Water);
        case Mount::Walk:
            return id <= Tile_GrassInFrontOfCastle || id == Tile_MobBlocker || id == Tile_BridgeHorizontal || id == Tile_BridgeVertical || (id >= Tile_SandELT && id <= Tile_Sand);
        default:
            break;
    }
    return false;
}

float Hero::getSpeedMul() const
{
    return _speedMul;
}

void Hero::setSpeedMul(float n)
{
    _speedMul = n;
}
