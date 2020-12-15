#include "engine/textbox.hpp"

#include <spdlog/spdlog.h>

#include "gfx/font.hpp"
#include "gfx/gfx.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {

/* clang-format off */
void TextBox::create(
	int x, int y,
	int w, int h,
	bty::BoxColor color,
	bty::Assets &assets
)
/* clang-format on */
{
    font_ = &assets.get_font();
    lines_.clear();

    auto &border = assets.get_border();

    for (int i = 0; i < 8; i++) {
        box_[i].set_texture(border[i]);
    }

    set_color(color);

    set_size(w, h);
    set_position(x, y);
}

void TextBox::set_size(int w, int h)
{
    /* Minus one here because each border takes 4 pixels. 2x4 = 8 = 1 cell. */
    width_ = static_cast<float>(w - 1) * 8;
    height_ = static_cast<float>(h - 1) * 8;

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
    box_[1].set_position({x + 4, y});
    box_[2].set_position({x + 4 + width_, y});
    box_[3].set_position({x + 4 + width_, y + 4});
    box_[4].set_position({x + 4 + width_, y + 4 + height_});
    box_[5].set_position({x + 4, y + 4 + height_});
    box_[6].set_position({x, y + 4 + height_});
    box_[7].set_position({x, y + 4});

    background_.set_position({x + 5, y + 5});
    background_outline_.set_position({x + 4, y + 4});
}

void TextBox::draw(Gfx &gfx, glm::mat4 &camera)
{
    for (int i = 0; i < 8; i++)
        gfx.draw_sprite(box_[i], camera);

    gfx.draw_rect(background_outline_, camera);
    gfx.draw_rect(background_, camera);

    for (auto i = 0u; i < lines_.size(); i++) {
        if (lines_visible_[i]) {
            gfx.draw_text(lines_[i], camera);
        }
    }
}

Text *TextBox::add_line(int x, int y, std::string const &str)
{
    if (!font_) {
        spdlog::warn("TextBox::add_line: no font");
        return nullptr;
    }

    Text text;
    text.create(x_ + x, y_ + y, str, *font_);
    lines_.push_back(std::move(text));
    lines_visible_.push_back(true);

    return &lines_.back();
}

void TextBox::set_line(int i, std::string const &str)
{
    if (i < 0) {
        spdlog::warn("TextBox::set_line: i < 0");
    }
    else if (i >= static_cast<int>(lines_.size())) {
        spdlog::warn("TextBox::set_line: didn't exist so adding '{}'", str);
        Text text;
        text.create(x_ + 2, y_ + i, str, *font_);
        lines_.push_back(std::move(text));
    }
    else {
        lines_[i].set_string(str);
    }
}

void TextBox::set_color(bty::BoxColor color)
{
    background_outline_.set_color(bty::get_color(color, true));
    background_.set_color(bty::get_color(color, false));
}

void TextBox::set_line_visible(int index, bool value)
{
    if (index < 0 || index >= static_cast<int>(lines_.size())) {
        spdlog::warn("TextBox::set_line_visible: {} out of range", index);
        return;
    }
    lines_visible_[index] = value;
}

void TextBox::clear()
{
    lines_.clear();
    lines_visible_.clear();
}

}    // namespace bty
