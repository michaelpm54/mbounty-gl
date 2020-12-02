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
    int i = 0;
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            sprites_[i].set_texture(textures_[i]);
            sprites_[i++].set_position(16 + x * 44, 32 + y * 32);
        }
    }
}

void ViewPuzzle::draw(bty::Gfx &gfx, glm::mat4 &camera) {
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
