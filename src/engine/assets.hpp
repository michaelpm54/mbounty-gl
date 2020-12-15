#ifndef BTY_ENGINE_ASSETS_HPP_
#define BTY_ENGINE_ASSETS_HPP_

#include <glm/vec2.hpp>
#include <string>
#include <unordered_map>

#include "gfx/font.hpp"
#include "gfx/gl.hpp"
#include "gfx/texture.hpp"

namespace bty {

class Assets {
public:
    Assets(const std::string &base_path);
    ~Assets();

    const std::vector<const Texture *> &get_border() const;
    const Font &get_font() const;
    Texture *get_texture(const std::string &path, glm::ivec2 num_frames = {1, 1});
    const std::string &get_base_path() const;

private:
    Texture *get_single_texture(const std::string &path);
    Texture *get_texture_array(const std::string &path, glm::ivec2 num_frames);

private:
    std::string base_path_;
    std::unordered_map<std::string, Texture> textures_;
    std::vector<const Texture *> border_;
    Font font_;
};

}    // namespace bty

#endif    // BTY_ENGINE_ASSETS_HPP_
