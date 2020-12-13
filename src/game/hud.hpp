#ifndef GAME_HUD_HPP_
#define GAME_HUD_HPP_

#include <functional>
#include <glm/mat4x4.hpp>
#include <vector>

#include "bounty.hpp"
#include "gfx/rect.hpp"
#include "gfx/sprite.hpp"
#include "gfx/text.hpp"

namespace bty {
class Assets;
class Font;
class Gfx;
struct Texture;
}    // namespace bty

struct SharedState;

class Hud {
public:
    Hud() = default;
    Hud(bty::Assets &assets);

    void draw(bty::Gfx &gfx, glm::mat4 &camera);
    void update(float dt);
    bty::Sprite *get_contract();
    void set_timestop(int amount);
    void clear_timestop();

    void set_error(const std::string &msg, std::function<void()> then = nullptr);
    void set_title(const std::string &msg); /* Similar to set_error except it doesn't take input. */
    void clear_error();
    bool get_error() const;

    void set_blank_frame();
    void set_hud_frame();
    void set_color(bty::BoxColor color);

    void set_hero(int hero, int rank);
    void set_days(int days);
    void set_contract(int contract);
    void set_magic(bool val);
    void set_siege(bool val);
    void set_puzzle(bool *villains, bool *artifacts);
    void set_gold(int gold);

    void update_state()
    {
    }

private:
    const bty::Texture *blank_frame_;
    const bty::Texture *hud_frame_;

    bty::Sprite frame_;
    bty::Rect top_bar_;
    bty::Text name_;
    bty::Text days_;
    bty::Text timestop_string_;
    bty::Sprite contract_;
    bty::Sprite siege_;
    bty::Sprite magic_;
    bty::Sprite puzzle_;
    bty::Sprite money_;
    std::vector<const bty::Texture *> contract_textures_;

    const bty::Texture *siege_no {nullptr};
    const bty::Texture *siege_yes {nullptr};

    const bty::Texture *magic_no {nullptr};
    const bty::Texture *magic_yes {nullptr};

    bty::Sprite pieces_[25];
    bool hide_piece_[25];

    bty::Sprite gold_[30];
    int num_gold_coins_ {0};
    int num_silver_coins_ {0};
    int num_copper_coins_ {0};

    bool timestop_ {false};

    bty::Text error_text;
    std::function<void()> error_then_;
    bool error {false};
};

#endif    // GAME_HUD_HPP_
