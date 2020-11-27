#ifndef BTY_GFX_HPP_
#define BTY_GFX_HPP_

#include <array>
#include <string>

#include <glm/vec4.hpp>

#include "gfx/sprite.hpp"
#include "assets.hpp"

namespace bty {

struct Gfx;

class TextBox
{
public:
	void create(int x, int y, int w, int h, const glm::vec4 * const accents, const std::array<const Texture*, 8> &border_textures);
	void draw(Gfx &gfx, glm::mat4 &camera);

private:
	void set_size(int w, int h);
	void set_position(int x, int y);
	void add_line(int x, int y, std::string const &str);
	void set_line(int i, std::string const &str);
	void set_colors(const glm::vec4 * const accents);

private:
	std::array<Sprite, 8> box_;
	// std::vector<> lines_;
	Sprite background_;
	float width_;
	float height_;
	int x_;
	int y_;
};

}

#endif // BTY_GFX_HPP_
