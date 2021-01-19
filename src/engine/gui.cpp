#include "engine/gui.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtc/matrix_transform.hpp>

#include "game/state.hpp"
#include "gfx/gfx.hpp"

namespace bty {

GUI::GUI()
    : _view(glm::ortho(0.0f, 320.0f, 224.0f, 0.0f, -1.0f, 1.0f))
{
}

void GUI::pushDialog(Dialog &dialog)
{
    _dialogs.push_back(&dialog);
}

void GUI::popDialog()
{
    if (_dialogs.empty()) {
        spdlog::warn("GUI::popDialog called with no dialogs");
        return;
    }

    auto dialog = std::find_if(_madeDialogs.begin(), _madeDialogs.end(), [this](const std::shared_ptr<Dialog> &a) {
        return a.get() == _dialogs.back();
    });

    if (dialog != _madeDialogs.end()) {
        _madeDialogs.erase(dialog);
    }

    _dialogs.pop_back();
}

bool GUI::handleEvent(Event event)
{
    if (_hud.getError()) {
        if (event.id == EventId::KeyDown && event.key == Key::Enter) {
            _hud.clearError();
        }
        return true;
    }

    if (!_dialogs.empty()) {
        if (_dialogs.back()->inputEnabled()) {
            _dialogs.back()->handleEvent(event);
            return true;
        }
    }

    return false;
}

void GUI::showHUD()
{
    _hudVisible = true;
}

void GUI::hideHUD()
{
    _hudVisible = false;
}

void GUI::render()
{
    GFX::instance().setView(_view);

    if (_hudVisible) {
        _hud.render();
    }

    if (!_dialogs.empty()) {
        for (auto *dialog : _dialogs) {
            dialog->render();
        }
    }
}

Hud &GUI::getHUD()
{
    return _hud;
}

bool GUI::update(float dt)
{
    if (_hudVisible) {
        _hud.update(dt);
    }

    for (auto *dialog : _dialogs) {
        dialog->update(dt);
    }

    return !_dialogs.empty();
}

bool GUI::hasDialog() const
{
    return !_dialogs.empty();
}

void GUI::showMessage(int x, int y, int w, int h, const std::string &message)
{
    auto dialog = makeDialog(x, y, w, h);
    dialog->addString(1, 1, message);
    dialog->bind(Key::Enter, [this](int) {
        popDialog();
    });
}

std::shared_ptr<Dialog> GUI::makeDialog(int x, int y, int w, int h, bool backspacePops)
{
    auto dialog = std::make_shared<Dialog>();
    dialog->create(x, y, w, h);
    dialog->setColor(bty::getBoxColor(State::difficulty));
    if (backspacePops) {
        dialog->bind(Key::Backspace, [this](int) {
            popDialog();
        });
    }
    _madeDialogs.push_back(dialog);
    _dialogs.push_back(dialog.get());
    return dialog;
}

}    // namespace bty
