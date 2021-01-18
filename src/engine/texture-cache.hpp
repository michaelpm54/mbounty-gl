#ifndef BTY_ENGINE_TEXTURE_CACHE_HPP
#define BTY_ENGINE_TEXTURE_CACHE_HPP

#include <glm/vec2.hpp>
#include <string>
#include <unordered_map>

#include "engine/singleton.hpp"
#include "gfx/font.hpp"
#include "gfx/gl.hpp"
#include "gfx/texture.hpp"

namespace bty {

class TextureCache {
public:
    void init(const std::string &basePath);
    void deinit();

    const std::vector<const Texture *> &getBorder() const;
    const Font &getFont() const;
    Texture *get(const std::string &path, glm::ivec2 numFrames = {1, 1});
    const std::string &getBasePath() const;
    void free(const Texture *texture);

private:
    Texture *getSingleTexture(const std::string &path);
    Texture *getArrayTexture(const std::string &path, glm::ivec2 numFrames);

private:
    std::string _basePath;
    std::unordered_map<std::string, Texture> _cache;
    std::vector<const Texture *> _border;
    Font _font;
};

}    // namespace bty

using Textures = bty::SingletonProvider<bty::TextureCache>;

#endif    // BTY_ENGINE_TEXTURE_CACHE_HPP
