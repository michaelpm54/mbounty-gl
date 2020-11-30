#ifndef BTY_GFX_HPP_
#define BTY_GFX_HPP_

#include <array>
#include <string>
#include <vector>

#include <glm/vec4.hpp>

#include "bounty.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "assets.hpp"

namespace bty {

class Font;
struct Gfx;
class Text;

class TextBox
{
public:
	/* clang-format off */
	void create(
		int x, int y,
		int w, int h,
		bty::BoxColor color,
		const std::array<const Texture*, 8> &border_textures,
		const Font &font
	);
	/* clang-format on */

	void draw(Gfx &gfx, glm::mat4 &camera);
	void add_line(int x, int y, std::string const &str);
	void set_line(int i, std::string const &str);

protected:
	void set_size(int w, int h);
	void set_position(int x, int y);
	void set_colors(bty::BoxColor color);

private:
	std::array<Sprite, 8> box_;
	std::vector<Text> lines_;
	Rect background_;
	Rect background_outline_;
	float width_;
	float height_;

protected:
	const Font *font_;
	int x_;
	int y_;
};

}

#endif // BTY_GFX_HPP_
