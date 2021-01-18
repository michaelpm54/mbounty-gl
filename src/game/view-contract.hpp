#ifndef BTY_GAME_VIEW_CONTRACT_HPP_
#define BTY_GAME_VIEW_CONTRACT_HPP_

#include "engine/component.hpp"
#include "engine/textbox.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

class ViewContract : public Component {
public:
    ViewContract(bty::Engine &engine, bty::Sprite *contract_sprite);

    void render() override;
    void enter() override;
    void load() override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    bty::Sprite *_spVillain {nullptr};
    bty::TextBox _box;
    bty::Text _btNoContract;
    bty::Text _btInfo;
    bool _noContract {false};
};

#endif    // BTY_GAME_VIEW_CONTRACT_HPP_
