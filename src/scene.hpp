#ifndef BTY_SCENE_HPP_
#define BTY_SCENE_HPP_

namespace bty {

struct Gfx;
class Assets;

class Scene {
public:
    virtual ~Scene() = default;

    virtual bool load(Assets &assets) = 0;
    virtual void draw(Gfx &gfx) = 0;
    virtual void key(int key, int scancode, int action, int mods) = 0;
    virtual void enter() = 0;
    virtual bool loaded() = 0;
    virtual void update(float dt) = 0;

private:
};

}    // namespace bty

#endif    // BTY_SCENE_HPP_
