#ifndef ENGINE_TIMER_HPP_
#define ENGINE_TIMER_HPP_

#include <functional>

namespace bty {

class Timer {
public:
    Timer(float durationSeconds, std::function<void()> callback);
    void setDuration(float durationSeconds);
    void tick(float dt);
    void reset();
    void trigger();

private:
    std::function<void()> _callback {nullptr};
    float _durationSeconds {0.0f};
    float _elapsedTime {0.0f};
};

}    // namespace bty

#endif    // ENGINE_TIMER_HPP_
