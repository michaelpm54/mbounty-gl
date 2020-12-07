#include "game/recruit-input.hpp"

#include <spdlog/spdlog.h>

#include "glfw.hpp"

void RecruitInput::set_max(int max)
{
    max_ = max;

    if (cur_ > max_) {
        cur_ = max_;
    }
}

void RecruitInput::key(int key, int action)
{
    switch (action) {
        case GLFW_PRESS:
            switch (key) {
                case GLFW_KEY_DOWN:
                    [[fallthrough]];
                case GLFW_KEY_LEFT:
                    decreasing_ = true;
                    increment_ = -1;
                    timer_ = 0.099f;
                    break;
                case GLFW_KEY_UP:
                    [[fallthrough]];
                case GLFW_KEY_RIGHT:
                    increasing_ = true;
                    increment_ = 1;
                    timer_ = 0.099f;
                    break;
                default:
                    break;
            }
            break;
        case GLFW_RELEASE:
            switch (key) {
                case GLFW_KEY_RIGHT:
                    [[fallthrough]];
                case GLFW_KEY_UP:
                    increasing_ = false;
                    increment_ = 0;
                    amount_added_while_holding_ = 0;
                    timer_ = 0;
                    break;
                case GLFW_KEY_LEFT:
                    [[fallthrough]];
                case GLFW_KEY_DOWN:
                    decreasing_ = false;
                    increment_ = 0;
                    amount_added_while_holding_ = 0;
                    timer_ = 0;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void RecruitInput::update(float dt)
{
    if (increasing_) {
        timer_ += dt;
        if (amount_added_while_holding_ == 10) {
            increment_ = 10;
        }
    }
    else if (decreasing_) {
        timer_ += dt;
        if (amount_added_while_holding_ == -10) {
            increment_ = -10;
        }
    }

    if (timer_ > 0.1f) {
        cur_ += increment_;
        amount_added_while_holding_ += increment_;
        timer_ = 0;
        if (cur_ < 0) {
            cur_ = 0;
        }
        if (cur_ > max_) {
            cur_ = max_;
        }
        cur_ = max_ < cur_ ? max_ : cur_;
    }
}

int RecruitInput::get_current() const
{
    return cur_;
}

void RecruitInput::clear()
{
    max_ = 0;
    cur_ = 0;
    timer_ = 0;
    amount_added_while_holding_ = 0;
    increasing_ = false;
    decreasing_ = false;
    increment_ = 0;
}
