#ifndef BTY_ASSETS_HPP_
#define BTY_ASSETS_HPP_

#include <string>
#include <unordered_map>

#include <glm/vec2.hpp>

#include "gfx/gl.hpp"
#include "gfx/texture.hpp"

namespace bty {

class Assets {
public:
    ~Assets();

    Texture *get_texture(const std::string &path, glm::ivec2 num_frames = {1, 1});

private:
    // Texture *get_single_texture(const std::string &path);
    Texture *get_texture_array(const std::string &path, glm::ivec2 num_frames);

private:
    std::unordered_map<std::string, Texture> textures_;
};

}    // namespace bty

#endif    // BTY_ASSETS_HPP_
