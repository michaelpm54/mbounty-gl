#ifndef BTY_GAME_VIEW_CHARACTER_HPP_
#define BTY_GAME_VIEW_CHARACTER_HPP_

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

struct Variables;
struct GenVariables;

class ViewCharacter : public bty::Scene {
public:
    ViewCharacter(bty::SceneStack &ss, bty::Assets &assets);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void update(float dt) override;
    void key(int key, int action) override;
    void update_info(const Variables &v, const GenVariables &gen);
    void set_color(bty::BoxColor color);

private:
    bty::SceneStack &ss;
    bty::Sprite frame_;
    bty::Sprite portrait_;
    bty::Rect rect_;
    bty::Text info_[11];
    const bty::Texture *artifacts_[8];
    const bty::Texture *portraits_[4];
    bty::Sprite artifact_sprites_[8];
    bty::Sprite map_sprites_[4];
    const bool *maps_found_;
    const bool *artifacts_found_;
};

#endif    // BTY_GAME_VIEW_CHARACTER_HPP_
