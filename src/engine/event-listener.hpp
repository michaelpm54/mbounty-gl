#ifndef BTY_ENGINE_EVENT_LISTENER_HPP
#define BTY_ENGINE_EVENT_LISTENER_HPP

#include "engine/events.hpp"

class EventListener {
public:
    virtual bool handleEvent(Event event)
    {
        return false;
    }

    virtual bool handleKey(Key key)
    {
        return false;
    }
};

#endif    // BTY_ENGINE_EVENT_LISTENER_HPP
