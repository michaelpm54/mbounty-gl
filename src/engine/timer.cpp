#include "engine/timer.hpp"

#include <spdlog/spdlog.h>

namespace bty {

Timer::Timer(float durationSeconds, std::function<void()> callback)
    : _callback(std::move(callback))
    , _durationSeconds(durationSeconds)
{
}

void Timer::setDuration(float durationSeconds)
{
    _durationSeconds = durationSeconds;
}

void Timer::tick(float dt)
{
    _elapsedTime += dt;
    if (_elapsedTime >= _durationSeconds) {
        _elapsedTime = 0.0f;
        trigger();
    }
}

void Timer::reset()
{
    _elapsedTime = 0.0f;
}

void Timer::trigger()
{
    if (_callback) {
        _callback();
    }
    else {
        spdlog::warn("Timer without a callback");
    }
}

}    // namespace bty
