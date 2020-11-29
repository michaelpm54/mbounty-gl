#ifndef BTY_GFX_DIALOG_HPP_
#define BTY_GFX_DIALOG_HPP_

#include <array>
#include <string>
#include <unordered_set>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include "gfx/text.hpp"
#include "gfx/textbox.hpp"
#include "gfx/texture.hpp"
#include "gfx/sprite.hpp"

namespace bty {

struct Gfx;
class Text;
struct Texture;

class Dialog : public TextBox {
public:
	/* clang-format off */
	void create(
		int x, int y,
		int w, int h,
		const glm::vec4 * const accents,
		const std::array<const Texture*, 8> &border_textures,
		const Font &font,
		const Texture *arrow
	);
	/* clang-format on */
	void set_position(int x, int y);
	void add_option(int x, int y, const std::string &str);
	void set_option(int index, std::string const &str);
	void next();
	void prev();
	void set_selection(int index);
	void disable_option(int index);
	void draw(Gfx &gfx, glm::mat4 &camera);
	void animate(float dt);
	int get_selection() const;

private:
	void update_arrow();

private:
	std::vector<Text> options_;
	std::unordered_set<int> disabled_options_;
	Sprite arrow_;
	int selection_{0};
};

}

#endif // BTY_GFX_DIALOG_HPP_
