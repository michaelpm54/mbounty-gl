#version 460

layout(location = 0) in vec2 position;

uniform mat4 camera;
uniform mat4 transform;

void main()
{
    gl_Position = camera * transform * vec4(position, 0, 1);
}
