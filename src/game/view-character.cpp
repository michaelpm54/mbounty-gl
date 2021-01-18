#include "game/view-character.hpp"

#include <spdlog/spdlog.h>

#include "data/bounty.hpp"
#include "data/hero.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"

ViewCharacter::ViewCharacter(bty::Engine &engine)
    : _engine(engine)
{
}

void ViewCharacter::render()
{
    auto &gfx {GFX::instance()};
    gfx.drawSprite(_spFrame);
    gfx.drawSprite(_spPortrait);
    gfx.drawRect(_fillRect);
    for (int i = 0; i < 11; i++) {
        gfx.drawText(_btInfo[i]);
    }
    for (int i = 0; i < 4; i++) {
        if (_mapsFound[i]) {
            gfx.drawSprite(_spMaps[i]);
        }
    }
    for (int i = 0; i < 8; i++) {
        if (_artifactsFound[i]) {
            gfx.drawSprite(_spArtifacts[i]);
        }
    }
}

void ViewCharacter::load()
{
    _engine.getGUI().getHUD().setBlankFrame();

    auto &textures = Textures::instance();

    _spFrame.setTexture(textures.get("frame/character.png"));
    _spFrame.setPosition(0, 16);

    static const std::string kPortraitFilenames[4] = {
        "crimsaun",
        "palmer",
        "tynnestra",
        "moham",
    };

    for (int i = 0; i < 8; i++) {
        _texArtifacts[i] = textures.get(fmt::format("artifacts/36x32/{}.png", i));
        _spArtifacts[i].setPosition(14.0f + (i % 4) * 48, 136.0f + (i / 4) * 40);
        _spArtifacts[i].setTexture(_texArtifacts[i]);
    }

    for (int i = 0; i < 4; i++) {
        _texPortraits[i] = textures.get(fmt::format("char-page/{}.png", kPortraitFilenames[i]));
    }

    _spPortrait.setPosition(8, 24);

    _fillRect.setSize(208, 104);
    _fillRect.setPosition(104, 24);

    const auto &font = textures.getFont();
    for (int i = 0; i < 11; i++) {
        _btInfo[i].setFont(font);
        _btInfo[i].setPosition(112, 40.0f + i * 8);
    }

    _btInfo[0].setPosition(112, 32);

    float x = 206;
    float y = 136;
    for (int i = 0; i < 4; i++) {
        if (i == 2) {
            x -= 112;
            y += 40;
        }
        _spMaps[i].setTexture(textures.get(fmt::format("maps/{}.png", i)));
        _spMaps[i].setPosition(x + i * 56, y);
    }

    _fillRect.setColor(bty::getBoxColor(State::difficulty));

    _spPortrait.setTexture(_texPortraits[State::hero]);

    _mapsFound = State::sail_maps_found.data();
    _artifactsFound = State::artifacts_found.data();

    _btInfo[0].setString(kHeroNames[State::hero][State::rank]);
    _btInfo[1].setString(fmt::format("Leadership {:>13}", State::leadership));
    _btInfo[2].setString(fmt::format("Commission/Week {:>8}", State::commission));
    _btInfo[3].setString(fmt::format("Gold {:>19}", State::gold));
    _btInfo[4].setString(fmt::format("Spell Power {:>12}", State::spell_power));
    _btInfo[5].setString(fmt::format("Max # of Spells {:>8}", State::max_spells));
    _btInfo[6].setString(fmt::format("Villains caught {:>8}", 0));
    _btInfo[7].setString(fmt::format("Artifacts found {:>8}", 0));
    _btInfo[8].setString(fmt::format("Castles garrisoned {:>5}", 0));
    _btInfo[9].setString(fmt::format("Followers killed {:>7}", State::followers_killed));
    _btInfo[10].setString(fmt::format("Current score {:>10}", 0));
    _spPortrait.setTexture(_texPortraits[State::hero]);
}

bool ViewCharacter::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool ViewCharacter::handleKey(Key key)
{
    switch (key) {
        case Key::Backspace:
            [[fallthrough]];
        case Key::Enter:
            SceneMan::instance().back();
            break;
        default:
            return false;
    }
    return true;
}
