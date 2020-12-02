#ifndef BTY_GAME_MAP_HPP_
#define BTY_GAME_MAP_HPP_

#include "gfx/gl.hpp"

#include <glm/mat4x4.hpp>

namespace bty {
class Assets;
class Texture;
}

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
    void erase_tile(const Tile &tile, int continent);

private:
    int num_vertices_{0};
    GLuint vbos_[4]{GL_NONE};
    GLuint vaos_[4]{GL_NONE};
    GLuint program_{GL_NONE};
    GLint camera_loc_{-1};
    GLint texture_loc_{-1};
    const bty::Texture *tilesets_[10]{nullptr};
    float tileset_anim_timer_{0};
    int tileset_index_{0};
    unsigned char *tiles_[4]{nullptr};
    unsigned char *read_only_tiles_[4]{nullptr};
};

enum TileId
{
    Grass = 0,
    GrassInFrontOfCastle,
    CastleTopLeft,
    CastleBottomLeft,
    CastleTopMid,
    CastleGate_Unused,
    CastleTopRight,
    CastleBottomRight,
    BridgeHorizontal,
    BridgeVertical,
    Town_Unused,
    Chest,
    ShopWagon,
    ShopTree,
    ShopCave,
    ShopDungeon,
    Sign_Unused,
    AfctScroll = 0x11,
    AfctRing,
    WaterInTopRight,
    WaterInTopLeft,
    WaterInBottomLeft,
    WaterInBottomRight,
    WaterOutTopLeft,
    WaterOutBottomLeft,
    WaterOutTopRight,
    WaterOutBottomRight,
    WaterRight,
    WaterLeft,
    WaterBottom,
    WaterTop,
    WaterOpen = 0x20,
    TreeOutBottomRight,
    TreeOutTopRight,
    TreeOutBottomLeft,
    TreeOutTopLeft,
    TreeInTopLeft,
    TreeInBottomLeft,
    TreeInTopRight,
    TreeInBottomRight,
    TreeRight,
    TreeLeft,
    TreeBottom,
    TreeTop,
    TreeOpen = 0x31,
    SandUnk0,
    SandUnk1,
    SandUnk2,
    SandUnk3,
    SandUnk4,
    SandUnk5,
    SandUnk6,
    SandUnk7,
    SandOpen,
    RockOutTopLeft,
    RockOutBottomLeft,
    RockOutTopRight,
    RockOutBottomRight,
    RockInBottomRight,
    RockInBottomLeft,
    RockInTopRight,
    RockInTopLeft,
    RockLeft,
    RockRight,
    RockTop,
    RockBottom,
    RockOpen,
    WaterConnector = 0x49,
    AfctAnchor = 0x4A,
    AfctCrown,
    AfctBook,
    AfctAmulet,
    AfctSword,
    AfctShield,
    MobBlocker = 0x80,
    CastleTileNoCollide0,
    CastleTileNoCollide1,
    CastleTileNoCollide2,
    CastleTileNoCollide3,
    CastleGate,
    Town = 0x8A,
    RandomEntity,
    PeasantCave,
    WizardCave = 0x8E,
    Sign = 0x90,
    MobTile,
    FriendlyMob,
};

#endif // BTY_GAME_MAP_HPP_
