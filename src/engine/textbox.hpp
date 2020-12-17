#ifndef BTY_ENGINE_TEXTBOX_HPP_
#define BTY_ENGINE_TEXTBOX_HPP_

#include <array>
#include <deque>
#include <glm/vec4.hpp>
#include <string>

#include "data/bounty.hpp"
#include "data/color.hpp"
#include "engine/assets.hpp"
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
		int w, int h,
		BoxColor color,
		Assets &assets
	);
    /* clang-format on */

    void draw(Gfx &gfx, glm::mat4 &camera);
    Text *add_line(int x, int y, std::string const &str);
    void set_line(int i, std::string const &str);
    void set_color(BoxColor color);
    void set_line_visible(int index, bool value);
    void set_size(int w, int h);
    void set_position(int x, int y);
    void clear();

private:
    std::array<Sprite, 8> box_;
    std::vector<bool> lines_visible_;
    Rect background_;
    Rect background_outline_;
    float width_;
    float height_;

protected:
    std::vector<glm::ivec2> cell_positions;
    std::deque<Text> lines_;
    const Font *font_;
    int x_;
    int y_;
};

}    // namespace bty

#endif    // BTY_ENGINE_TEXTBOX_HPP_
