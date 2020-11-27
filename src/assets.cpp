#include "assets.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>

#include "gfx/stb_image.h"

namespace bty {

Assets::~Assets()
{
    for (auto &[path, texture] : textures_) {
        glDeleteTextures(1, &texture.handle);
    }
}

Texture *Assets::load_texture(const std::string &path, bool &success)
{
    const auto texture_path = "data/textures/" + path;
    if (textures_.contains(texture_path)) {
        return &textures_[texture_path];
    }

    GLuint tex;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);

    int c;
    int w;
    int h;

    stbi_set_flip_vertically_on_load(false);
    stbi_uc *data = stbi_load(texture_path.c_str(), &w, &h, &c, 0);
    if (!data) {
        spdlog::error("stbi error: {}: {}", texture_path, stbi_failure_reason());
        success = false;
        return nullptr;
    }
    else {
        spdlog::debug("Texture {} dimensions {}x{} components {}", texture_path, w, h, c);
    }

    GLenum iformat = c == 3 ? GL_RGB8 : GL_RGBA8;
    GLenum format = c == 3 ? GL_RGB : GL_RGBA;

    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTextureStorage2D(tex, 1, iformat, w, h);
    glTextureSubImage2D(tex, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);

    glGenerateTextureMipmap(tex);

    stbi_image_free(data);

    textures_[texture_path] = {w, h, tex};

    return &textures_[texture_path];
}

}    // namespace bty
