#ifndef BTY_DATA_TILES_HPP_
#define BTY_DATA_TILES_HPP_

enum TileId {
    Tile_Grass = 0,
    Tile_GrassInFrontOfCastle = 1,
    Tile_CastleLT = 2,
    Tile_CastleLB = 3,
    Tile_CastleT = 4,
    Tile_CastleB = 5,
    Tile_CastleRT = 6,
    Tile_CastleRB = 7,
    Tile_BridgeVertical = 8,
    Tile_BridgeHorizontal = 9,
    Tile_Town = 10,
    Tile_Chest = 11,
    Tile_ShopWagon = 12,
    Tile_ShopTree = 13,
    Tile_ShopCave = 14,
    Tile_ShopDungeon = 15,
    Tile_Sign = 16,
    Tile_TeleportCave = 17,    // CUSTOM
    Tile_AfctScroll = 18,      // Articles of nobility, 0x12 = 18d
    Tile_AfctRing = 19,        // Ring of heroism, 0x13 = 19d
    Tile_WaterIRT = 20,
    Tile_WaterILT = 21,
    Tile_WaterILB = 22,
    Tile_WaterIRB = 23,
    Tile_WaterELT = 24,
    Tile_WaterELB = 25,
    Tile_WaterERT = 26,
    Tile_WaterERB = 27,
    Tile_WaterR = 28,
    Tile_WaterL = 29,
    Tile_WaterB = 30,
    Tile_WaterT = 31,
    Tile_Water = 32,
    Tile_TreeELT = 33,
    Tile_TreeELB = 34,
    Tile_TreeERT = 35,
    Tile_TreeERB = 36,
    Tile_TreeILT = 37,
    Tile_TreeILB = 38,
    Tile_TreeIRT = 39,
    Tile_TreeIRB = 40,
    Tile_TreeR = 41,
    Tile_TreeL = 42,
    Tile_TreeB = 43,
    Tile_TreeT = 44,
    Tile_Tree = 45,
    Tile_SandELT = 46,
    Tile_SandELB = 47,
    Tile_SandERT = 48,
    Tile_SandERB = 49,
    Tile_SandILT = 50,
    Tile_SandILB = 51,
    Tile_SandIRT = 52,
    Tile_SandIRB = 53,
    Tile_SandR = 54,
    Tile_SandL = 55,
    Tile_SandT = 56,
    Tile_SandB = 57,
    Tile_Sand = 58,
    Tile_RockELT = 59,
    Tile_RockELB = 60,
    Tile_RockERT = 61,
    Tile_RockERB = 62,
    Tile_RockILT = 63,
    Tile_RockILB = 64,
    Tile_RockIRT = 65,
    Tile_RockIRB = 66,
    Tile_RockR = 67,
    Tile_RockL = 68,
    Tile_RockB = 69,
    Tile_RockT = 70,
    Tile_Rock = 71,
    Tile_TreeConnector = 72,     // CUSTOM
    Tile_AfctShield = 73,        // Shield of protection, 0x49 = 73d
    Tile_AfctAnchor = 74,        // Anchor of admirality, 0x4A = 74d
    Tile_AfctCrown = 75,         // Crown of command, 0x4B = 75d
    Tile_AfctBook = 76,          // Book of necros, 0x4C = 76d
    Tile_AfctAmulet = 77,        // Amulet of augmentation, 0x4D = 77d
    Tile_AfctSword = 78,         // Sword of prowess, 0x4E = 78d
    Tile_WaterConnector = 79,    // CUSTOM
    Tile_SandConnector = 80,     // CUSTOM
    /*
	* 81-127 GAP
	*/
    Tile_MobBlocker = 128,
    Tile_CastleTileNoCollide0 = 129,
    Tile_CastleTileNoCollide1 = 130,
    Tile_CastleTileNoCollide2 = 131,
    Tile_CastleTileNoCollide3 = 132,
    Tile_GenCastleGate = 133,
    /*
	* 134-137 GAP
	*/
    Tile_GenTown = 138,
    Tile_GenRandom = 139,
    Tile_GenPeasantCave = 140,
    /*
	* 141 GAP
	*/
    Tile_GenWizardCave = 142,
    /*
	* 143 GAP
	*/
    Tile_GenSign = 144,
    Tile_GenMonster = 145,
    Tile_GenFriendlyMonster = 146,
};

namespace bty {

inline constexpr bool is_event_tile(int id)
{
    return id == Tile_AfctAmulet || id == Tile_AfctAnchor || id == Tile_AfctBook || id == Tile_AfctCrown || id == Tile_AfctRing || id == Tile_AfctScroll || id == Tile_AfctShield || id == Tile_AfctSword || id == Tile_Chest || id == Tile_ShopCave || id == Tile_ShopDungeon || id == Tile_ShopTree || id == Tile_ShopWagon || id == Tile_Sign || id == Tile_GenSign || id == Tile_Town;
}

}    // namespace bty

#endif    // BTY_DATA_TILES_HPP_
