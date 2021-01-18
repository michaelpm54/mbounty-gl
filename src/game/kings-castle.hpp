#ifndef BTY_GAME_KINGS_CASTLE_HPP_
#define BTY_GAME_KINGS_CASTLE_HPP_

#include "data/bounty.hpp"
#include "data/color.hpp"
#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "game/recruit-input.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Engine;
}    // namespace bty

class KingsCastle : public Component {
public:
    KingsCastle(bty::Engine &engine);
    void render() override;
    void load();
    void enter();
    bool handleEvent(Event event) override;
    void update(float dt) override;

private:
    void recruitOpt(int opt);
    void showAudience();
    bool handleKeyDown(Key key);
    bool handleKeyUp(Key key);
    void checkPromote(int);
    void resetAudience();
    void confirmRecruitAmount();
    void startRecruiting();
    void stopRecruiting();

private:
    bty::Engine &_engine;
    bty::Sprite _spBg;
    bty::Sprite _spUnit;
    bty::Dialog _dlgMain;
    bty::Dialog _dlgRecruit;
    bty::Dialog _dlgAudience;
    bty::Text *_btRecruitCanGet;
    bty::Text *_btRecruitWillGet;
    bty::Text *_btAudience;
    bty::Text *_btGP;
    bty::Text *_btSelectArmy;
    bty::Text *_btHowMany;
    const bty::Texture *_texUnits[5];
    bool _givingRecruitInput {false};
    RecruitInput _recruitInput;
};

#endif    // BTY_GAME_KINGS_CASTLE_HPP_
