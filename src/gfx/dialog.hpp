#ifndef BTY_GFX_DIALOG_HPP_
#define BTY_GFX_DIALOG_HPP_

#include <array>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <unordered_set>
#include <vector>

#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "gfx/texture.hpp"

namespace bty {

class Gfx;
class Text;
struct Texture;

class Dialog : public TextBox {
public:
    /* clang-format off */
	void create(
		int x, int y,
		int w, int h,
		bty::BoxColor color,
		bty::Assets &assets
	);
    /* clang-format on */
    void set_position(int x, int y);
    Text *add_option(int x, int y, const std::string &str);
    void set_option(int index, std::string const &str);
    void next();
    void prev();
    void set_selection(int index);
    void draw(Gfx &gfx, glm::mat4 &camera);
    void animate(float dt);
    int get_selection() const;
    void clear_options();
    void set_option_disabled(int index, bool disabled);
    void set_option_visibility(int index, bool visible);

private:
    void update_arrow();

private:
    std::deque<Text> options_;
    std::vector<bool> disabled_options_;
    std::vector<bool> visible_options_;
    Sprite arrow_;
    int selection_ {0};
    bool draw_arrow_ {true};
};

}    // namespace bty

#endif    // BTY_GFX_DIALOG_HPP_
