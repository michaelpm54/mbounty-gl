#include "game/view-continent.hpp"

#include <spdlog/spdlog.h>

#include <glm/trigonometric.hpp>

#include "assets.hpp"
#include "bounty.hpp"
#include "game/map.hpp"
#include "game/scene-stack.hpp"
#include "game/variables.hpp"
#include "gfx/gfx.hpp"
#include "gfx/texture.hpp"
#include "glfw.hpp"

ViewContinent::ViewContinent(SceneStack &ss, bty::Assets &assets)
    : ss(ss)
    , map_texture_({64, 64, GL_NONE, 1, 1, 64, 64})
{
    box_.create(6, 4, 20, 22, bty::BoxColor::Intro, assets);
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

ViewContinent::~ViewContinent()
{
    glDeleteTextures(1, &map_texture_.handle);
}

void ViewContinent::draw(bty::Gfx &gfx, glm::mat4 &camera)
{
    box_.draw(gfx, camera);
    gfx.draw_sprite(map_, camera);
}

void ViewContinent::update_info(Variables &v, bool have_map, bool force_show)
{
    this->v = &v;
    this->have_map = have_map;
    fog = !force_show;

    set_color(bty::get_box_color(v.diff));

    x_ = v.x;
    y_ = v.y;

    continent_->set_string(kContinents[v.continent]);
    coordinates_->set_string(fmt::format("X={:>2} Position Y={:>2}", v.x, 63 - v.y));

    gen_texture();
}

void ViewContinent::update(float dt)
{
    dot_timer_ += dt;
    dot_alpha_ = glm::abs(glm::cos(dot_timer_ * 4));

    uint8_t val = static_cast<uint8_t>(255 * dot_alpha_);

    /* Cyan */
    uint32_t pixel = 0xFF000000 | (val << 8) | val;

    glTextureSubImage2D(
        map_texture_.handle,
        0,
        x_,
        y_,
        1,
        1,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        &pixel);
}

void ViewContinent::set_color(bty::BoxColor color)
{
    box_.set_color(color);
}

void ViewContinent::key(int key, int action)
{
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_BACKSPACE:
                ss.pop(0);
                break;
            case GLFW_KEY_ENTER:
                if (have_map) {
                    fog = !fog;
                    gen_texture();
                }
                break;
            default:
                break;
        }
    }
}

void ViewContinent::gen_texture()
{
    /* ARGB */
    static constexpr uint32_t water_edge = 0xFF2161C7;
    static constexpr uint32_t water_deep = 0xFF002084;
    static constexpr uint32_t grass = 0xFF21A300;
    static constexpr uint32_t trees = 0xFF006100;
    static constexpr uint32_t rocks = 0xFF844100;
    static constexpr uint32_t black = 0xFF000000;
    static constexpr uint32_t yellow = 0xFFCCCC00;
    static constexpr uint32_t castle = 0xFFe8E4E8;

    const unsigned char *const map = fog ? v->visited_tiles[v->continent].data() : v->tiles[v->continent];

    std::vector<unsigned char> pixels(64 * 64 * 4);
    unsigned char *p = pixels.data();

    for (int i = 0; i < 4096; i++) {
        int id = map[i];
        if (id == 0xFF) {
            std::memcpy(p + i * 4, &black, 4);
        }
        else if (id <= Tile_GrassInFrontOfCastle) {
            std::memcpy(p + i * 4, &grass, 4);
        }
        else if (id >= Tile_WaterIRT && id < Tile_Water) {
            std::memcpy(p + i * 4, &water_edge, 4);
        }
        else if (id == Tile_Water) {
            std::memcpy(p + i * 4, &water_deep, 4);
        }
        else if (id >= Tile_TreeERB && id <= Tile_Tree) {
            std::memcpy(p + i * 4, &trees, 4);
        }
        else if (id >= Tile_SandELT && id <= Tile_Sand) {
            std::memcpy(p + i * 4, &yellow, 4);
        }
        else if (id >= Tile_RockELT && id <= Tile_Rock) {
            std::memcpy(p + i * 4, &rocks, 4);
        }
        else if (id >= Tile_CastleLT && id <= Tile_CastleRB) {
            std::memcpy(p + i * 4, &castle, 4);
        }
        else {
            std::memcpy(p + i * 4, &trees, 4);
        }
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 64);

    glTextureSubImage2D(
        map_texture_.handle,
        0,
        0,
        0,
        64,
        64,
        GL_BGRA,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        pixels.data());
}
