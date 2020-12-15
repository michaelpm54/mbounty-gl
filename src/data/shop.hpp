#ifndef BTY_DATA_SHOP_HPP_
#define BTY_DATA_SHOP_HPP_

#include "data/tiles.hpp"

inline constexpr int kShopUnits[24] = {
    0x00,
    0x01,
    0x07,
    0x04,
    0x03,
    0x06,
    0x0c,
    0x05,
    0x0b,
    0x09,
    0x0f,
    0x09,
    0x0d,
    0x10,
    0x11,
    0x13,
    0x00,
    0x00,
    0x16,
    0x15,
    0x14,
    0x18,
    0x17,
    0x00,
};

inline constexpr int kMaxShopCounts[25] = {
    250,
    200,
    0,
    150,
    150,
    100,
    250,
    200,
    0,
    100,
    0,
    150,
    100,
    25,
    0,
    200,
    100,
    25,
    0,
    25,
    25,
    50,
    50,
    25,
    25,
};

inline constexpr int kShopTileForUnit[] = {
    Tile_ShopWagon,      // peasants
    Tile_ShopTree,       // militia
    Tile_ShopTree,       // sprites
    Tile_ShopWagon,      // wolves
    Tile_ShopDungeon,    // skeletons
    Tile_ShopDungeon,    // zombies
    Tile_ShopTree,       // gnomes
    Tile_ShopCave,       // orcs
    Tile_ShopTree,       // archers
    Tile_ShopTree,       // elves
    Tile_ShopWagon,      // pikemen
    Tile_ShopWagon,      // nomads
    Tile_ShopCave,       // dwarves
    Tile_ShopDungeon,    // ghosts
    Tile_ShopTree,       // knights
    Tile_ShopCave,       // ogres
    Tile_ShopWagon,      // barbarians
    Tile_ShopCave,       // trolls
    Tile_ShopTree,       // cavalries
    Tile_ShopTree,       // druids
    Tile_ShopTree,       // archmages
    Tile_ShopDungeon,    // vampires
    Tile_ShopWagon,      // giants
    Tile_ShopDungeon,    // demons
    Tile_ShopCave,       // dragons
};

#endif    // BTY_DATA_SHOP_HPP_
