#ifndef BTY_GAME_TOWN_HPP_
#define BTY_GAME_TOWN_HPP_

#include "data/bounty.hpp"
#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Engine;
}    // namespace bty

struct TownGen;

class Town : public Component {
public:
    Town(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    void update(float dt) override;

    void setTown(TownGen *info);

private:
    void optGetContract();
    void optRentBoat();
    void optGatherInfo();
    void optBuySpell();
    void optBuySiege();
    void handleSelection(int opt);

private:
    bty::Engine &_engine;
    TownGen *_info {nullptr};
    bty::Sprite _spBg;
    bty::Sprite _spUnit;
    const bty::Texture *_texUnits[25];

    bty::Dialog _dlgMain;
    bty::Dialog _dlgOccupier;
    bty::Text *_btOccupierName;
    bty::Text *_btOccupierArmy;
    bty::Text *_btTownName;
    bty::Text *_btGP;
    bty::Option *_optGetContract;
    bty::Option *_optRentBoat;
    bty::Option *_optGatherInfo;
    bty::Option *_optBuySpell;
    bty::Option *_optBuySiege;
};

#endif    // BTY_GAME_TOWN_HPP_
