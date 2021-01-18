#ifndef BTY_ENGINE_TEXTBOX_HPP_
#define BTY_ENGINE_TEXTBOX_HPP_

#include <array>
#include <deque>
#include <glm/vec2.hpp>
#include <string>

#include "data/bounty.hpp"
#include "data/color.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {

class Font;
class Gfx;
class Text;

class TextBox {
public:
    /* clang-format off */
	void create(
		int x, int y,
		int w, int h
	);
    /* clang-format on */

    Text *addString(int x, int y, std::string str = "");
    void set_line(int i, std::string const &str);    // TODO: Don't need this; use addString
    void setColor(BoxColor color);
    void setCellSize(int w, int h);
    void setCellPosition(int x, int y);
    void clear();
    void render();

private:
    std::array<Sprite, 8> _spBox;
    Rect _rectBg;
    Rect _rectBgOutline;
    float _realWidth;
    float _realHeight;

protected:
    std::vector<glm::ivec2> _stringCellPositions;
    std::deque<Text> _strings;
    int _cellX;
    int _cellY;
    int _cellWidth {0};
    int _cellHeight {0};
};

}    // namespace bty

#endif    // BTY_ENGINE_TEXTBOX_HPP_
