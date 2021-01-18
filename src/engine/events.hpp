#ifndef BTY_ENGINE_EVENTS_HPP
#define BTY_ENGINE_EVENTS_HPP

#include "window/keys.hpp"

enum class EventId {
    KeyDown,
    KeyUp,
    MouseMove,
    Quit,
};

struct Event {
    EventId id;
    union {
        Key key;
        float motion[2];
    };
};

#endif    // BTY_ENGINE_EVENTS_HPP
