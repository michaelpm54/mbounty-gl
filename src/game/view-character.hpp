#ifndef BTY_GAME_VIEW_CHARACTER_HPP_
#define BTY_GAME_VIEW_CHARACTER_HPP_

#include "engine/component.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Engine;
}    // namespace bty

class ViewCharacter : public Component {
public:
    ViewCharacter(bty::Engine &engine);
    void load() override;
    void render() override;
    bool handleEvent(Event event) override;
    bool handleKey(Key key) override;

private:
    bty::Engine &_engine;
    bty::Sprite _spFrame;
    bty::Sprite _spPortrait;
    bty::Rect _fillRect;
    bty::Text _btInfo[11];
    const bty::Texture *_texArtifacts[8];
    const bty::Texture *_texPortraits[4];
    bty::Sprite _spArtifacts[8];
    bty::Sprite _spMaps[4];
    const bool *_artifactsFound;
    const bool *_mapsFound;
};

#endif    // BTY_GAME_VIEW_CHARACTER_HPP_
