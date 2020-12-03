#ifndef BTY_GAME_TOWN_HPP_
#define BTY_GAME_TOWN_HPP_

#include "gfx/sprite.hpp"
#include "gfx/dialog.hpp"
#include "bounty.hpp"

struct SharedState;
struct Tile;
class Hud;

namespace bty {
class Gfx;
struct Texture;
}

struct CastleOccupation {
    int index;
    int occupier;
    std::array<int, 5> army;
    std::array<int, 5> army_counts;
};

class Town {
public:
    void load(bty::Assets &assets, bty::BoxColor color, SharedState &state);
    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void view(const Tile &tile, int continent, int unit_id, const CastleOccupation &info);
    void update(float dt);
    int key(int key);

private:
    SharedState *state_;
    Hud *hud_;
    bty::Sprite bg_;
    bty::Sprite unit_;
    bty::Dialog dialog_;
    const bty::Texture *unit_textures_[25];
    bty::TextBox gather_information_;
    bool show_gather_information_{false};
};

#endif // BTY_GAME_TOWN_HPP_
