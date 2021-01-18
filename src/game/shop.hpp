#ifndef BTY_GAME_SHOP_HPP_
#define BTY_GAME_SHOP_HPP_

#include "data/bounty.hpp"
#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "game/recruit-input.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Engine;
}    // namespace bty

struct Tile;
struct ShopInfo;

class Shop : public Component {
public:
    Shop(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    void update(float dt) override;
    bool handleEvent(Event event) override;

    bool handleKeyDown(Key key);
    bool handleKeyUp(Key key);

    void setShop(ShopInfo &info);

private:
    void confirm();

private:
    bty::Engine &_engine;
    ShopInfo *_info {nullptr};
    bty::Sprite _spBg;
    bty::Sprite _spUnit;
    bty::TextBox _box;
    const bty::Texture *_texUnits[25];
    const bty::Texture *_texDwellings[4];
    RecruitInput _recruitInput;
};

#endif    // BTY_GAME_SHOP_HPP_
