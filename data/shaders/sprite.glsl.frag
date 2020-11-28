#version 460

uniform sampler2DArray image;
uniform int frame;

in vec2 texture_coord;

out vec4 colour;

void main()
{
    colour = texture(image, vec3(texture_coord, frame));
}
