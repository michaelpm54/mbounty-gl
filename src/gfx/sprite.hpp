#ifndef BTY_GFX_SPRITE_HPP_
#define BTY_GFX_SPRITE_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "engine/texture-cache.hpp"
#include "gfx/transformable.hpp"

namespace bty {

struct Animation {
    bool exists {false};
    int curFrame {0};
    int totalFrames {0};
    float secondsPerFrame {0.0f};
    float curTime {0.0f};
    bool repeat {true};
    bool play {true};
    bool done {false};
};

class Sprite : public Transformable {
public:
    Sprite() = default;
    Sprite(const Sprite &) = default;
    Sprite(const Texture *texture, const glm::vec2 &position);
    void setTexture(const Texture *texture);
    const Texture *getTexture() const;
    void update(float dt);
    int getFrame() const;
    void setFlip(bool val);
    bool getFlip() const;
    void setRepeat(bool val);
    bool getRepeat() const;
    void resetAnimation();
    void setAnimationRepeat(bool repeat);
    bool isAnimationDone() const;
    void playAnimation();
    void pauseAnimation();

private:
    void loadAnimation();

protected:
    const Texture *_texture {nullptr};

private:
    bool _flip {false};
    bool _repeat {false};
    Animation _animation;
};

}    // namespace bty

#endif    // BTY_GFX_SPRITE_HPP_