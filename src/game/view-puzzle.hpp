#ifndef BTY_GAME_VIEW_PUZZLE_HPP_
#define BTY_GAME_VIEW_PUZZLE_HPP_

#include "engine/component.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

class ViewPuzzle : public Component {
public:
    ViewPuzzle(bty::Engine &engine);

    void load() override;
    void enter() override;
    void render() override;
    void update(float dt) override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    const bty::Texture *_texPieces[25] {nullptr};
    bty::Sprite _spPieces[25];
    bty::Sprite _spBorder[8];
    bool _hiddenPieces[25];
    int _piecesToHide[25];
    float _popTimer {0};
    int _nextPop {-1};
    bool _doneAnimation {false};
    bool _debug {false};
};

#endif    // BTY_GAME_VIEW_PUZZLE_HPP_
