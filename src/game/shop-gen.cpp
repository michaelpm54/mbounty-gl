#include "game/shop-gen.hpp"

#include "data/bounty.hpp"
#include "data/shop.hpp"

int genShopUnit(int shopIndex, int continent)
{
    return kShopUnits[shopIndex + continent * 6];
}

int genShopCount(int unit)
{
    int max = kMaxShopCounts[unit];
    int a = bty::random(kMaxShopCounts[unit] >> 4, max / 2);
    int b = bty::random(kMaxShopCounts[unit] >> 5, max / 2);
    int c = bty::random(4);
    return c + a + b + max;
}

int getShopTileType(int unit)
{
    return kShopTileForUnit[unit];
}
