#ifndef BTY_GFX_SPRITE_HPP_
#define BTY_GFX_SPRITE_HPP_

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "engine/assets.hpp"
#include "gfx/transformable.hpp"

namespace bty {

struct Animation {
    bool exists {false};
    int current_frame {0};
    int total_frames {0};
    float time_per_frame {0.0f};
    float current_time {0.0f};
    bool repeat {true};
    bool play {true};
    bool done {false};
};

class Sprite : public Transformable {
public:
    Sprite() = default;
    Sprite(const Sprite &) = default;
    Sprite(const Texture *texture, const glm::vec2 &position);
    void set_texture(const Texture *texture);
    const Texture *get_texture() const;
    void update(float dt);
    int get_frame() const;
    void set_flip(bool val);
    bool get_flip() const;
    void set_repeat(bool val);
    bool get_repeat() const;
    void reset_animation();
    void set_animation_repeat(bool repeat);
    bool is_animation_done() const;
    void play_animation();
    void pause_animation();

private:
    void load_animation();

protected:
    const Texture *texture_ {nullptr};

private:
    bool flip_ {false};
    bool repeat_ {false};
    Animation animation_;
};

}    // namespace bty

#endif    // BTY_GFX_SPRITE_HPP_