#include "engine/dialog-stack.hpp"

#include <spdlog/spdlog.h>

#include "data/color.hpp"
#include "window/glfw.hpp"

namespace bty {

DialogStack::DialogStack(Assets &assets)
    : assets_(assets)
{
}

bool DialogStack::empty() const
{
    return stack_.empty();
}

Dialog *DialogStack::show_dialog(const DialogDef &dialog_, BoxColor color)
{
    BoxColor color_ = color == BoxColor::None ? default_color_ : color;

    auto dialog = std::make_shared<Dialog>();
    dialog->create(dialog_.x, dialog_.y, dialog_.w, dialog_.h, color_, assets_);

    std::vector<Option *> options;

    for (const auto &str : dialog_.strings) {
        dialog->add_line(str.x, str.y, str.str);
    }
    for (const auto &opt : dialog_.options) {
        options.push_back(dialog->add_option(opt.x, opt.y, opt.str));
    }
    for (auto i = 0u; i < dialog_.visible_options.size(); i++) {
        options[i]->set_visible(dialog_.visible_options[i]);
    }
    stack_.push_back({dialog, dialog_});
    return stack_.back().first.get();
}

bool DialogStack::key(int key, int action)
{
    if (stack_.empty() || !stack_.back().second.steals_input) {
        return false;
    }

    std::pair<std::shared_ptr<Dialog>, DialogDef> back;
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_BACKSPACE:
                if (stack_.back().second.callbacks.back) {
                    stack_.back().second.callbacks.back();
                }
                stack_.pop_back();
                break;
            case GLFW_KEY_ENTER:
                back = stack_.back();
                if (stack_.back().second.pop_on_confirm) {
                    stack_.pop_back();
                }
                if (back.second.callbacks.confirm) {
                    back.second.callbacks.confirm(back.first->get_selection());
                }
                break;
            case GLFW_KEY_UP:
                if (stack_.back().second.callbacks.up) {
                    stack_.back().second.callbacks.up(*stack_.back().first);
                }
                else {
                    stack_.back().first->prev();
                }
                break;
            case GLFW_KEY_DOWN:
                if (stack_.back().second.callbacks.down) {
                    stack_.back().second.callbacks.down(*stack_.back().first);
                }
                else {
                    stack_.back().first->next();
                }
                break;
            case GLFW_KEY_LEFT:
                if (stack_.back().second.callbacks.left) {
                    stack_.back().second.callbacks.left(*stack_.back().first);
                }
                break;
            case GLFW_KEY_RIGHT:
                if (stack_.back().second.callbacks.right) {
                    stack_.back().second.callbacks.right(*stack_.back().first);
                }
                break;
            default:
                break;
        }
    }

    return true;
}

void DialogStack::draw(Gfx &gfx, glm::mat4 &camera)
{
    if (stack_.empty()) {
        return;
    }

    for (auto &[dialog, d] : stack_) {
        dialog->draw(gfx, camera);
    }
}

void DialogStack::set_default_color(BoxColor color)
{
    default_color_ = color;
}

void DialogStack::update(float dt)
{
    if (stack_.empty()) {
        return;
    }

    for (auto &[dialog, _] : stack_) {
        dialog->update(dt);
    }
}

void DialogStack::pop()
{
    if (stack_.empty()) {
        return;
    }

    stack_.pop_back();
}

}    // namespace bty
