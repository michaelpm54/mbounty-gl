#ifndef BTY_ENGINE_COMPONENT_HPP
#define BTY_ENGINE_COMPONENT_HPP

#include "engine/event-listener.hpp"

class Component : public EventListener {
public:
    virtual ~Component() = default;
    virtual void update(float dt) {};
    virtual void render() {};
    virtual void renderLate() {};
    virtual void load() {};
    virtual void unload() {};
    virtual void enter() {};
    virtual bool isOverlay() const
    {
        return false;
    }
};

#endif    // BTY_ENGINE_COMPONENT_HPP
