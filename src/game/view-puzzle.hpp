#ifndef BTY_GAME_VIEW_PUZZLE_HPP_
#define BTY_GAME_VIEW_PUZZLE_HPP_

#include "engine/scene.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
class SceneStack;
}    // namespace bty

struct GenVariables;

class ViewPuzzle : public bty::Scene {
public:
    ViewPuzzle(bty::SceneStack &ss, bty::Assets &assets);

    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;

    void update_info(const GenVariables &gen);

private:
    bty::SceneStack &ss;
    const bty::Texture *textures_[25] {nullptr};
    bty::Sprite sprites_[25];
    bty::Sprite border_[8];
    bool hide_[25];
    int to_hide_[25];
    float pop_timer_ {0};
    int next_pop_ {-1};
    bool done_ {false};
    bool debug {false};
};

#endif    // BTY_GAME_VIEW_PUZZLE_HPP_
