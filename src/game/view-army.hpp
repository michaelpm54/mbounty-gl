#ifndef BTY_GAME_VIEW_ARMY_HPP_
#define BTY_GAME_VIEW_ARMY_HPP_

#include <array>

#include "engine/component.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

class ViewArmy : public Component {
public:
    ViewArmy(bty::Engine &engine);
    void load() override;
    void render() override;
    void update(float dt) override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    bty::Sprite _spFrame;
    std::array<const bty::Texture *, 25> _texUnits;
    std::array<bty::Rect, 5> _fillRects;
    std::array<bty::Sprite, 5> _spUnits;
    std::array<std::array<bty::Text, 7>, 5> _btInfo;
    int _armySize {0};
};

#endif    // BTY_GAME_VIEW_ARMY_HPP_
