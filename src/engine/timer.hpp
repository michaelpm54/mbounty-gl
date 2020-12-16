#ifndef ENGINE_TIMER_HPP_
#define ENGINE_TIMER_HPP_

#include <functional>

namespace bty {

class Timer {
public:
    Timer(float duration_s, std::function<void()> callback);
    void set_timer(float duration);
    void tick(float dt);
    void reset();
    void trigger();

private:
    std::function<void()> callback {nullptr};
    float duration {0.0f};
    float timer {0.0f};
};

}    // namespace bty

#endif    // ENGINE_TIMER_HPP_
