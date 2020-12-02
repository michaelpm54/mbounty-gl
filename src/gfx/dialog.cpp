#include "gfx/dialog.hpp"

#include <cassert>

#include "gfx/gfx.hpp"
#include "gfx/text.hpp"
#include "gfx/texture.hpp"

namespace bty {

void Dialog::next()
{
	if (options_.empty())
		return;

	int count = 0;
	int sel = selection_;
	do {
		sel = (sel + 1) % options_.size();
		count++;
	} while (count < static_cast<int>(options_.size()) && disabled_options_.contains(sel));

	set_selection(sel);
}

void Dialog::prev()
{
	if (options_.empty())
		return;

	int count = 0;
	int sel = selection_;
	do {
		sel = sel = (sel - 1 + options_.size()) % options_.size();
		count++;
	} while (count < options_.size() && disabled_options_.contains(sel));

	set_selection(sel);
}

void Dialog::set_position(int x, int y)
{
	TextBox::set_position(x, y);

	for (int i = 0; i < options_.size(); i++)
	{
		options_[i].set_position({x_ + x, y_ + y});
	}
}

/* clang-format off */
void Dialog::create(
	int x, int y,
	int w, int h,
	bty::BoxColor color,
	bty::Assets &assets
)
/* clang-format on */
{
	TextBox::create(x, y, w, h, color, assets);
	TextBox::set_size(w, h);

	options_.clear();
	selection_ = 0;
	arrow_.set_texture(assets.get_texture("arrow.png", {2, 2}));

	set_position(x, y);
}

Text *Dialog::add_option(int x, int y, const std::string &str)
{
	Text text;
	text.create(x_ + x, y_ + y, str, *font_);

	options_.push_back(std::move(text));

	set_selection(0);

	return &options_.back();
}

void Dialog::set_option(int index, std::string const &str)
{
	assert(index > -1 && index < static_cast<int>(options_.size()));
	options_[index].set_string(str);
}

void Dialog::set_selection(int index)
{
	assert(index > -1 && index < static_cast<int>(options_.size()));

	if (options_.empty())
		return;

	selection_ = index;

	update_arrow();
}

void Dialog::draw(Gfx &gfx, glm::mat4 &camera)
{
	TextBox::draw(gfx, camera);

	if (options_.empty())
		return;

	/* Don't draw the arrow if there are no usable options. */
	if (disabled_options_.size() != options_.size())
		gfx.draw_sprite(arrow_, camera);
	
	for (auto &option : options_) {
		gfx.draw_text(option, camera);
	}
}

void Dialog::update_arrow()
{
	if (options_.empty())
		return;

	if (!arrow_.get_texture())
		return;

	arrow_.set_position(options_[selection_].get_position() - glm::vec2(arrow_.get_texture()->frame_width, 0.0f));
}

void Dialog::disable_option(int index)
{
	if (disabled_options_.contains(index))
		return;

	disabled_options_.insert(index);
	if (selection_ == index)
		next();
}

void Dialog::animate(float dt)
{
	arrow_.animate(dt);
}

int Dialog::get_selection() const
{
	return selection_;
}

void Dialog::clear_options()
{
	options_.clear();
}

}
