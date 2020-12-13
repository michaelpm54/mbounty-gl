#ifndef BTY_GAME_VIEW_CONTRACT_HPP_
#define BTY_GAME_VIEW_CONTRACT_HPP_

#include "game/scene.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"
#include "gfx/textbox.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
}    // namespace bty

class SceneStack;
struct GenVariables;
struct Variables;

class ViewContract : public Scene {
public:
    ViewContract(SceneStack &ss, bty::Assets &assets, Variables &v, GenVariables &gen, bty::Sprite *contract_sprite);
    void draw(bty::Gfx &gfx, glm::mat4 &camera) override;
    void view(int contract, const std::string &castle, bty::Sprite *contract_sprite);
    void key(int key, int action);
    void update(float dt);
    void update_info();
    void set_color(bty::BoxColor color);

private:
    SceneStack &ss;
    Variables &v;
    GenVariables &gen;
    bty::TextBox box_;
    bty::Text no_contract_text_;
    bool no_contract_ {false};
    bty::Sprite *contract_sprite_ {nullptr};
    bty::Text info_;
};

#endif    // BTY_GAME_VIEW_CONTRACT_HPP_
