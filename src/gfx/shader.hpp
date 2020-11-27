#ifndef BTY_GFX_SHADER_HPP_
#define BTY_GFX_SHADER_HPP_

#include <string>

#include "gfx/gl.hpp"

namespace bty {

GLuint load_shader(const std::string &vert_shader, const std::string &frag_shader);

}    // namespace bty

#endif    // BTY_GFX_SHADER_HPP_
