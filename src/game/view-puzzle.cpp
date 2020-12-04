#include "game/view-puzzle.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>

#include "assets.hpp"
#include "bounty.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"

void ViewPuzzle::load(bty::Assets &assets)
{
    for (int i = 0; i < 17; i++) {
        textures_[kPuzzleVillainPositions[i]] = assets.get_texture(fmt::format("villains/{}.png", i));
    }
    for (int i = 0; i < 8; i++) {
        textures_[kPuzzleArtifactPositions[i]] = assets.get_texture(fmt::format("artifacts/44x32/{}.png", i));
    }
    int n = 0;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            sprites_[n].set_texture(textures_[n]);
            sprites_[n++].set_position(18 + x * 44, 40 + y * 32);
        }
    }

    float x = 8;
    float y = 24;
    float width = 5 * 44;
    float height = 5 * 32;

    for (int i = 0; i < 8; i++) {
        border_[i].set_texture(assets.get_texture(fmt::format("border-puzzle/{}.png", i)));
    }

    // top bottom
    border_[1].set_size(width, 16);
    border_[5].set_size(width, 16);
    border_[1].set_repeat(true);
    border_[5].set_repeat(true);

    // left right
    border_[3].set_size(10, height);
    border_[7].set_size(10, height);
    border_[3].set_repeat(true);
    border_[7].set_repeat(true);

    border_[0].set_position({x, y});
    border_[1].set_position({x + 10, y});
    border_[2].set_position({x + 10 + width, y});
    border_[3].set_position({x + 10 + width, y + 16});
    border_[4].set_position({x + 10 + width, y + 16 + height});
    border_[5].set_position({x + 10, y + 16 + height});
    border_[6].set_position({x, y + 16 + height});
    border_[7].set_position({x, y + 16});
}

void ViewPuzzle::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    for (int i = 0; i < 8; i++) {
        gfx.draw_sprite(border_[i], camera);
    }
    for (int i = 0; i < 25; i++) {
        if (!hide_[i]) {
            gfx.draw_sprite(sprites_[i], camera);
        }
    }
}

void ViewPuzzle::view(bool *villains, bool *artifacts)
{
    for (int i = 0; i < 25; i++) {
        to_hide_[i] = -1;
    }
    int sprite_index = 0;
    for (int i = 0; i < 17; i++) {
        if (villains[i]) {
            to_hide_[sprite_index++] = kPuzzleVillainPositions[i];
        }
    }
    for (int i = 0; i < 8; i++) {
        if (artifacts[i]) {
            to_hide_[sprite_index++] = kPuzzleArtifactPositions[i];
        }
    }
    for (int i = 0; i < 25; i++) {
        hide_[i] = false;
    }
    if (sprite_index) {
        next_pop_ = 0;
        done_ = false;
    }
    else {
        next_pop_ = -1;
        done_ = true;
    }
}

void ViewPuzzle::update(float dt)
{
    if (!done_) {
        pop_timer_ -= dt;
        if (pop_timer_ <= 0) {
            pop_timer_ = 0.3f;
            hide_[to_hide_[next_pop_++]] = true;
            if (next_pop_ == 25 || to_hide_[next_pop_] == -1) {
                done_ = true;
            }
        }
    }

    for (int i = 0; i < 25; i++) {
        if (!hide_[i]) {
            sprites_[i].animate(dt);
        }
    }
}
