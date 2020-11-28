#include "gfx/textbox.hpp"

#include "gfx/font.hpp"
#include "gfx/gfx.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

#include <spdlog/spdlog.h>

namespace bty {

/* clang-format off */
void TextBox::create(
	int x, int y,
	int w, int h,
	const glm::vec4 * const accents,
	const std::array<const Texture*, 8> &border_textures,
	const Font &font)
/* clang-format on */
{
	font_ = &font;
	lines_.clear();

	for (int i = 0; i < 8; i++) {
		box_[i].set_texture(border_textures[i]);
	}

	set_colors(accents);

	set_size(w, h);
	set_position(x, y);
}

void TextBox::set_size(int w, int h)
{
	/* Minus one here because each border takes 4 pixels. 2x4 = 8 = 1 cell. */
	width_ = static_cast<float>(w-1) * 8;
	height_ = static_cast<float>(h-1) * 8;

	// top bottom
	box_[1].set_size(width_, 4);
	box_[5].set_size(width_, 4);

	// left right
	box_[3].set_size(4, height_);
	box_[7].set_size(4, height_);

	background_.set_size(width_ - 2, height_ - 2);
	background_outline_.set_size(width_, height_);
}

void TextBox::set_position(int x__, int y__)
{
	x_ = x__;
	y_ = y__;

	float x = static_cast<float>(x_) * 8;
	float y = static_cast<float>(y_) * 8;

	box_[0].set_position({x, y});
	box_[1].set_position({x+4, y});
	box_[2].set_position({x+4 + width_, y});
	box_[3].set_position({x+4 + width_, y+4});
	box_[4].set_position({x+4 + width_, y+4+height_});
	box_[5].set_position({x+4, y+4+height_});
	box_[6].set_position({x, y+4 + height_});
	box_[7].set_position({x, y+4});

	background_.set_position({x+5, y+5});
	background_outline_.set_position({x+4, y+4});
}

void TextBox::draw(Gfx &gfx, glm::mat4 &camera)
{
	for (int i = 0; i < 8; i++)
		gfx.draw_sprite(box_[i], camera);

	gfx.draw_rect(background_outline_, camera);
	gfx.draw_rect(background_, camera);

	for (auto &line : lines_) {
		gfx.draw_text(line, camera);
	}
}

void TextBox::add_line(int x, int y, std::string const &str)
{
	if (!font_) {
		spdlog::warn("TextBox::add_line: no font");
		return;
	}

	Text text;
	text.create(x_ + x, y_ + y, str, *font_);
	lines_.push_back(std::move(text));
}

void TextBox::set_line(int i, std::string const &str)
{
	assert(i > -1 && lines_.size() > i);
	lines_[i].set_string(str);
}

void TextBox::set_colors(const glm::vec4 * const accents)
{
	background_outline_.set_color(accents[0]);
	background_.set_color(accents[1]);
}

}
