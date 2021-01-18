#include "engine/textbox.hpp"

#include <spdlog/spdlog.h>

#include "engine/texture-cache.hpp"
#include "game/state.hpp"
#include "gfx/font.hpp"
#include "gfx/gfx.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "window/keys.hpp"

namespace bty {

/* clang-format off */
void TextBox::create(
	int x, int y,
	int w, int h
)
/* clang-format on */
{
    _strings.clear();
    _stringCellPositions.clear();

    auto &border {Textures::instance().getBorder()};

    for (int i = 0; i < 8; i++) {
        _spBox[i].setTexture(border[i]);
    }

    setColor(bty::BoxColor::Intro);

    setCellSize(w, h);
    setCellPosition(x, y);
}

void TextBox::setCellSize(int w, int h)
{
    _cellWidth = w;
    _cellHeight = h;

    /* Minus one here because each border takes 4 pixels. 2x4 = 8 = 1 cell. */
    _realWidth = static_cast<float>(w - 1) * 8;
    _realHeight = static_cast<float>(h - 1) * 8;

    // top bottom
    _spBox[1].setSize(_realWidth, 4);
    _spBox[5].setSize(_realWidth, 4);

    // left right
    _spBox[3].setSize(4, _realHeight);
    _spBox[7].setSize(4, _realHeight);

    _rectBg.setSize(_realWidth - 2, _realHeight - 2);
    _rectBgOutline.setSize(_realWidth, _realHeight);
}

void TextBox::setCellPosition(int x__, int y__)
{
    _cellX = x__;
    _cellY = y__;

    float x = static_cast<float>(_cellX) * 8;
    float y = static_cast<float>(_cellY) * 8;

    _spBox[0].setPosition({x, y});
    _spBox[1].setPosition({x + 4, y});
    _spBox[2].setPosition({x + 4 + _realWidth, y});
    _spBox[3].setPosition({x + 4 + _realWidth, y + 4});
    _spBox[4].setPosition({x + 4 + _realWidth, y + 4 + _realHeight});
    _spBox[5].setPosition({x + 4, y + 4 + _realHeight});
    _spBox[6].setPosition({x, y + 4 + _realHeight});
    _spBox[7].setPosition({x, y + 4});

    _rectBg.setPosition({x + 5, y + 5});
    _rectBgOutline.setPosition({x + 4, y + 4});

    for (int i = 0; i < _strings.size(); i++) {
        _strings[i].setPosition(_stringCellPositions[i].x * 8.0f + x, _stringCellPositions[i].y * 8.0f + y);
    }
}

Text *TextBox::addString(int x, int y, std::string str)
{
    Text text;
    text.create(_cellX + x, _cellY + y, str);
    _strings.push_back(std::move(text));
    _stringCellPositions.push_back({x, y});

    return &_strings.back();
}

void TextBox::set_line(int i, std::string const &str)
{
    if (i < 0) {
        spdlog::warn("TextBox::set_line: i < 0");
    }
    else if (i >= static_cast<int>(_strings.size())) {
        spdlog::warn("TextBox::set_line: didn't exist so adding '{}'", str);
        Text text;
        text.create(_cellX + 2, _cellY + i, str);
        _strings.push_back(std::move(text));
    }
    else {
        _strings[i].setString(str);
    }
}

void TextBox::setColor(BoxColor color)
{
    if (color == BoxColor::None) {
        color = getBoxColor(State::difficulty);
    }
    _rectBgOutline.setColor(bty::getColor(color, true));
    _rectBg.setColor(bty::getColor(color, false));
}

void TextBox::clear()
{
    _strings.clear();
    _stringCellPositions.clear();
}

void TextBox::render()
{
    for (int i = 0; i < 8; i++)
        GFX::instance().drawSprite(_spBox[i]);

    GFX::instance().drawRect(_rectBgOutline);
    GFX::instance().drawRect(_rectBg);

    for (auto i = 0u; i < _strings.size(); i++) {
        GFX::instance().drawText(_strings[i]);
    }
}

}    // namespace bty
