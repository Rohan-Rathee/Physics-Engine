#version 330 core
layout (location = 0) in vec2 aPos;

uniform float uAspectCorrection;

void main()
{
    vec2 pos = vec2(aPos.x * uAspectCorrection, aPos.y);
    gl_Position = vec4(pos, 0.0, 1.0);
}
