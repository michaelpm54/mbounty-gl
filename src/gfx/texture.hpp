#ifndef BTY_GFX_TEXTURE_HPP_
#define BTY_GFX_TEXTURE_HPP_

#include "gfx/gl.hpp"

namespace bty {

struct Texture {
    int width;
    int height;
    GLuint handle;
    int num_frames_x;
    int num_frames_y;
    int frame_width;
    int frame_height;
};
    
}

#endif // BTY_GFX_TEXTURE_HPP_
