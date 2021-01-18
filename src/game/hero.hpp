#ifndef BTY_GAME_HERO_HPP_
#define BTY_GAME_HERO_HPP_

#include "game/entity.hpp"

namespace bty {
struct Texture;
}    // namespace bty

enum class Mount {
    Fly,
    Boat,
    Walk,
};

class Hero : public Entity {
public:
    void load();
    bool canMove(int id, int x, int y, int c) override;
    void setMount(Mount mount);
    Mount getMount() const;
    void setMoving(bool val);

    float getSpeedMul() const;
    void setSpeedMul(float n);

private:
    void updateTexture();

private:
    bool _moving {false};
    Mount _mount {Mount::Walk};
    const bty::Texture *_texWalkMoving;
    const bty::Texture *_texWalkStationary;
    const bty::Texture *_texBoatMoving;
    const bty::Texture *_texBoatStationary;
    const bty::Texture *_texFlying;
    float _speedMul {1};
};

#endif    // BTY_GAME_HERO_HPP_
