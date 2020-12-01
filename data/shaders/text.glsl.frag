#version 460

uniform sampler2D image;

in vec2 texture_coord;

out vec4 colour;

void main()
{
    colour = texture(image, texture_coord);
}
