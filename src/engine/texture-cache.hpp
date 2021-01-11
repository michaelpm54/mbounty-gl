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
    void init(const std::string &base_path);
    void deinit();

    const std::vector<const Texture *> &get_border() const;
    const Font &get_font() const;
    Texture *get(const std::string &path, glm::ivec2 num_frames = {1, 1});
    const std::string &get_base_path() const;

private:
    Texture *get_single_texture(const std::string &path);
    Texture *get_texture_array(const std::string &path, glm::ivec2 num_frames);

private:
    std::string base_path_;
    std::unordered_map<std::string, Texture> _cache;
    std::vector<const Texture *> border_;
    Font font_;
};

}    // namespace bty

using Textures = bty::singleton_provider<bty::TextureCache>;

#endif    // BTY_ENGINE_TEXTURE_CACHE_HPP
