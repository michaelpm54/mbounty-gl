#ifndef BTY_GAME_MAP_HPP_
#define BTY_GAME_MAP_HPP_

#include <array>
#include <glm/mat4x4.hpp>
#include <vector>

#include "gfx/gl.hpp"

namespace bty {
class Assets;
struct Texture;
}    // namespace bty

struct Tile {
    int tx;
    int ty;
    int id;
};

class Map {
public:
    ~Map();
    void load(bty::Assets &assets);
    void draw(glm::mat4 &camera, int continent);
    void update(float dt);
    Tile get_tile(int tx, int ty, int continent) const;
    Tile get_tile(float x, float y, int continent) const;
    Tile get_tile(glm::vec2 pos, int continent) const;
    Tile get_tile(glm::ivec2 coord, int continent) const;
    unsigned char *get_data(int continent);
    void create_geometry();
    void reset();
    void set_tile(const Tile &tile, int continent, int id);

private:
    int num_vertices_ {0};
    GLuint vbos_[4] {GL_NONE};
    GLuint vaos_[4] {GL_NONE};
    GLuint program_ {GL_NONE};
    GLint camera_loc_ {-1};
    GLint texture_loc_ {-1};
    const bty::Texture *tilesets_[10] {nullptr};
    float tileset_anim_timer_ {0};
    int tileset_index_ {0};
    std::array<std::vector<unsigned char>, 4> tiles_;
    std::array<std::vector<unsigned char>, 4> read_only_tiles_;
};

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

#endif    // BTY_GAME_MAP_HPP_
