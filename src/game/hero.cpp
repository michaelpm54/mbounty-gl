#include "game/hero.hpp"

#include <spdlog/spdlog.h>

#include "data/tiles.hpp"
#include "engine/assets.hpp"
#include "game/map.hpp"

Hero::Hero(int &boat_x, int &boat_y, int &boat_c)
    : boat_x(boat_x)
    , boat_y(boat_y)
    , boat_c(boat_c)
{
}

void Hero::load(bty::Assets &assets)
{
    tex_walk_moving_ = assets.get_texture("hero/walk-moving.png", {4, 1});
    tex_walk_stationary_ = assets.get_texture("hero/walk-stationary.png", {4, 1});
    tex_boat_moving_ = assets.get_texture("hero/boat-moving.png", {4, 1});
    tex_boat_stationary_ = assets.get_texture("hero/boat-stationary.png", {2, 1});
    tex_flying = assets.get_texture("hero/flying.png", {4, 1});
    set_texture(tex_walk_stationary_);
}

void Hero::update_texture()
{
    switch (mount_) {
        case Mount::Walk:
            set_texture(moving_ ? tex_walk_moving_ : tex_walk_stationary_);
            break;
        case Mount::Boat:
            set_texture(moving_ ? tex_boat_moving_ : tex_boat_stationary_);
            break;
        case Mount::Fly:
            set_texture(tex_flying);
            break;
        default:
            break;
    }
}

void Hero::set_mount(Mount mount)
{
    mount_ = mount;
    update_texture();
}

Mount Hero::get_mount() const
{
    return mount_;
}

void Hero::set_moving(bool val)
{
    moving_ = val;
    update_texture();
}

bool Hero::can_move(int id, int x, int y, int c)
{
    switch (mount_) {
        case Mount::Fly:
            return true;
        case Mount::Boat:
            return id <= Tile_GrassInFrontOfCastle || id == Tile_BridgeHorizontal || id == Tile_BridgeVertical || id == Tile_WaterConnector || (id >= Tile_WaterIRT && id <= Tile_Water);
        case Mount::Walk:
            return id <= Tile_GrassInFrontOfCastle || id == Tile_MobBlocker || id == Tile_BridgeHorizontal || id == Tile_BridgeVertical || (id >= Tile_SandELT && id <= Tile_Sand);
        default:
            break;
    }
    return false;
}

float Hero::get_speed_multiplier() const
{
    return speed_multiplier;
}

void Hero::set_speed_multiplier(float n)
{
    speed_multiplier = n;
}
