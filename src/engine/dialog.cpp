#include "engine/dialog.hpp"

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "gfx/text.hpp"
#include "gfx/texture.hpp"

namespace bty {

void Dialog::next()
{
    if (options_.empty()) {
        return;
    }

    int found = -1;

    for (auto i = 0u; i < options_.size(); i++) {
        int selection = (selection_ + i + 1) % std::max(static_cast<int>(options_.size()), 1);
        if (options_[selection].enabled() && options_[selection].visible()) {
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
    if (options_.empty()) {
        return;
    }

    int found = -1;

    for (int i = 0; i < static_cast<int>(options_.size()); i++) {
        int selection = (selection_ - i - 1 + options_.size()) % std::max(static_cast<int>(options_.size()), 1);
        if (options_[selection].enabled() && options_[selection].visible()) {
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

    for (auto i = 0u; i < options_.size(); i++) {
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
    arrow_.set_texture(assets.get_texture("arrow.png", {2, 2}));
    selection_ = -1;

    set_position(x, y);
}

Option *Dialog::add_option(int x, int y, const std::string &str)
{
    Option opt;
    opt.create(x_ + x, y_ + y, str, *font_);

    options_.push_back(std::move(opt));

    set_selection(0);

    return &options_.back();
}

void Dialog::set_option(int index, std::string const &str)
{
    if (index < 0 || index >= static_cast<int>(options_.size())) {
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

    for (auto &opt : options_) {
        if (opt.visible()) {
            gfx.draw_text(opt, camera);
        }
    }
}

void Dialog::update_arrow()
{
    if (options_.empty())
        return;

    if (!arrow_.get_texture())
        return;

    if (!options_[selection_].enabled() || !options_[selection_].visible()) {
        draw_arrow_ = false;
        return;
    }
    else {
        draw_arrow_ = true;
    }

    arrow_.set_position(options_[selection_].get_position() - glm::vec2(arrow_.get_texture()->frame_width, 0.0f));
}

void Dialog::update(float dt)
{
    arrow_.update(dt);
}

int Dialog::get_selection() const
{
    return selection_;
}

void Dialog::clear_options()
{
    options_.clear();
    selection_ = -1;
}

std::deque<Option> &Dialog::get_options()
{
    return options_;
}

void Option::enable()
{
    enabled_ = true;
}

void Option::disable()
{
    enabled_ = false;
}

bool Option::enabled() const
{
    return enabled_;
}

void Option::show()
{
    visible_ = true;
}

void Option::hide()
{
    visible_ = false;
}

bool Option::visible() const
{
    return visible_;
}

void Option::set_enabled(bool val)
{
    enabled_ = val;
}

void Option::set_visible(bool val)
{
    visible_ = val;
}

}    // namespace bty
