#include "engine/timer.hpp"

#include <spdlog/spdlog.h>

namespace bty {

Timer::Timer(float duration, std::function<void()> callback)
    : callback(std::move(callback))
    , duration(duration)
{
}

void Timer::set_timer(float duration)
{
    this->duration = duration;
}

void Timer::tick(float dt)
{
    timer += dt;
    if (timer >= duration) {
        timer = 0.0f;
        if (callback) {
            callback();
        }
        else {
            spdlog::warn("Timer without a callback");
        }
    }
}

void Timer::reset()
{
    timer = 0.0f;
}

void Timer::trigger()
{
    if (callback) {
        callback();
    }
    else {
        spdlog::warn("Timer without a callback");
    }
}

}    // namespace bty
