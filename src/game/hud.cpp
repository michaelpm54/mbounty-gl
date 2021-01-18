#include "game/hud.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "data/color.hpp"
#include "data/hero.hpp"
#include "data/puzzle.hpp"
#include "engine/texture-cache.hpp"
#include "gfx/font.hpp"
#include "gfx/gfx.hpp"

Hud::Hud()
{
    auto &textures {Textures::instance()};

    _texBlankFrame = textures.get("frame/game-empty.png");
    _texHudFrame = textures.get("frame/game-hud.png");

    _spFrame.setTexture(_texHudFrame);

    _topFillRect.setSize(304, 9);
    _topFillRect.setPosition({8, 7});

    _btName.create(1, 1, "");
    _btDays.create(26, 1, "");
    _btError.create(1, 1, "");
    _btTimestop.create(26, 1, "");

    _texContracts.resize(18);
    for (int i = 0, max = static_cast<int>(_texContracts.size() - 1); i < max; i++) {
        _texContracts[i] = textures.get(fmt::format("villains/{}.png", i), {4, 1});
    }
    _texContracts.back() = textures.get("villains/empty.png");
    _spContract.setTexture(_texContracts[0]);
    _spContract.setPosition({262, 24});

    _texSiegeNo = textures.get("hud/siege-no.png");
    _texSiegeYes = textures.get("hud/siege-yes.png", {4, 1});
    _spSiege.setPosition({262, 64});

    _texMagicNo = textures.get("hud/magic-no.png");
    _texMagicYes = textures.get("hud/magic-yes.png", {4, 1});
    _spMagic.setPosition({262, 104});

    _spPuzzle.setTexture(textures.get("hud/puzzle-bg.png"));
    _spPuzzle.setPosition({262, 144});

    _spMoney.setTexture(textures.get("hud/gold-bg.png"));
    _spMoney.setPosition({262, 184});

    const auto *texPiece = textures.get("hud/puzzle-piece.png");

    int p = 0;

    float x = 264;
    float y = 145;
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 5; i++) {
            auto &piece = _spPieces[p++];
            piece.setPosition(x + i * 8, y + j * 6);
            piece.setTexture(texPiece);
        }
    }

    const auto *texGold = textures.get("hud/gold-2-gold.png");
    const auto *texSilver = textures.get("hud/gold-1-silver.png");
    const auto *texCopper = textures.get("hud/gold-0-copper.png");

    for (int i = 0; i < 10; i++) {
        _spGold[i].setTexture(texGold);
        _spGold[i].setPosition(264, 208.0f - i * 2);
    }
    for (int i = 0; i < 10; i++) {
        _spGold[10 + i].setTexture(texSilver);
        _spGold[10 + i].setPosition(280, 208.0f - i * 2);
    }
    for (int i = 0; i < 10; i++) {
        _spGold[20 + i].setTexture(texCopper);
        _spGold[20 + i].setPosition(296, 208.0f - i * 2);
    }
}

void Hud::render()
{
    auto &gfx {GFX::instance()};
    gfx.drawSprite(_spFrame);
    gfx.drawRect(_topFillRect);
    if (_isError) {
        gfx.drawText(_btError);
    }
    else {
        gfx.drawText(_btName);
        if (_isTimestop) {
            gfx.drawText(_btTimestop);
        }
        else {
            gfx.drawText(_btDays);
        }
    }
    if (_noSprites) {
        return;
    }
    gfx.drawSprite(_spContract);
    gfx.drawSprite(_spSiege);
    gfx.drawSprite(_spMagic);
    gfx.drawSprite(_spPuzzle);
    gfx.drawSprite(_spMoney);
    for (int i = 0; i < 25; i++) {
        if (!_hiddenPieces[i]) {
            gfx.drawSprite(_spPieces[i]);
        }
    }
    for (int i = 0; i < _numGoldSprites; i++) {
        gfx.drawSprite(_spGold[i]);
    }
    for (int i = 0; i < _numSilverSprites; i++) {
        gfx.drawSprite(_spGold[10 + i]);
    }
    for (int i = 0; i < _numCopperSprites; i++) {
        gfx.drawSprite(_spGold[20 + i]);
    }
}

void Hud::setContract(int contract)
{
    _spContract.setTexture(_texContracts[contract]);
}

void Hud::setDays(int days)
{
    _btDays.setString(fmt::format("Days Left:{}", days));
}

void Hud::setMagic(bool val)
{
    _spMagic.setTexture(val ? _texMagicYes : _texMagicNo);
}

void Hud::setSiege(bool val)
{
    _spSiege.setTexture(val ? _texSiegeYes : _texSiegeNo);
}

void Hud::setPuzzle(bool *villains, bool *artifacts)
{
    for (int i = 0; i < 17; i++) {
        _hiddenPieces[kPuzzleVillainPositions[i]] = villains[i];
    }
    for (int i = 0; i < 8; i++) {
        _hiddenPieces[kPuzzleArtifactPositions[i]] = artifacts[i];
    }
}

void Hud::setTitle(const std::string &str)
{
    _btName.setString(str);
    _btDays.setString("");
}

void Hud::setGold(int gold)
{
    int numGold = gold / 10000;
    gold -= (numGold * 10000);
    int numSilver = gold / 1000;
    gold -= (numSilver * 1000);
    int numCopper = gold / 100;
    _numGoldSprites = numGold > 10 ? 10 : numGold;
    _numSilverSprites = numSilver > 10 ? 10 : numSilver;
    _numCopperSprites = numCopper > 10 ? 10 : numCopper;
}

void Hud::update(float dt)
{
    _spContract.update(dt);
    _spSiege.update(dt);
    _spMagic.update(dt);
}

void Hud::setError(const std::string &msg, std::function<void()> then)
{
    _errorCallback = then;
    _btError.setString(msg);
    _isError = true;
}

void Hud::clearError()
{
    _isError = false;
    if (_errorCallback) {
        _errorCallback();
    }
}

bty::Sprite *Hud::getContractSprite()
{
    return &_spContract;
}

void Hud::setBlankFrame()
{
    _spFrame.setTexture(_texBlankFrame);
    _noSprites = true;
}

void Hud::setHudFrame()
{
    _spFrame.setTexture(_texHudFrame);
    _noSprites = false;
}

void Hud::setColor(bty::BoxColor color)
{
    _topFillRect.setColor(color);
}

void Hud::setTimestop(int amount)
{
    _btTimestop.setString(fmt::format("Timestop:{:>4}", amount));
    _isTimestop = true;
}

void Hud::clearTimestop()
{
    _isTimestop = false;
}

void Hud::setHero(int hero, int rank)
{
    _btName.setString(kHeroNames[hero][rank]);
}

bool Hud::getError() const
{
    return _isError;
}
