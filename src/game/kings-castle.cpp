#include "game/kings-castle.hpp"

#include <spdlog/spdlog.h>

#include "data/hero.hpp"
#include "engine/engine.hpp"
#include "engine/scene-manager.hpp"
#include "engine/texture-cache.hpp"
#include "game/hud.hpp"
#include "game/state.hpp"
#include "gfx/gfx.hpp"

static constexpr int kKingsCastleUnits[5] = {
    Militias,
    Archers,
    Pikemen,
    Cavalries,
    Knights,
};

inline constexpr const char *const kAudienceMain {
    "   Trumpets announce your\n"
    " arrival with regal fanfare.\n"
    "\n"
    "   King Maximus rises from\n"
    "   his throne to greet you\n"
    "       and proclaims:\n",
};

inline constexpr const char *const kAudiencePromote {
    "{},\n"
    "\n"
    "      Congratulations!\n"
    "    I now promote you to\n"
    "          {}.",
};

inline constexpr const char *const kAudienceNeedMore {
    "{},\n"
    "\n"
    "	    I can aid you better\n"
    "   after you've captured {}\n"
    "        more villains.",
};

KingsCastle::KingsCastle(bty::Engine &engine)
    : _engine(engine)
{
}

void KingsCastle::load()
{
    _dlgMain.create(1, 18, 30, 9);
    _dlgMain.addString(1, 1, "Castle of King Maximus");
    _dlgMain.addString(22, 2);    // Gold
    _dlgMain.addOption(3, 4, "Recruit soldiers");
    _dlgMain.addOption(3, 5, "Audience with the King");
    _dlgMain.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        SceneMan::instance().back();
    });
    _dlgMain.bind(Key::Enter, [this](int opt) {
        _engine.getGUI().popDialog();
        if (opt == 0) {
            _engine.getGUI().pushDialog(_dlgRecruit);
        }
        else if (opt == 1) {
            _engine.getGUI().pushDialog(_dlgAudience);
        }
    });

    _dlgRecruit.create(1, 18, 30, 9);
    _dlgRecruit.addString(1, 1, "Recruit Soldiers");
    _btGP = _dlgRecruit.addString(22, 1);
    _btSelectArmy = _dlgRecruit.addString(17, 4, "Select army\nto recruit.");
    _btHowMany = _dlgRecruit.addString(17, 3, "You may get\nup to\n\nRecruit how\nmany?");
    _btHowMany->hide();
    _dlgRecruit.bind(Key::Enter, std::bind(&KingsCastle::recruitOpt, this, std::placeholders::_1));
    _dlgRecruit.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        _engine.getGUI().pushDialog(_dlgMain);
    });
    _btRecruitCanGet = _dlgRecruit.addString(23, 4);
    _btRecruitWillGet = _dlgRecruit.addString(25, 7);

    _dlgAudience.create(1, 18, 30, 9);
    _btAudience = _dlgAudience.addString(1, 1);

    _spUnit.setPosition(64, 104);
    _spBg.setTexture(Textures::instance().get("bg/castle.png"));
    _spBg.setPosition(8, 24);

    for (int i = 0; i < 5; i++) {
        _texUnits[i] = Textures::instance().get(fmt::format("units/{}.png", kKingsCastleUnits[i]), {2, 2});
    }
}

void KingsCastle::render()
{
    GFX::instance().drawSprite(_spBg);
    GFX::instance().drawSprite(_spUnit);
}

void KingsCastle::enter()
{
    auto color {bty::getBoxColor(State::difficulty)};
    _dlgMain.setColor(color);
    _dlgRecruit.setColor(color);
    _dlgAudience.setColor(color);
    resetAudience();

    _recruitInput.clear();

    _btRecruitWillGet->setString("  0");

    _btGP->setString(fmt::format("GP={}", bty::numberK(State::gold)));
    _spUnit.setTexture(_texUnits[rand() % 5]);

    _dlgRecruit.clearOptions();

    if (State::rank == 0) {
        _dlgRecruit.addOption(3, 3, "Militia   50");
        _dlgRecruit.addOption(3, 4, "Archers  250");
        _dlgRecruit.addOption(3, 5, "Pikemen  300");
        auto *a = _dlgRecruit.addOption(3, 6, "Cavalry  n/a");
        auto *b = _dlgRecruit.addOption(3, 7, "Knights  n/a");
        a->disable();
        b->disable();
    }
    else if (State::rank == 1) {
        _dlgRecruit.addOption(3, 3, "Militia   50");
        _dlgRecruit.addOption(3, 4, "Archers  250");
        _dlgRecruit.addOption(3, 5, "Pikemen  300");
        _dlgRecruit.addOption(3, 6, "Cavalry  800");
        auto *a = _dlgRecruit.addOption(3, 7, "Knights  n/a");
        a->disable();
    }
    else if (State::rank == 2 || State::rank == 3) {
        _dlgRecruit.addOption(3, 3, "Militia   50");
        _dlgRecruit.addOption(3, 4, "Archers  250");
        _dlgRecruit.addOption(3, 5, "Pikemen  300");
        _dlgRecruit.addOption(3, 6, "Cavalry  800");
        _dlgRecruit.addOption(3, 7, "Knights  1000");
    }

    _engine.getGUI().pushDialog(_dlgMain);
}

void KingsCastle::update(float dt)
{
    _spUnit.update(dt);
    if (_recruitInput.update(dt)) {
        _btRecruitWillGet->setString(fmt::format("{:>3}", _recruitInput.getCurrentAmount()));
    }
}

bool KingsCastle::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKeyDown(event.key);
    }
    else if (event.id == EventId::KeyUp) {
        return handleKeyUp(event.key);
    }
    return false;
}

bool KingsCastle::handleKeyDown(Key key)
{
    if (_givingRecruitInput) {
        if (key == Key::Enter) {
            confirmRecruitAmount();
        }
        else if (key == Key::Backspace) {
            stopRecruiting();
        }
        else {
            _recruitInput.handleKeyDown(key);
        }
        return true;
    }

    return false;
}

bool KingsCastle::handleKeyUp(Key key)
{
    if (_givingRecruitInput) {
        _recruitInput.handleKeyUp(key);
        return true;
    }

    return false;
}

void KingsCastle::recruitOpt(int opt)
{
    startRecruiting();

    int current = _recruitInput.getCurrentAmount();

    int id = kKingsCastleUnits[opt];
    int potentialAmt = State::leadership / kUnits[id].hp;
    int existingAmt = 0;

    for (int i = 0; i < 5; i++) {
        if (State::army[i] == id) {
            existingAmt = State::counts[i];
            break;
        }
    }

    int maxAmt = potentialAmt > existingAmt ? potentialAmt - existingAmt : 0;
    maxAmt = std::min(maxAmt, State::gold / kUnits[id].recruitCost);
    _recruitInput.setMax(maxAmt);
    _btRecruitCanGet->setString(fmt::format("{}.", maxAmt));

    if (kUnits[id].recruitCost > State::gold) {
        _engine.getGUI().getHUD().setError("     You do not have enough gold!");
    }

    _btRecruitWillGet->setString(fmt::format("{:>3}", std::min(current, maxAmt)));
}

void KingsCastle::confirmRecruitAmount()
{
    int current = _recruitInput.getCurrentAmount();

    if (current > 0) {
        int id = kKingsCastleUnits[_dlgRecruit.getSelection()];

        int cost = current * kUnits[id].recruitCost;
        State::gold -= cost;
        _engine.getGUI().getHUD().setGold(State::gold);
        _btGP->setString(fmt::format("GP={}", bty::numberK(State::gold)));
        _givingRecruitInput = false;

        bool found = false;
        for (int i = 0; i < 5; i++) {
            if (State::army[i] == id) {
                State::counts[i] += current;
                found = true;
                break;
            }
        }

        if (!found) {
            for (int i = 0; i < 5; i++) {
                if (State::army[i] == -1) {
                    State::army[i] = id;
                    State::counts[i] = current;
                    break;
                }
            }
        }
    }

    stopRecruiting();
    _givingRecruitInput = false;
    _dlgRecruit.enableInput();
}

void KingsCastle::checkPromote(int)
{
    _dlgAudience.bind(Key::Enter, [this](int) {
        _engine.getGUI().popDialog();
        _engine.getGUI().pushDialog(_dlgMain);
        resetAudience();
    });

    int num_captured = 0;
    for (int i = 0; i < 17; i++) {
        if (State::villains_captured[i]) {
            num_captured++;
        }
    }

    int reqVillains = 0;
    if (State::rank < 3) {
        if (num_captured < kRankVillainsCaptured[State::hero][State::rank + 1]) {
            reqVillains = kRankVillainsCaptured[State::hero][State::rank + 1] - num_captured;
        }
    }

    if (reqVillains <= 0) {
        if (State::rank != 3) {
            _btAudience->setString(fmt::format(kAudiencePromote, kHeroNames[State::hero][State::rank], kHeroNames[State::hero][State::rank + 1]));
            State::rank++;
        }
        else {
            /* TODO */
            assert(0);
        }
    }
    else {
        _btAudience->setString(fmt::format(kAudienceNeedMore, kHeroNames[State::hero][State::rank], reqVillains));
    }
}

void KingsCastle::resetAudience()
{
    _dlgAudience.bind(Key::Enter, std::bind(&KingsCastle::checkPromote, this, std::placeholders::_1));
    _btAudience->setString(kAudienceMain);
}

void KingsCastle::startRecruiting()
{
    _givingRecruitInput = true;
    _btSelectArmy->hide();
    _btHowMany->show();
    _btRecruitCanGet->show();
    _btRecruitWillGet->show();
    _dlgRecruit.disableInput();
}

void KingsCastle::stopRecruiting()
{
    _givingRecruitInput = false;
    _btSelectArmy->show();
    _btHowMany->hide();
    _btRecruitCanGet->hide();
    _btRecruitWillGet->hide();
    _dlgRecruit.enableInput();
}
