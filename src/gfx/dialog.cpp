#include "gfx/dialog.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "gfx/text.hpp"
#include "gfx/texture.hpp"

namespace bty {

void Dialog::next()
{
    if (options_.empty()) {
        spdlog::debug("Empty");
        return;
    }

    int found = -1;

    for (int i = 0; i < options_.size(); i++) {
        int selection = (selection_ + i + 1) % std::max(static_cast<int>(options_.size()), 1);
        if (!disabled_options_[selection] && visible_options_[selection]) {
            found = selection;
            break;
        }
    }

    if (found != -1) {
        draw_arrow_ = true;
        set_selection(found);
    }
    else {
        draw_arrow_ = false;
        spdlog::warn("Dialog::next: No available option found.");
    }
}

void Dialog::prev()
{
    if (options_.empty())
        return;

    int found = -1;

    for (int i = 0; i < options_.size(); i++) {
        int selection = (selection_ - i - 1 + options_.size()) % std::max(static_cast<int>(options_.size()), 1);
        if (!disabled_options_[selection] && visible_options_[selection]) {
            found = selection;
            break;
        }
    }

    if (found != -1) {
        draw_arrow_ = true;
        set_selection(found);
    }
    else {
        draw_arrow_ = false;
        spdlog::warn("Dialog::prev: No available option found.");
    }
}

void Dialog::set_position(int x, int y)
{
    TextBox::set_position(x, y);

    for (int i = 0; i < options_.size(); i++) {
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
    disabled_options_.clear();
    visible_options_.clear();
    arrow_.set_texture(assets.get_texture("arrow.png", {2, 2}));
    selection_ = -1;

    set_position(x, y);
}

Text *Dialog::add_option(int x, int y, const std::string &str)
{
    Text text;
    text.create(x_ + x, y_ + y, str, *font_);

    options_.push_back(std::move(text));
    visible_options_.push_back(true);
    disabled_options_.push_back(false);

    set_selection(0);

    return &options_.back();
}

void Dialog::set_option(int index, std::string const &str)
{
    if (index < 0 || index >= options_.size()) {
        spdlog::warn("Dialog::set_option: {} out of range", index);
        return;
    }
    options_[index].set_string(str);
}

void Dialog::set_selection(int index)
{
    if (options_.empty()) {
        return;
    }

    selection_ = index;

    update_arrow();
}

void Dialog::draw(Gfx &gfx, glm::mat4 &camera)
{
    TextBox::draw(gfx, camera);

    if (options_.empty())
        return;

    if (draw_arrow_)
        gfx.draw_sprite(arrow_, camera);

    for (int i = 0; i < options_.size(); i++) {
        if (visible_options_[i]) {
            gfx.draw_text(options_[i], camera);
        }
    }
}

void Dialog::update_arrow()
{
    if (options_.empty())
        return;

    if (!arrow_.get_texture())
        return;

    if (disabled_options_[selection_] || !visible_options_[selection_]) {
        draw_arrow_ = false;
        return;
    }
    else {
        draw_arrow_ = true;
    }

    arrow_.set_position(options_[selection_].get_position() - glm::vec2(arrow_.get_texture()->frame_width, 0.0f));
}

void Dialog::set_option_disabled(int index, bool disabled)
{
    disabled_options_[index] = disabled;

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
    disabled_options_.clear();
    visible_options_.clear();
    selection_ = -1;
}

void Dialog::set_option_visibility(int index, bool visible)
{
    if (index < 0 || index >= options_.size()) {
        spdlog::warn("Dialog::set_option_visibility: {} out of range", index);
        return;
    }
    visible_options_[index] = visible;
    if (index == selection_) {
        next();
    }
}

}    // namespace bty
