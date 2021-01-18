#include "engine/dialog.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "gfx/text.hpp"
#include "gfx/texture.hpp"

namespace bty {

void Dialog::next()
{
    if (_options.empty()) {
        return;
    }

    int found = -1;

    for (auto i = 0u; i < _options.size(); i++) {
        int selection = (_selection + i + 1) % std::max(static_cast<int>(_options.size()), 1);
        if (_options[selection].enabled() && _options[selection].visible()) {
            found = selection;
            break;
        }
    }

    if (found != -1) {
        _arrowVisible = true;
        setSelection(found);
    }
    else {
        _arrowVisible = false;
        spdlog::warn("Dialog::next: No available option found.");
    }
}

void Dialog::prev()
{
    if (_options.empty()) {
        return;
    }

    int found = -1;

    for (int i = 0; i < static_cast<int>(_options.size()); i++) {
        int selection = (_selection - i - 1 + _options.size()) % std::max(static_cast<int>(_options.size()), 1);
        if (_options[selection].enabled() && _options[selection].visible()) {
            found = selection;
            break;
        }
    }

    if (found != -1) {
        _arrowVisible = true;
        setSelection(found);
    }
    else {
        _arrowVisible = false;
        spdlog::warn("Dialog::prev: No available option found.");
    }
}

void Dialog::setCellPosition(int x, int y)
{
    TextBox::setCellPosition(x, y);

    for (auto i = 0u; i < _options.size(); i++) {
        _options[i].setPosition({_optCellPositions[i].x * 8 + 8 * x, _optCellPositions[i].y * 8 + 8 * y});
    }

    updateArrow();
}

/* clang-format off */
void Dialog::create(
	int x, int y,
	int w, int h,
	bool setCommonBindings
)
/* clang-format on */
{
    TextBox::create(x, y, w, h);
    TextBox::setCellSize(w, h);
    TextBox::setColor(bty::BoxColor::Intro);

    _options.clear();
    _optCellPositions.clear();
    _spArrow.setTexture(Textures::instance().get("arrow.png", {2, 2}));
    _selection = -1;

    setCellPosition(x, y);

    if (setCommonBindings) {
        bind(Key::Up, [this](int) {
            prev();
        });
        bind(Key::Down, [this](int) {
            next();
        });
    }
}

Option *Dialog::addOption(int x, int y, std::string str)
{
    Option opt;
    opt.create(_cellX + x, _cellY + y, str);

    _options.push_back(std::move(opt));
    _optCellPositions.push_back({x, y});

    setSelection(0);

    return &_options.back();
}

void Dialog::setSelection(int index)
{
    if (_options.empty()) {
        return;
    }

    _selection = index;

    updateArrow();
}

void Dialog::render()
{
    TextBox::render();

    if (_options.empty())
        return;

    if (_arrowVisible)
        GFX::instance().drawSprite(_spArrow);

    for (auto &opt : _options) {
        if (opt.visible()) {
            GFX::instance().drawText(opt);
        }
    }
}

void Dialog::updateArrow()
{
    if (_options.empty())
        return;

    if (!_spArrow.getTexture())
        return;

    if (!_options[_selection].enabled() || !_options[_selection].visible()) {
        _arrowVisible = false;
        return;
    }
    else {
        _arrowVisible = true;
    }

    _spArrow.setPosition(_options[_selection].getPosition() - glm::vec2(_spArrow.getTexture()->frameW, 0.0f));
}

void Dialog::update(float dt)
{
    _spArrow.update(dt);
}

int Dialog::getSelection() const
{
    return _selection;
}

void Dialog::clearOptions()
{
    _options.clear();
    _optCellPositions.clear();
    _selection = -1;
}

std::deque<Option> &Dialog::get_options()
{
    return _options;
}

void Option::enable()
{
    _enabled = true;
}

void Option::disable()
{
    _enabled = false;
}

bool Option::enabled() const
{
    return _enabled;
}

void Option::show()
{
    _visible = true;
}

void Option::hide()
{
    _visible = false;
}

bool Option::visible() const
{
    return _visible;
}

void Option::setEnabled(bool val)
{
    _enabled = val;
}

void Option::setVisible(bool val)
{
    _visible = val;
}

void Dialog::bind(Key key, std::function<void(int)> callback)
{
    _keyBindings[key] = callback;
}

bool Dialog::handleEvent(Event event)
{
    switch (event.id) {
        case EventId::KeyDown:
            if (_keyBindings.contains(event.key)) {
                _keyBindings[event.key](_selection);
                return true;
            }
            if (_keyCallback) {
                return _keyCallback(event.key);
            }
        default:
            break;
    }
    return false;
}

void Dialog::disableInput()
{
    _inputEnabled = false;
}

void Dialog::enableInput()
{
    _inputEnabled = true;
}

bool Dialog::inputEnabled() const
{
    return _inputEnabled;
}

void Dialog::onKey(std::function<bool(Key)> callback)
{
    _keyCallback = callback;
}

}    // namespace bty
