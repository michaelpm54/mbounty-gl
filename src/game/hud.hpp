#ifndef GAME_HUD_HPP_
#define GAME_HUD_HPP_

#include <functional>
#include <glm/mat4x4.hpp>
#include <vector>

#include "data/color.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Font;
struct Texture;
}    // namespace bty

class Hud {
public:
    Hud();

    void render();
    void update(float dt);
    bty::Sprite *getContractSprite();
    void setTimestop(int amount);
    void clearTimestop();

    void setError(const std::string &msg, std::function<void()> then = nullptr);
    void setTitle(const std::string &msg); /* Similar to setError except it doesn't take input. */
    void clearError();
    bool getError() const;

    void setBlankFrame();
    void setHudFrame();
    void setColor(bty::BoxColor color);

    void setHero(int hero, int rank);
    void setDays(int days);
    void setContract(int contract);
    void setMagic(bool val);
    void setSiege(bool val);
    void setPuzzle(bool *villains, bool *artifacts);
    void setGold(int gold);

private:
    const bty::Texture *_texBlankFrame;
    const bty::Texture *_texHudFrame;

    bty::Sprite _spFrame;
    bty::Rect _topFillRect;
    bty::Text _btName;
    bty::Text _btDays;
    bty::Text _btTimestop;
    bty::Sprite _spContract;
    bty::Sprite _spSiege;
    bty::Sprite _spMagic;
    bty::Sprite _spPuzzle;
    bty::Sprite _spMoney;
    std::vector<const bty::Texture *> _texContracts;

    const bty::Texture *_texSiegeNo {nullptr};
    const bty::Texture *_texSiegeYes {nullptr};

    const bty::Texture *_texMagicNo {nullptr};
    const bty::Texture *_texMagicYes {nullptr};

    bty::Sprite _spPieces[25];
    bool _hiddenPieces[25];

    bty::Sprite _spGold[30];
    int _numGoldSprites {0};
    int _numSilverSprites {0};
    int _numCopperSprites {0};

    bool _isTimestop {false};

    bty::Text _btError;
    std::function<void()> _errorCallback;
    bool _isError {false};

    bool _noSprites {false};
};

#endif    // GAME_HUD_HPP_
