#include "game/view-continent.hpp"

#include <spdlog/spdlog.h>

#include <glm/trigonometric.hpp>

#include "gfx/gfx.hpp"
#include "assets.hpp"
#include "gfx/texture.hpp"
#include "shared-state.hpp"
#include "bounty.hpp"
#include "game/map.hpp"

ViewContinent::ViewContinent()
    : map_texture_({64, 64, GL_NONE, 1, 1, 64, 64})
{
}

ViewContinent::~ViewContinent() {
    glDeleteTextures(1, &map_texture_.handle);
}

void ViewContinent::load(bty::Assets &assets, bty::BoxColor color) {
    box_.create(6, 4, 20, 22, color, assets);
    continent_ = box_.add_line(5, 1, "");
    coordinates_ = box_.add_line(1, 20, "");

    glCreateTextures(GL_TEXTURE_2D, 1, &map_texture_.handle);
    glTextureStorage2D(map_texture_.handle, 1, GL_RGBA8, 64, 64);
    glTextureParameterf(map_texture_.handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameterf(map_texture_.handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameterf(map_texture_.handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameterf(map_texture_.handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    map_.set_texture(&map_texture_);
    map_.set_position(64, 56);
    map_.set_size(128, 128);
}

void ViewContinent::draw(bty::Gfx &gfx, glm::mat4 &camera) {
    box_.draw(gfx, camera);
    gfx.draw_sprite(map_, camera);
}

void ViewContinent::view(int x, int y, int continent, const unsigned char * const map) {
    x_ = x;
    y_ = y;
    dot_timer_ = 0;

    continent_->set_string(kContinents[continent]);
    coordinates_->set_string(fmt::format("X={:>2} Position Y={:>2}", x, 63-y));

    std::vector<unsigned char> pixels(64*64*4);
    unsigned char *p = pixels.data();

    /* ARGB */
    static constexpr uint32_t cyan = 0xFF00AAAA;
    static constexpr uint32_t green = 0xFF00BB00;
    static constexpr uint32_t dark_green = 0xFF00AA00;
    static constexpr uint32_t yellow = 0xFFCCCC00;
    static constexpr uint32_t brown = 0xFF786316;
    static constexpr uint32_t grey = 0xFFAAAAAA;
    static constexpr uint32_t black = 0xFF000000;

    for (int i = 0; i < 4096; i++) {
        int id = map[i];
        if (id == 0xFF) {
            std::memcpy(p+i*4, &black, 4);
        }
        else if (id <= GrassInFrontOfCastle) {
            std::memcpy(p+i*4, &green, 4);
        }
        else if (id >= WaterInTopRight && id <= WaterOpen) {
            std::memcpy(p+i*4, &cyan, 4);
        }
        else if (id >= TreeOutBottomRight && id <= TreeOpen) {
            std::memcpy(p+i*4, &dark_green, 4);
        }
        else if (id >= SandUnk0 && id <= SandOpen) {
            std::memcpy(p+i*4, &yellow, 4);
        }
        else if (id >= RockOutTopLeft && id <= RockOpen) {
            std::memcpy(p+i*4, &brown, 4);
        }
        else if (id >= CastleTopLeft && id <= CastleBottomRight) {
            std::memcpy(p+i*4, &grey, 4);
        }
        else {
            std::memcpy(p+i*4, &dark_green, 4);
        }
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 64);

    glTextureSubImage2D(
        map_texture_.handle,
        0, 0, 0,
        64, 64,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        pixels.data()
    );
}

void ViewContinent::update(float dt) {
    dot_timer_ += dt;
    dot_alpha_ = glm::abs(glm::cos(dot_timer_ * 4));

    uint32_t pixel = 0xFF000000 | (static_cast<uint32_t>(0xFF * dot_alpha_) << 16);
    
    glTextureSubImage2D(
        map_texture_.handle,
        0, x_, y_,
        1, 1,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        &pixel
    );
}

void ViewContinent::set_color(bty::BoxColor color) {
    box_.set_color(color);
}
