#include "game/view-continent.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "data/bounty.hpp"
#include "data/tiles.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/ingame.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"

ViewContinent::ViewContinent(bty::Engine &engine)
    : _engine(engine)
    , _texMap({64, 64, GL_NONE, 1, 1, 64, 64})
    , _view(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
{
}

void ViewContinent::load()
{
    _box.create(6, 4, 20, 22);
    _btContinent = _box.addString(5, 1);
    _btCoordinates = _box.addString(1, 20);

    glCreateTextures(GL_TEXTURE_2D, 1, &_texMap.handle);
    glTextureStorage2D(_texMap.handle, 1, GL_RGBA8, 64, 64);
    glTextureParameterf(_texMap.handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameterf(_texMap.handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameterf(_texMap.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameterf(_texMap.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _spMap.setTexture(&_texMap);
    _spMap.setPosition(64, 56);
    _spMap.setSize(128, 128);
}

void ViewContinent::unload()
{
    glDeleteTextures(1, &_texMap.handle);
}

void ViewContinent::render()
{
    SceneMan::instance().getLastScene()->render();
    GFX::instance().setView(_view);
    _box.render();
    GFX::instance().drawSprite(_spMap);
}

void ViewContinent::update(float dt)
{
    static_cast<Ingame *>(SceneMan::instance().getLastScene())->updateAnimations(dt);

    _dotTimer += dt;
    _dotAlpha = glm::abs(glm::cos(_dotTimer * 4));

    uint8_t val = static_cast<uint8_t>(255 * _dotAlpha);

    /* Cyan */
    uint32_t pixel = 0xFF000000 | (val << 8) | val;

    glTextureSubImage2D(
        _texMap.handle,
        0,
        State::x,
        State::y,
        1,
        1,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        &pixel);
}

void ViewContinent::enter()
{
    _box.setColor(bty::getBoxColor(State::difficulty));

    this->_haveThisMap = State::continent_maps_found[State::continent];

    _btContinent->setString(kContinentNames[State::continent]);
    _btCoordinates->setString(fmt::format("X={:>2} Position Y={:>2}", State::x, 63 - State::y));

    genTexture();
}

bool ViewContinent::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool ViewContinent::handleKey(Key key)
{
    switch (key) {
        case Key::Backspace:
            SceneMan::instance().back();
            break;
        case Key::Enter:
            if (_haveThisMap) {
                _fogEnabled = !_fogEnabled;
                genTexture();
            }
            break;
        default:
            return false;
    }
    return true;
}

void ViewContinent::genTexture()
{
    /* ARGB */
    static constexpr uint32_t waterEdge = 0xFF2161C7;
    static constexpr uint32_t waterDeep = 0xFF002084;
    static constexpr uint32_t grass = 0xFF21A300;
    static constexpr uint32_t trees = 0xFF006100;
    static constexpr uint32_t rocks = 0xFF844100;
    static constexpr uint32_t black = 0xFF000000;
    static constexpr uint32_t yellow = 0xFFCCCC00;
    static constexpr uint32_t castle = 0xFFE8E4E8;

    const unsigned char *const map = _fogEnabled ? State::visited_tiles[State::continent].data() : State::tiles[State::continent];

    std::vector<unsigned char> pixels(64 * 64 * 4);
    unsigned char *p = pixels.data();

    for (int i = 0; i < 4096; i++) {
        int id = map[i];
        if (id == 0xFF) {
            std::memcpy(p + i * 4, &black, 4);
        }
        else if (id <= Tile_GrassInFrontOfCastle) {
            std::memcpy(p + i * 4, &grass, 4);
        }
        else if (id >= Tile_WaterIRT && id < Tile_Water) {
            std::memcpy(p + i * 4, &waterEdge, 4);
        }
        else if (id == Tile_Water) {
            std::memcpy(p + i * 4, &waterDeep, 4);
        }
        else if (id >= Tile_TreeERB && id <= Tile_Tree) {
            std::memcpy(p + i * 4, &trees, 4);
        }
        else if (id >= Tile_SandELT && id <= Tile_Sand) {
            std::memcpy(p + i * 4, &yellow, 4);
        }
        else if (id >= Tile_RockELT && id <= Tile_Rock) {
            std::memcpy(p + i * 4, &rocks, 4);
        }
        else if (id >= Tile_CastleLT && id <= Tile_CastleRB) {
            std::memcpy(p + i * 4, &castle, 4);
        }
        else {
            std::memcpy(p + i * 4, &trees, 4);
        }
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 64);

    glTextureSubImage2D(
        _texMap.handle,
        0,
        0,
        0,
        64,
        64,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        pixels.data());
}
