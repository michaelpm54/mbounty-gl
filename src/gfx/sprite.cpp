#include "gfx/sprite.hpp"

#include <spdlog/spdlog.h>

#include <glm/gtx/transform.hpp>

namespace bty {

Sprite::Sprite(const Texture *texture, const glm::vec2 &position)
{
    setTexture(texture);
    setPosition(position);
}

void Sprite::setTexture(const Texture *texture)
{
    if (!texture) {
        spdlog::warn("Sprite::set_texture: nullptr");
    }
    else if (texture == _texture) {
        return;
    }

    _texture = texture;

    if (texture) {
        setSize(static_cast<float>(texture->frameW), static_cast<float>(texture->frameH));
        if (texture->framesX && texture->framesY) {
            loadAnimation();
        }
    }
}

const Texture *Sprite::getTexture() const
{
    return _texture;
}

void Sprite::loadAnimation()
{
    _animation.exists = true;
    _animation.totalFrames = _texture->framesX * _texture->framesY;
    _animation.secondsPerFrame = 0.15f;
    _animation.curFrame = rand() % _animation.totalFrames;
}

void Sprite::update(float dt)
{
    if (!_animation.exists || !_animation.play) {
        return;
    }

    _animation.curTime += dt;
    if (_animation.curTime >= _animation.secondsPerFrame) {
        _animation.curFrame++;
        if (!_animation.repeat && _animation.curFrame == _animation.totalFrames) {
            _animation.play = false;
            _animation.done = true;
            return;
        }
        _animation.curFrame %= _animation.totalFrames;
        _animation.curTime = _animation.curTime - _animation.secondsPerFrame;
    }
}

int Sprite::getFrame() const
{
    return _animation.curFrame;
}

void Sprite::setFlip(bool val)
{
    _flip = val;
}

bool Sprite::getFlip() const
{
    return _flip;
}

void Sprite::setRepeat(bool val)
{
    if (_texture && (_texture->framesX > 1 || _texture->framesY > 1)) {
        spdlog::warn("Repeat is set for a sprite with multiple frames. This is not implemented.");
    }
    _repeat = val;
}

bool Sprite::getRepeat() const
{
    return _repeat;
}

void Sprite::resetAnimation()
{
    if (!_animation.exists) {
        return;
    }

    _animation.curFrame = 0;
    _animation.curTime = 0;
    _animation.play = true;
    _animation.done = false;
}

void Sprite::setAnimationRepeat(bool repeat)
{
    if (!_animation.exists) {
        return;
    }

    _animation.repeat = repeat;
}

void Sprite::playAnimation()
{
    _animation.play = true;
}

void Sprite::pauseAnimation()
{
    _animation.play = false;
}

bool Sprite::isAnimationDone() const
{
    return _animation.done;
}

}    // namespace bty
