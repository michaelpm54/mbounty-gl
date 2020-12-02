#include "assets.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>

#include "gfx/stb_image.h"

namespace bty {

Assets::Assets() {
    border_.resize(8);
    for (int i = 0; i < 8; i++) {
        border_[i] = get_texture(fmt::format("border-normal/box{}.png", i));
    }
    font_.load_from_texture(get_texture("fonts/genesis_custom.png"), {8.0f, 8.0f});
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

Assets::~Assets()
{
    for (auto &[path, texture] : textures_) {
        glDeleteTextures(1, &texture.handle);
    }
}

const std::vector<const Texture *> &Assets::get_border() const {
    return border_;
}

const Font &Assets::get_font() const {
    return font_;
}

Texture *Assets::get_texture(const std::string &path, glm::ivec2 num_frames)
{
    const auto texture_path = "data/textures/" + path;
    if (textures_.contains(texture_path)) {
        return &textures_[texture_path];
    }

    if (num_frames.x > 1 || num_frames.y > 1) {
        return get_texture_array(texture_path, num_frames);
    }

    return get_single_texture(texture_path);
}

Texture *Assets::get_texture_array(const std::string &path, glm::ivec2 num_frames) {
    int c;
    int w;
    int h;

    stbi_set_flip_vertically_on_load(false);
    stbi_uc *data = stbi_load(path.c_str(), &w, &h, &c, 0);

    if (!data) {
        spdlog::error("stbi error: {}: {}", path, stbi_failure_reason());
        return nullptr;
    }
    else {
        // spdlog::debug("Texture {} dimensions {}x{} components {}. Frames: {}x{}", path, w, h, c, num_frames.x, num_frames.y);
    }

    GLenum iformat = c == 3 ? GL_RGB8 : GL_RGBA8;
    GLenum format = c == 3 ? GL_RGB : GL_RGBA;

    int frame_width = w / num_frames.x;
    int frame_height = h / num_frames.y;
    int frame_count = num_frames.x * num_frames.y;

    GLuint tex;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex);
    glTextureStorage3D(tex, 1, iformat, frame_width, frame_height, frame_count);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, w);

    for (int i = 0; i < num_frames.x; i++) {
        for (int j = 0; j < num_frames.y; j++) {
            glTextureSubImage3D(
                tex,
                0, 0, 0,
                num_frames.x * j + i,
                frame_width, frame_height,
                1,
                format,
                GL_UNSIGNED_BYTE,
                data + ((j * frame_height * w) + (i * frame_width)) * c
            );
        }
    }

    glGenerateTextureMipmap(tex);

    glTextureParameterf(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameterf(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameterf(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameterf(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameterf(tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);

    textures_[path] = {w, h, tex, num_frames.x, num_frames.y, frame_width, frame_height};

    return &textures_[path];
}

Texture *Assets::get_single_texture(const std::string &path) {
    int c;
    int w;
    int h;

    stbi_set_flip_vertically_on_load(false);
    stbi_uc *data = stbi_load(path.c_str(), &w, &h, &c, 0);

    if (!data) {
        spdlog::error("stbi error: {}: {}", path, stbi_failure_reason());
        return nullptr;
    }
    else {
        // spdlog::debug("Texture {} dimensions {}x{} components {}", path, w, h, c);
    }

    GLenum iformat = c == 3 ? GL_RGB8 : GL_RGBA8;
    GLenum format = c == 3 ? GL_RGB : GL_RGBA;

    GLuint tex;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, w);

    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureStorage2D(tex, 1, iformat, w, h);
    glTextureSubImage2D(tex, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    textures_[path] = {w, h, tex, 1, 1, w, h};

    return &textures_[path];
}

}    // namespace bty
