#include "engine/texture-cache.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <spdlog/spdlog.h>

#include "gfx/stb_image.hpp"

namespace bty {

void TextureCache::init(const std::string &basePath)
{
    _basePath = basePath;
    _border.resize(8);
    for (int i = 0; i < 8; i++) {
        _border[i] = get(fmt::format("border-normal/box{}.png", i));
    }
    _font.loadFromTexture(get("fonts/genesis_custom.png"), {8.0f, 8.0f});
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void TextureCache::deinit()
{
    int memBefore = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &memBefore);
    for (auto &[path, texture] : _cache) {
        glDeleteTextures(1, &texture.handle);
    }
    int memAfter = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &memAfter);
    spdlog::debug("TextureCache :: freed {} bytes", memAfter - memBefore);
}

const std::vector<const Texture *> &TextureCache::getBorder() const
{
    return _border;
}

const Font &TextureCache::getFont() const
{
    return _font;
}

Texture *TextureCache::get(const std::string &path, glm::ivec2 numFrames)
{
    const auto texturePath = fmt::format("{}/textures/{}", _basePath, path);

    if (_cache.contains(texturePath)) {
        return &_cache[texturePath];
    }

    if (numFrames.x > 1 || numFrames.y > 1) {
        return getArrayTexture(texturePath, numFrames);
    }

    return getSingleTexture(texturePath);
}

Texture *TextureCache::getArrayTexture(const std::string &path, glm::ivec2 numFrames)
{
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
        // spdlog::debug("Texture {} dimensions {}x{} components {}. Frames: {}x{}", path, w, h, c, numFrames.x, numFrames.y);
    }

    GLenum internalFormat = c == 3 ? GL_RGB8 : GL_RGBA8;
    GLenum format = c == 3 ? GL_RGB : GL_RGBA;

    int frameWidth = w / numFrames.x;
    int frameHeight = h / numFrames.y;
    int frameCount = numFrames.x * numFrames.y;

    GLuint tex;
    glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &tex);
    glTextureStorage3D(tex, 1, internalFormat, frameWidth, frameHeight, frameCount);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, w);

    for (int i = 0; i < numFrames.x; i++) {
        for (int j = 0; j < numFrames.y; j++) {
            glTextureSubImage3D(
                tex,
                0,
                0,
                0,
                numFrames.x * j + i,
                frameWidth,
                frameHeight,
                1,
                format,
                GL_UNSIGNED_BYTE,
                data + ((j * frameHeight * w) + (i * frameWidth)) * c);
        }
    }

    glGenerateTextureMipmap(tex);

    glTextureParameterf(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameterf(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameterf(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameterf(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameterf(tex, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_image_free(data);

    _cache[path] = {w, h, tex, numFrames.x, numFrames.y, frameWidth, frameHeight};

    return &_cache[path];
}

Texture *TextureCache::getSingleTexture(const std::string &path)
{
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

    GLenum internalFormat = c == 3 ? GL_RGB8 : GL_RGBA8;
    GLenum format = c == 3 ? GL_RGB : GL_RGBA;

    GLuint tex;
    glCreateTextures(GL_TEXTURE_2D, 1, &tex);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, w);

    glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTextureStorage2D(tex, 1, internalFormat, w, h);
    glTextureSubImage2D(tex, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    _cache[path] = {w, h, tex, 1, 1, w, h};

    return &_cache[path];
}

const std::string &TextureCache::getBasePath() const
{
    return _basePath;
}

void TextureCache::free(const Texture *texture)
{
    auto it = _cache.begin();
    for (it; it != _cache.end(); ++it) {
        if (&it->second == texture) {
            break;
        }
    }
    if (it == _cache.end()) {
        spdlog::warn("Attempted to free texture not contained in cache");
    }
    else {
        glDeleteTextures(1, &const_cast<Texture *>(texture)->handle);
        _cache.erase(it->first);
    }
}

}    // namespace bty
