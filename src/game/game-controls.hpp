#ifndef GAME_GAME_CONTROLS_HPP_
#define GAME_GAME_CONTROLS_HPP_

#include "engine/component.hpp"
#include "engine/dialog.hpp"

namespace bty {
class Engine;
}    // namespace bty

struct GameOptions;

class GameControls : public Component {
public:
    GameControls(bty::Engine &engine);
    void load() override;
    void enter() override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;
    void render() override;
    void update(float dt) override;

private:
    bty::Engine &_engine;
    bty::Dialog _dlg;
    bty::Option *_optMusic;
    bty::Option *_optSound;
    bty::Option *_optDelay;

    void updateOptions();
};

#endif    // GAME_GAME_CONTROLS_HPP_
