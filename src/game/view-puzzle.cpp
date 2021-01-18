#include "game/view-puzzle.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

#include "data/bounty.hpp"
#include "data/puzzle.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"

ViewPuzzle::ViewPuzzle(bty::Engine &engine)
    : _engine(engine)
{
}

void ViewPuzzle::load()
{
    auto &textures = Textures::instance();

    for (int i = 0; i < 17; i++) {
        _texPieces[kPuzzleVillainPositions[i]] = textures.get(fmt::format("villains/{}.png", i), {4, 1});
    }
    for (int i = 0; i < 8; i++) {
        _texPieces[kPuzzleArtifactPositions[i]] = textures.get(fmt::format("artifacts/44x32/{}.png", i));
    }
    int n = 0;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            _spPieces[n].setTexture(_texPieces[n]);
            _spPieces[n++].setPosition(18.0f + x * 44, 40.0f + y * 32);
        }
    }

    float x = 8;
    float y = 24;
    float width = 5 * 44;
    float height = 5 * 32;

    for (int i = 0; i < 8; i++) {
        _spBorder[i].setTexture(textures.get(fmt::format("border-puzzle/{}.png", i)));
    }

    // top bottom
    _spBorder[1].setSize(width, 16);
    _spBorder[5].setSize(width, 16);
    _spBorder[1].setRepeat(true);
    _spBorder[5].setRepeat(true);

    // left right
    _spBorder[3].setSize(10, height);
    _spBorder[7].setSize(10, height);
    _spBorder[3].setRepeat(true);
    _spBorder[7].setRepeat(true);

    _spBorder[0].setPosition({x, y});
    _spBorder[1].setPosition({x + 10, y});
    _spBorder[2].setPosition({x + 10 + width, y});
    _spBorder[3].setPosition({x + 10 + width, y + 16});
    _spBorder[4].setPosition({x + 10 + width, y + 16 + height});
    _spBorder[5].setPosition({x + 10, y + 16 + height});
    _spBorder[6].setPosition({x, y + 16 + height});
    _spBorder[7].setPosition({x, y + 16});
}

void ViewPuzzle::render()
{
    SceneMan::instance().getLastScene()->render();
    auto view {glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f)};
    GFX::instance().setView(view);
    for (int i = 0; i < 8; i++) {
        GFX::instance().drawSprite(_spBorder[i]);
    }
    for (int i = 0; i < 25; i++) {
        if (!_debug && !_hiddenPieces[i]) {
            GFX::instance().drawSprite(_spPieces[i]);
        }
    }
}

void ViewPuzzle::enter()
{
    for (int i = 0; i < 25; i++) {
        _piecesToHide[i] = -1;
    }
    int spriteIndex = 0;
    for (int i = 0; i < 17; i++) {
        if (State::villains_captured[i]) {
            _piecesToHide[spriteIndex++] = kPuzzleVillainPositions[i];
        }
    }
    for (int i = 0; i < 8; i++) {
        if (State::artifacts_found[i]) {
            _piecesToHide[spriteIndex++] = kPuzzleArtifactPositions[i];
        }
    }
    for (int i = 0; i < 25; i++) {
        _hiddenPieces[i] = false;
    }
    if (spriteIndex) {
        _nextPop = 0;
        _doneAnimation = false;
    }
    else {
        _nextPop = -1;
        _doneAnimation = true;
    }
}

void ViewPuzzle::update(float dt)
{
    if (!_doneAnimation) {
        _popTimer += dt;
        if (_popTimer >= 0.5f) {
            _popTimer = 0;
            _hiddenPieces[_piecesToHide[_nextPop++]] = true;
            if (_nextPop == 25 || _piecesToHide[_nextPop] == -1) {
                _doneAnimation = true;
            }
        }
    }

    for (int i = 0; i < 25; i++) {
        if (!_hiddenPieces[i]) {
            _spPieces[i].update(dt);
        }
    }
}

bool ViewPuzzle::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool ViewPuzzle::handleKey(Key key)
{
    switch (key) {
        case Key::Backspace:
            [[fallthrough]];
        case Key::Enter:
            SceneMan::instance().back();
            break;
        case Key::D:
            _debug = !_debug;
            break;
        default:
            return false;
    }
    return true;
}
