#ifndef BTY_ASSETS_HPP_
#define BTY_ASSETS_HPP_

#include <string>
#include <unordered_map>

#include "gfx/gl.hpp"
#include "gfx/texture.hpp"

namespace bty {

class Assets {
public:
    ~Assets();

    Texture *load_texture(const std::string &path, bool &success);

private:
    std::unordered_map<std::string, Texture> textures_;
};

}    // namespace bty

#endif    // BTY_ASSETS_HPP_
