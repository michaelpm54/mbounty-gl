#version 460

uniform sampler2D image;
uniform bool flip;
uniform bool repeat;
uniform vec2 size;

in vec2 texture_coord;

out vec4 colour;

void main()
{
    vec2 uv = texture_coord;
    if (flip) {
        uv.x = 1 - uv.x;
    }
    if (repeat) {
        uv *= size;
    }
    colour = texture(image, uv);
}
