#version 460

uniform sampler2D image;
uniform bool flip;

in vec2 texture_coord;

out vec4 colour;

void main()
{
    vec2 uv = texture_coord;
    if (flip) {
        uv.x = 1 - uv.x;
    }
    colour = texture(image, uv);
}
