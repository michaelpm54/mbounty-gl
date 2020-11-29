#version 460

uniform sampler2DArray image;
uniform int frame;
uniform bool flip;

in vec2 texture_coord;

out vec4 colour;

void main()
{
    vec2 uv = texture_coord;
    if (flip) {
        uv.x = 1 - uv.x;
    }
    colour = texture(image, vec3(uv, frame));
}
