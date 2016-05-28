
#version 330 core

layout (location = 0) in vec4 position;

uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(position.x, position.y, 0.0, 1.0);
}
