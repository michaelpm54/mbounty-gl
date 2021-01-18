#ifndef BTY_GAME_GARRISON_HPP_
#define BTY_GAME_GARRISON_HPP_

#include "data/bounty.hpp"
#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "game/recruit-input.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Engine;
}    // namespace bty

class Garrison : public Component {
public:
    Garrison(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    void renderLate() override;
    void update(float dt) override;

    void setCastle(int castleId);

private:
    void showArmyUnits();
    void showCastleUnits();
    void tryGarrisonUnit(int index);
    void tryRemoveUnit(int index);

private:
    bty::Engine &_engine;
    bty::Sprite _spBg;
    bty::Sprite _spUnit;
    bty::Dialog _dlgMain;
    bty::Dialog _dlgGarrison;
    bty::Dialog _dlgRemove;
    const bty::Texture *_texUnits[5];

    bty::Text _btCastleName;
    bty::Text _btGP;
    bty::Text _btUnits[5];

    bty::Option *_optGarrisonUnits[5];
    bty::Option *_optRemoveUnits[5];

    int _castleId {-1};
};

#endif    // BTY_GAME_GARRISON_HPP_
