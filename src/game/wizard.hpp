#ifndef BTY_GAME_WIZARD_HPP_
#define BTY_GAME_WIZARD_HPP_

#include "data/bounty.hpp"
#include "engine/component.hpp"
#include "engine/dialog.hpp"
#include "gfx/sprite.hpp"

namespace bty {
class Engine;
}    // namespace bty

class Wizard : public Component {
public:
    Wizard(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    void update(float dt) override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    void handleDialogOption(int opt);

    bty::Engine &_engine;
    bty::Sprite _spBg;
    bty::Sprite _spUnit;
    bty::Dialog _dlgWizard;
    bty::Dialog _dlgScold;
};

#endif    // BTY_GAME_WIZARD_HPP_
