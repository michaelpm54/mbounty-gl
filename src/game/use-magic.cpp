#include "game/use-magic.hpp"

#include <spdlog/spdlog.h>

#include "data/spells.hpp"
#include "engine/engine.hpp"
#include "game/state.hpp"

static constexpr const char *const kUntrainedMessage = {
    " You haven't been\n"
    "trained in the art\n"
    "  of spellcasting\n"
    "  yet. Visit the\n"
    " Archmage Aurange\n"
    " in Continentia at\n"
    "  11,19 for this\n"
    "     ability.\n",
};

UseMagic::UseMagic(bty::Engine &engine)
    : _engine(engine)
{
}

void UseMagic::load()
{
    _dlg.create(6, 4, 20, 22);
    _dlg.addString(1, 1, "Adventuring Spells");
    _dlg.addString(3, 12, "Combat Spells");

    _dlg.bind(Key::Enter, [this](int opt) {
        if (!_engine.getGameOptions().debug && !State::magic) {
            _engine.getGUI().popDialog();
            _engine.getGUI().showMessage(State::combat ? 10 : 6, 10, 20, 10, kUntrainedMessage, []() {
                SceneMan::instance().back();
            });
            return;
        }

        if (!_callbacks.contains(opt)) {
            spdlog::warn("UseMagic: No callback for spell ID {}", opt);
        }
        else {
            _engine.getGUI().popDialog();
            SceneMan::instance().back();
            _callbacks[opt]();
            State::spells[opt]--;
        }
    });

    _dlg.bind(Key::Backspace, [this](int) {
        _engine.getGUI().popDialog();
        SceneMan::instance().back();
    });

    for (int i = 0; i < 7; i++) {
        _dlg.addOption(4, 14 + i);
    }

    for (int i = 7; i < 14; i++) {
        _dlg.addOption(4, 3 + i - 7);
    }
}

void UseMagic::bindSpell(int id, std::function<void()> callback)
{
    _callbacks[id] = callback;
}

void UseMagic::enter()
{
    _dlg.setSelection(State::combat ? 0 : 7);
    _dlg.setColor(bty::getBoxColor(State::difficulty));
    auto &opts = _dlg.get_options();
    for (int i = 0; i < 14; i++) {
        if (State::combat) {
            if (i >= 7) {
                opts[i].disable();
            }
        }
        else {
            if (i < 7) {
                opts[i].disable();
            }
        }
        opts[i].setString(fmt::format("{} {}", State::spells[i], kSpellNames[i]));
        if (State::spells[i] == 0) {
            opts[i].disable();
        }
    }

    _engine.getGUI().pushDialog(_dlg);

    bool no_spells = true;
    for (int i = 7; i < 14; i++) {
        if (State::spells[i] != 0) {
            no_spells = false;
            break;
        }
    }

    if (no_spells) {
        _engine.getGUI().getHUD().setError(fmt::format("You have no {} spell to cast!", State::combat ? "Combat" : "Adventuring"));
        auto &options = _dlg.get_options();
        for (int i = 0; i < 7; i++) {
            options[i].disable();
        }
    }

    _dlg.setCellPosition(State::combat ? 10 : 6, 4);
}

bool UseMagic::isOverlay() const
{
    return true;
}

bool UseMagic::handleEvent(Event event)
{
    if (event.id == EventId::KeyDown) {
        return handleKey(event.key);
    }
    return false;
}

bool UseMagic::handleKey(Key key)
{
    if (key == Key::Backspace) {
        return true;
    }
    return false;
}
