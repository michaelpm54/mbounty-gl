#include "game/view-puzzle.hpp"

#include <algorithm>

#include <spdlog/spdlog.h>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"

void ViewPuzzle::load(bty::Assets &assets) {
    static constexpr int villain_positions[17] = {
        15, 3, 23, 5, 19, 1, 9, 21, 7, 18, 11, 8, 17, 6, 13, 16, 12,
    };
    for (int i = 0; i < 17; i++) {
        textures_[villain_positions[i]] = assets.get_texture(fmt::format("villains/{}.png", i));
    }
    static constexpr int artifact_positions[8] = {
        10, 2, 4, 14, 20, 24, 22, 0,
    };
    for (int i = 0; i < 8; i++) {
        textures_[artifact_positions[i]] = assets.get_texture(fmt::format("artifacts/44x32/{}.png", i));
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
    border_[5].set_texture(assets.get_texture(fmt::format("border-puzzle/{}.png", 9)));

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
    border_[1].set_position({x+10, y});
    border_[2].set_position({x+10 + width, y});
    border_[3].set_position({x+10 + width, y+16});
    border_[4].set_position({x+10 + width, y+16+height});
    border_[5].set_position({x+10, y+16+height});
    border_[6].set_position({x, y+16 + height});
    border_[7].set_position({x, y+16});
}

void ViewPuzzle::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    for (int i = 0; i < 8; i++) {
        gfx.draw_sprite(border_[i], camera);
    }
    for (int i = 0; i < 25; i++) {
        gfx.draw_sprite(sprites_[i], camera);
    }
}

void ViewPuzzle::view(const SharedState &state) {
    
}

void ViewPuzzle::update(float dt) {
    for (int i = 0; i < 25; i++) {
        sprites_[i].animate(dt);
    }
}
