#include "game/shop-gen.hpp"

#include "data/bounty.hpp"
#include "data/shop.hpp"

int gen_shop_unit(int shop_index, int continent)
{
    return kShopUnits[shop_index + continent * 6];
}

int gen_shop_count(int unit)
{
    int max = kMaxShopCounts[unit];
    int a = bty::random(kMaxShopCounts[unit] >> 4, max / 2);
    int b = bty::random(kMaxShopCounts[unit] >> 5, max / 2);
    int c = bty::random(4);
    return c + a + b + max;
}

int get_shop_tile(int unit)
{
    return kShopTileForUnit[unit];
}
