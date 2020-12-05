#include "gfx/sprite.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtx/transform.hpp>

namespace bty {

Sprite::Sprite(const Texture *texture, const glm::vec2 &position)
{
    set_texture(texture);
    set_position(position);
}

void Sprite::set_texture(const Texture *texture)
{
    if (!texture) {
        spdlog::warn("Sprite::set_texture: nullptr");
        return;
    }
    else if (texture == texture_) {
        return;
    }

    texture_ = texture;

    set_size(texture->frame_width, texture->frame_height);

    if (texture->num_frames_x && texture->num_frames_y) {
        load_animation();
    }
}

const Texture *Sprite::get_texture() const
{
    return texture_;
}

void Sprite::load_animation()
{
    animation_.exists = true;
    animation_.total_frames = texture_->num_frames_x * texture_->num_frames_y;
    animation_.time_per_frame = 0.15f;
    animation_.current_frame = rand() % animation_.total_frames;
}

void Sprite::animate(float dt)
{
    if (!animation_.exists || !animation_.play) {
        return;
    }

    animation_.current_time += dt;
    if (animation_.current_time >= animation_.time_per_frame) {
        animation_.current_frame++;
        if (!animation_.repeat && animation_.current_frame == animation_.total_frames) {
            animation_.play = false;
            animation_.done = true;
            spdlog::debug("Animation done!");
            return;
        }
        animation_.current_frame %= animation_.total_frames;
        animation_.current_time = animation_.current_time - animation_.time_per_frame;
    }
}

int Sprite::get_frame() const
{
    return animation_.current_frame;
}

void Sprite::set_flip(bool val)
{
    flip_ = val;
}

bool Sprite::get_flip() const
{
    return flip_;
}

void Sprite::set_repeat(bool val)
{
    if (texture_ && (texture_->num_frames_x > 1 || texture_->num_frames_y > 1)) {
        spdlog::warn("Repeat is set for a sprite with multiple frames. This is not implemented.");
    }
    repeat_ = val;
}

bool Sprite::get_repeat() const
{
    return repeat_;
}

void Sprite::reset_animation()
{
    if (!animation_.exists) {
        return;
    }

    animation_.current_frame = 0;
    animation_.current_time = 0;
    animation_.play = true;
    animation_.done = false;
}

void Sprite::set_animation_repeat(bool repeat)
{
    if (!animation_.exists) {
        return;
    }

    animation_.repeat = repeat;
}

void Sprite::play_animation()
{
    animation_.play = true;
}

void Sprite::pause_animation()
{
    animation_.play = false;
}

bool Sprite::is_animation_done() const
{
    return animation_.done;
}

}    // namespace bty
