#include "gfx/sprite.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtx/transform.hpp>

namespace bty {

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

void Sprite::load_animation() {
    animation_.exists = true;
    animation_.total_frames = texture_->num_frames_x * texture_->num_frames_y;
    animation_.time_per_frame = 0.15f;
    animation_.current_frame = rand() % animation_.total_frames;
}

void Sprite::animate(float dt) {
    if (!animation_.exists) {
        return;
    }

    animation_.current_time += dt;
    if (animation_.current_time >= animation_.time_per_frame) {
        animation_.current_frame = (animation_.current_frame + 1) % animation_.total_frames;
        animation_.current_time = animation_.current_time - animation_.time_per_frame;
    }
}

int Sprite::get_frame() const {
    return animation_.current_frame;
}

void Sprite::set_flip(bool val) {
    flip_ = val;
}

bool Sprite::get_flip() const {
    return flip_;
}

}    // namespace bty
