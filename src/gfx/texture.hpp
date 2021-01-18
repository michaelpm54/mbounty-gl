#ifndef BTY_GFX_TEXTURE_HPP_
#define BTY_GFX_TEXTURE_HPP_

#include "gfx/gl.hpp"

namespace bty {

struct Texture {
    int width;
    int height;
    GLuint handle;
    int framesX;
    int framesY;
    int frameW;
    int frameH;
};

}    // namespace bty

#endif    // BTY_GFX_TEXTURE_HPP_
