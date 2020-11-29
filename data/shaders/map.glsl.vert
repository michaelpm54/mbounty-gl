#version 460

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tex_coord;

uniform mat4 camera;

out vec2 texture_coord;

void main()
{
    gl_Position = camera * vec4(position, 0, 1);
    texture_coord = tex_coord;
}
