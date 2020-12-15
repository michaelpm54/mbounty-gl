#ifndef BTY_ENGINE_DIALOG_HPP_
#define BTY_ENGINE_DIALOG_HPP_

#include <array>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <unordered_set>
#include <vector>

#include "data/color.hpp"
#include "engine/textbox.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/texture.hpp"

namespace bty {

class Gfx;
class Text;
struct Texture;

class Option : public Text {
public:
    void enable();
    void disable();
    bool enabled() const;

    void show();
    void hide();
    bool visible() const;

    void set_enabled(bool);
    void set_visible(bool);

private:
    bool enabled_ {true};
    bool visible_ {true};
};

class Dialog : public TextBox {
public:
    /* clang-format off */
	void create(
		int x, int y,
		int w, int h,
		BoxColor color,
		Assets &assets
	);
    /* clang-format on */
    void set_position(int x, int y);
    Option *add_option(int x, int y, const std::string &str);
    void set_option(int index, std::string const &str);
    void next();
    void prev();
    void set_selection(int index);
    void draw(Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    int get_selection() const;
    void clear_options();
    std::deque<Option> &get_options();

private:
    void update_arrow();

private:
    std::deque<Option> options_;
    Sprite arrow_;
    int selection_ {0};
    bool draw_arrow_ {true};
};

}    // namespace bty

#endif    // BTY_ENGINE_DIALOG_HPP_
