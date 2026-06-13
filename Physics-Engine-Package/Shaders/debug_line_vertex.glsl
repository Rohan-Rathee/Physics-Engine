#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
    vec3 vertexColor;
} vs_out;

void main()
{
    gl_Position = projection * view * vec4(position, 1.0);
    vs_out.vertexColor = color;
}
