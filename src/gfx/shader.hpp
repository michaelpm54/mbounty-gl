#ifndef BTY_GFX_SHADER_HPP_
#define BTY_GFX_SHADER_HPP_

#include <string>

#include "gfx/gl.hpp"

namespace bty {

GLuint loadShader(const std::string &vertShader, const std::string &fragShader);

}    // namespace bty

#endif    // BTY_GFX_SHADER_HPP_
